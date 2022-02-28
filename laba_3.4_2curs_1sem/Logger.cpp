#define _CRT_SECURE_NO_WARNINGS
#include "Logger.h"

void Logger::log(SEV severity, const std::string& info, const std::string& _time)
{
	auto tmp = streams_list;
	std::string sev;
	switch ((int)severity)
	{
	case 0:
		sev = "TRACE";
		break;
	case 1:
		sev = "DEBUG";
		break;
	case 2:
		sev = "INFORMATION";
		break;
	case 3:
		sev = "WARNING";
		break;
	case 4:
		sev = "ERROR";
		break;
	case 5:
		sev = "FATAL";
		break;
	default:
		throw std::exception("wrong severity");
	}
	/*time_t seconds = time(NULL);
	tm* timeinfo = localtime(&seconds);*/

	while (tmp != nullptr)
	{
		if (severity >= tmp->_severity)
		{
			if (tmp->write_console)
				std::cout<< "[" << _time << "][" << sev << "]: " << info << std::endl;
			else
				tmp->_stream << "[" << _time << "][" << sev << "]: " << info<< std::endl;
		}
		tmp = tmp->next;
	}
}

Logger::~Logger()
{
	Stream* st = streams_list;
	Stream* tmp = nullptr;
	while (st != nullptr)
	{
		tmp = st;
		st = st->next;
		delete tmp;
	}
}
