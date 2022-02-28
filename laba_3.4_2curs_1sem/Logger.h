#pragma once
#include"Stream.h"
#include<ctime>
class Logger
{
	friend class logBuilder;
public:
	void log(SEV severity, const std::string& info, const std::string& time);
	~Logger();
private:
	Logger() { streams_list = nullptr; }
	Stream* streams_list;
};

