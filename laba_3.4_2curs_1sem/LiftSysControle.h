#pragma once
#include<vector>
#include<stack>
#include<queue>
#include<iostream>
#include<string>
#include<map>
#include<forward_list>
#include"containers/BinaryHeap.h"
#include"containers/BinomialHeap.h"
#include"containers/FibonacciHeap.h"
#include"logBuilder.h"
#include"Logger.h"


class LiftSysControle
{
	// 
	struct Person;
	struct Floor;
	struct Lift
	{
		Lift(int M, int floors_num);
		~Lift();

		enum STATUS { UP = 1, DOWN = -1, STAY };
		enum BUTTON { UNPRESSED, PRESSED };

		bool is_door_closed = true;
		STATUS cur_status = STAY;
		int cur_floor = 0;
		std::vector<BUTTON> buttons; // should be initialized
		int max_weight; // ������������ ��������, should be initialized
		int time_left_to_next_floor = 0; // ����� ������� �������� ����� �� ���������� �����
		int cur_weight = 0;
		int staying_time = 0; // ����� �������
		int moving_time = 0; // ����� �������
		int floors_done = 0; // ����� ������� ������ ������
		int maximum_elevated_weight = 0; // ����������� �������� ����
		int sum_elevated_weight = 0; // ��������� ������������ ����
		int overloads = 0; // ���������� ����������
		Logger* logger;//init
		int move_time_between_floors = 0;
		BinaryHeap<int, int>* target_floors_heap; // ������ ��� ���������, ������ ����� ����� (������ ����� ���������)
		CmpForInt cmp;
		std::vector<std::forward_list<Person*>> people_list_array; // shoeld be initialized

		//------------------------------------------------------------------------------------------------
		// ������
		//------------------------------------------------------------------------------------------------
		void moving(const std::vector<Floor*>& floors);
		void put_in_out(int floor, int time_seconds, const std::vector<Floor*>& floors);
		void add_to_heap(int floor, int floors_num);
		void log_all_information(int lift_num);
	};

	struct Person
	{
		Person(int app_time, int tf, int sf, int weight, int person_id);
		//static int person_id_creator;
		int person_id;//should be init
		int appearance_time;//should be init
		int targete_floor;//init
		int source_floor;//init
		int weight;//init
		int moving_time = 0; // ����� ������� (�������� ����� �������� ����� � ������)
		int enter_lift_time = 0; // ����� ����� � ���� (���������� ����������)
		bool left_lift_because_overload = false; // ������� �� ���� ����������
		std::map<int, Person*> persons_met; // ����������� ���������
		Logger* logger; // init
		//------------------------------------------------------------------------------------------------
		// ������
		//------------------------------------------------------------------------------------------------
		void enter_the_lift(int time_seconds);
		void leave_the_lift(int time_seconds);
		void meet_people_on_floor(Floor* floor); // �������� ����� ��� ������� ��������� � ����� ������ ����� ���� �������
		void meet_people_in_lift(Lift* lift); // �������� ����� ���� ������� �� ��������� ���� ����� ������ ���� ����������
		void log_all_info(); // ��������� ��� ����������� ���������� ����� ������� ����� �� �����
	};
	struct Floor
	{
		Floor(int num);
		std::forward_list<Person*> people_on_floor;
		bool is_button_pressed = false;
		int num;
	};
public:
	LiftSysControle(const std::string& people_conf_file, const std::string& build_conf_file);
	~LiftSysControle();
	void start();
private:
	void init(const std::string& build_conf_file, const std::string& people_conf_file);
	void init_building(const std::string& build_conf_file);
	void init_people(const std::string& people_conf_file);
	void people_appearance(); // adds to queue of unprocessed buttons new button and add person to floor and meet with people
	void lifts_distrubution(); // distribute floors between lifts
	void main_working();


private: // fields
	std::vector<Floor*> floors; //init
	std::vector<Lift*> lifts; // init
	BinomialHeap<int, Person*>* people; // ���� - ����� ���������//init
	CmpForInt cmp;
	static int cur_time;
	static std::queue<int> unprocessed_buttons;
};

