#include "LiftSysControle.h"
#include<filesystem>

int LiftSysControle::cur_time = 0;
std::queue<int> LiftSysControle::unprocessed_buttons;

LiftSysControle::LiftSysControle(const std::string& people_conf_file, const std::string& build_conf_file)
{
	init(people_conf_file, build_conf_file);
	//main_working();
}

LiftSysControle::~LiftSysControle()
{
	while (1)
	{
		try
		{
			delete people->delMin();
		}
		catch (const std::exception&)
		{
			break;
		}
	}
}

void LiftSysControle::start()
{
	main_working();
}

void LiftSysControle::init(const std::string& build_conf_file, const std::string& people_conf_file)
{
	std::filesystem::create_directory("results");
	people = new BinomialHeap<int, Person*>(&cmp);
	init_building(build_conf_file);
	init_people(people_conf_file);
}

void LiftSysControle::init_building(const std::string& build_conf_file)
{
	std::ifstream f(build_conf_file);
	if (!f.is_open())
		throw std::exception("buildeng init file can't be opened");

	std::string buf = "";
	f >> buf;
	int floors_num = atoi(buf.c_str());

	for (int i = 0; i < floors_num; i++)
	{
		floors.push_back(new Floor(i));
	}

	f >> buf;
	int lifts_num = atoi(buf.c_str());

	int M;
	for (int i = 0; i < lifts_num; i++)
	{
		f >> buf;
		M = atoi(buf.c_str());
		lifts.push_back(new Lift(M, floors_num));
	}
}

void LiftSysControle::init_people(const std::string& people_conf_file)
{
	std::ifstream f(people_conf_file);
	if (!f.is_open())
		throw std::exception("people init file can't be opened");

	std::string buf;
	int _time, tf, sf, weight, id, index;
	while (!f.eof())
	{
		f >> buf;
		if (f.eof())
			break;
		id = atoi(buf.c_str());
		f >> buf;
		weight = atoi(buf.c_str());
		f >> buf;
		sf = atoi(buf.c_str());
		f >> buf;
		tf = atoi(buf.c_str());
		f >> buf;
		index = buf.find(':');
		_time = atoi(buf.substr(0, index + 1).c_str()) * 60 + atoi(buf.substr(index, buf.size()).c_str());

		people->insert(_time, new Person(_time, tf, sf, weight, id));
	}
}

void LiftSysControle::people_appearance()
{
	if (people->is_empty()) return; // людей больше нет
	Person* ps;
	while (!people->is_empty() && cur_time == people->getMin()->appearance_time)
	{
		ps = people->getMin();
		Floor* tfloor = floors[ps->source_floor];
		tfloor->people_on_floor.push_front(ps);
		ps->meet_people_on_floor(tfloor);
		if (tfloor->is_button_pressed == false)
		{
			tfloor->is_button_pressed = true;
			unprocessed_buttons.push(tfloor->num);
		}
		people->delMin();
	}
}

void LiftSysControle::lifts_distrubution()
{ 
	while (!unprocessed_buttons.empty())
	{
		Floor* floor = floors[unprocessed_buttons.front()];
		unprocessed_buttons.pop();

		Lift* chosen_l = nullptr;
		int max_ratio = 0, ratio = 0;
		for (int i = 0; i < lifts.size(); i++)
		{
			if (lifts[i]->cur_status == Lift::STAY)
				ratio += floors.size();
			else if (lifts[i]->cur_status == Lift::UP && (lifts[i]->cur_floor - floor->num < 0))
				ratio += floors.size();
			else if (lifts[i]->cur_status == Lift::DOWN && (lifts[i]->cur_floor - floor->num > 0))
				ratio += floors.size();

			ratio += 12;
			ratio -= (abs(lifts[i]->cur_floor - floor->num));
			if (ratio > max_ratio)
			{
				max_ratio = ratio;
				chosen_l = lifts[i];
			}
				
			ratio = 0;
		}
		chosen_l->add_to_heap(floor->num, floors.size());
	}
}

void LiftSysControle::main_working()
{
	while (cur_time != 1440)
	{
		people_appearance();
		lifts_distrubution();
		for (int i = 0; i < lifts.size(); i++)
		{
			lifts[i]->moving(floors);
		}
		cur_time++;
	}
	for (int i = 0; i < lifts.size(); i++)
	{
		lifts[i]->log_all_information(i + 1);
	}
}

