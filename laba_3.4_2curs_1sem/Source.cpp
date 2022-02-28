#include"LiftSysControle.h"

int main()
{
	std::string people_file = "people.txt";
	std::string build_file = "building.txt";
	LiftSysControle lc(build_file, people_file);
	lc.start();
	return 0;
}