LiftSysControle::Person::Person(int app_time, int tf, int sf, int weight, int person_id)
{
	this->appearance_time = app_time;
	this->targete_floor = tf;
	this->source_floor = sf;
	this->weight = weight;
	this->person_id = person_id;


	
	logBuilder bld;
	logger = bld.buildLog("results/persons_result.txt");
}

void LiftSysControle::Person::enter_the_lift(int time_seconds)
{
	enter_lift_time = cur_time * 60 + time_seconds;
}

void LiftSysControle::Person::leave_the_lift(int time_seconds)
{
	moving_time = (cur_time * 60 + time_seconds) - enter_lift_time;
	log_all_info();
}

void LiftSysControle::Person::meet_people_on_floor(Floor* floor)
{
	Person* ps;
	std::forward_list<Person*>::iterator it = floor->people_on_floor.begin();
	while (it != floor->people_on_floor.end())
	{
		ps = *it;
		if (!persons_met.count(ps->person_id) && ps != this)
		{
			ps->persons_met.insert(std::make_pair(person_id, this)); // знакомим постороннего с вошедшим
			persons_met.insert(std::make_pair(ps->person_id, ps)); // знакомим вошедвшего с посторонним
		}
		it++;
	}
}

void LiftSysControle::Person::meet_people_in_lift(Lift* lift)
{
	
	Person* ps;
	for (int i = 0; i < lift->people_list_array.size(); i++) // этот цикл нужен чтобы пройтись по всем €чейкам массива списков
	{
		std::forward_list<Person*>::iterator it = lift->people_list_array[i].begin();
		while (it != lift->people_list_array[i].end()) // этот цикл проходитс€ непосредственно по списку
		{
			ps = *it;
			if (!persons_met.count(ps->person_id) && ps != this)
			{
				ps->persons_met.insert(std::make_pair(person_id, this)); // знакомим постороннего с вошедшим
				persons_met.insert(std::make_pair(ps->person_id, ps)); // знакомим вошедвшего с посторонним
			}
			it++;
		}
	}
	
}

void LiftSysControle::Person::log_all_info()
{
	std::string time = "00:00";
	std::string info = "person id: " + std::to_string(person_id) + "\n\tappearance time: " + std::to_string(appearance_time/60)+":" + std::to_string(appearance_time % 60)+ "\n\tsource floor: " + std::to_string(source_floor) + "\n\ttarget floor: " +
		std::to_string(targete_floor) + "\n\tentering lift time: " + std::to_string(enter_lift_time) + "\n\tmoving time: " + std::to_string(moving_time) +
		"\n\tleft the lift because overload: " + (left_lift_because_overload ? "yes" : "no") + "\n\tpersons met: ";
	
	for (auto item : persons_met)
	{
		info += std::to_string(item.first) + " ";
	}
	logger->log(INFORMATION, info, time);
}

LiftSysControle::Lift::Lift(int M, int floors_num)
{
	logBuilder lb;
	logger = lb.buildLog("results/lifts_work_result.txt");
	target_floors_heap = new BinaryHeap<int, int>(&cmp);
	max_weight = M;
	for (int i = 0; i < floors_num; i++)
	{
		std::forward_list<Person*> some;
		people_list_array.push_back(some);
		buttons.push_back(UNPRESSED);
	}
}

LiftSysControle::Lift::~Lift()
{
	delete target_floors_heap;
	delete logger;
}

void LiftSysControle::Lift::moving(const std::vector<Floor*>& floors)
{
	// ћ≈“ќƒ ƒќЋ∆≈Ќ ƒќ≈«∆ј“№ ƒќ —Ћ≈ƒ”ёў≈√ќ ÷≈Ћ≈¬ќ√ќ Ё“ј∆ј
	if (target_floors_heap->is_empty()) // никуда ехать не надо
	{
		staying_time += 60;
		return;
	}
	int time_seconds = 0;
	int tfloor = target_floors_heap->getMin();
	double cur_load = (double)cur_weight / max_weight;
	int speed = std::floor(3 + 5 * cur_load);
	if(time_left_to_next_floor == 0)
		time_left_to_next_floor = speed;
	if (cur_floor > tfloor)
		cur_status = DOWN;
	else if (cur_floor < tfloor)
		cur_status = UP;
	else
		time_left_to_next_floor = 0; // если лифт сто€л и его вызвали на том же этаже )

	while (1)
	{
		if (time_left_to_next_floor == 0) // доехали до этажа
		{
			floors_done++;
			cur_floor += cur_status;
			if (cur_floor == tfloor) // если на этом этаже заход€т или выход€т
			{
				cur_status = STAY;
				target_floors_heap->delMin();
				is_door_closed = false;
				put_in_out(tfloor, time_seconds, floors);
				is_door_closed = true;

				if (target_floors_heap->is_empty())
				{
					staying_time += (60 - time_seconds);
					return;
				}
				tfloor = target_floors_heap->getMin();
				cur_status = (cur_floor > tfloor ? DOWN : UP);
				cur_load = (double)cur_weight / max_weight;
				speed = std::floor(3 + 5 * cur_load);
				time_left_to_next_floor = speed;
			}
			time_left_to_next_floor = speed;
		}
		if (time_seconds == 60)
		{
			break;
		}
		time_left_to_next_floor--;
		time_seconds++;
		moving_time++;
	}
}

void LiftSysControle::Lift::put_in_out(int floor, int time_seconds, const std::vector<Floor*>& floors)
{
	std::forward_list<Person*>::iterator it;
	std::stack<Person*> overloaded_people;
	Person* ps;
	
	
	buttons[floor] = UNPRESSED;
	floors[floor]->is_button_pressed = false;
	if (!people_list_array[floor].empty()) // if there is anyone in elevator
	{
		it = people_list_array[floor].begin();
		while (it != people_list_array[floor].end()) // высаживаем людей
		{
			sum_elevated_weight += (*it)->weight;
			cur_weight -= (*it)->weight;
			(*it)->meet_people_on_floor(floors[floor]);
			(*it)->leave_the_lift(time_seconds);
			delete (*it);
			people_list_array[floor].pop_front();
			it = people_list_array[floor].begin();
		}
	}
	
	if (!floors[floor]->people_on_floor.empty()) // if there is anyone on the floor
	{
		it = floors[floor]->people_on_floor.begin();
		while (!floors[floor]->people_on_floor.empty()) // вводим людей в лифт
		{
			(*it)->meet_people_in_lift(this);
			if ((*it)->weight + cur_weight > max_weight)
			{
				overloaded_people.push((*it));
				overloads++;
				(*it)->left_lift_because_overload = true;
			}
			else
			{
				(*it)->enter_the_lift(time_seconds);
				if (buttons[(*it)->targete_floor] == UNPRESSED)
				{
					buttons[(*it)->targete_floor] = PRESSED; // нажимаем на кнопку
					// далее закидываем в кучу целевых этажей
					if (!floors[(*it)->targete_floor]->is_button_pressed)
						add_to_heap((*it)->targete_floor, floors.size());

				}


				people_list_array[(*it)->targete_floor].push_front((*it));
				cur_weight += (*it)->weight;
				if (cur_weight > maximum_elevated_weight)
					maximum_elevated_weight = cur_weight;
			}
			floors[floor]->people_on_floor.pop_front();
			it = floors[floor]->people_on_floor.begin();
		}
		while (!overloaded_people.empty()) // не поместившихс€ людей обратно на этаж
		{
			floors[floor]->is_button_pressed = true;
			unprocessed_buttons.push(floor);
			floors[floor]->people_on_floor.push_front(overloaded_people.top());
			overloaded_people.pop();
		}
	}
	
}

void LiftSysControle::Lift::add_to_heap(int floor, int floors_num)
{
	if (target_floors_heap->is_empty())
	{
		if (cur_floor - floor < 0)
			target_floors_heap->insert(floor + cur_floor, floor);
		else
			target_floors_heap->insert(floors_num - floor, floor);
	}
	else
	{
		int next_floor = target_floors_heap->getMin();
		if (cur_floor - next_floor < 0) // moving up
		{
			if (cur_floor - floor < 0)
				target_floors_heap->insert(floor, floor);
			else
				target_floors_heap->insert(floors_num - floor, floor);
		}
		else
		{
			if (cur_floor - floor < 0)
				target_floors_heap->insert(floor + cur_floor, floor);
			else
				target_floors_heap->insert(floors_num - floor, floor);
		}
	}
	
}

void LiftSysControle::Lift::log_all_information(int lift_num)
{
	std::string info = "lift nomber: " + std::to_string(lift_num) + ", staying time : " + std::to_string(staying_time) + ", moving time: " + 
		std::to_string(moving_time) + ", completed floors: " + std::to_string(floors_done) + ", summary passed weight: " + std::to_string(sum_elevated_weight) + 
		", max load: " + std::to_string(maximum_elevated_weight) + ", times of overloads: " + std::to_string(overloads);
	std::string time = "00:00";
	logger->log(INFORMATION, info, time);
}

LiftSysControle::Floor::Floor(int num)
{
	this->num = num;
}
