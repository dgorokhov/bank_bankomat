#include <assert.h>
#include "fsm_log.h"

using namespace std;

//tuple<std::string, ofstream*, LogOp>
auto Log::GetThreadDesc(std::string filename)
{
	try
	{
		auto tpl = _map.at(filename);
		return tpl;
	}
	catch (const std::exception&)
	{
		assert(!"This thread id doesn't exist in thread description map!");
	}
}

std::string Log::GetFileNameByThreadId(std::thread::id id) const
{
	std::ostringstream ss;
	ss << id;
	std::string name = ss.str();
	return name;
}

std::string Log::GetFileNameThisThread() const
{
	std::thread::id id = std::this_thread::get_id();
	auto name = dig::to_str<std::thread::id>(id);
	return name;
}

//fout будет вызываться потоком который логируется
inline void Log::FOut(string str)
{
	auto  desc = GetThreadDesc(GetFileNameThisThread());
	ofstream* os = get<1>(desc);
	(*os) << str;
}

Log& Log::Get()
{
	static Log log;
	return log;
}
Log::~Log()
{
	for (auto member : _map)
	{
		try
		{
			auto tpl = member.second;
			ofstream* strm = get<1>(tpl);
			strm->close();
			delete strm;
		}
		catch (const std::exception&)
		{
			assert(!"Error in Log object destructor!");
		}
	}
}
//сохранить информацию по логируемому потоку 
//внутри объекта Log
void Log::Add(std::thread::id id, description desc)
{
	auto filename = GetFileNameByThreadId(id);
	try
	{
		auto at = _map.at(filename);
		//если поток с этим номером уже есть, остановка
		assert(!"This thread id is already in thread table!");
	}
	catch (std::exception&)
	{
		//такого потока нет. ок. добавляем
		ofstream* os = new ofstream(_dirforlogs + "/" + filename + "_" + desc);
		_map[filename] = make_tuple(desc, os, LogOp::no_option);
	}
}

//запомнить опции форматирования для логируемого потока
Log& Log::operator<<(LogOp option)
{
	auto filename = GetFileNameThisThread();
	auto desc = GetThreadDesc(filename);
	auto newdesc = make_tuple(get<0>(desc), get<1>(desc), option);
	_map[filename] = newdesc;
	return *this;
}

Log& Log::operator<<(string str)
{
	auto filename = GetFileNameThisThread();
	auto desc = GetThreadDesc(filename);
	auto lastoption = get<2>(desc);

	switch (lastoption)
	{
	case LogOp::no_option:
		FOut(str);
		break;
	case LogOp::start_timer:
		break;
	case LogOp::time_now:
		break;
	case LogOp::indent10:
		FOut((str.append(10, ' ').substr(0, 10)));
		break;
	case LogOp::indent15:
		FOut((str.append(15, ' ').substr(0, 15)));
		break;
	case LogOp::indent20:
		FOut((str.append(20, ' ').substr(0, 20)));
		break;
	case LogOp::indent25:
		FOut((str.append(25, ' ').substr(0, 25)));
		break;
	case LogOp::indent30:
		FOut((str.append(30, ' ').substr(0, 30)));
		break;
	case LogOp::indent35:
		FOut((str.append(35, ' ').substr(0, 35)));
		break;
	case LogOp::indent40:
		FOut((str.append(40, ' ').substr(0, 40)));
		break;
	default:
		FOut(str);
		break;
	}
	//после вывода сообщения в лог потока 
	//сбросить опции его логироввания
	auto newdesc = make_tuple(get<0>(desc), get<1>(desc), LogOp::no_option);
	_map[filename] = newdesc;
	return *this;
}

Log& Log::operator<<(int num)
{
	std::ostringstream ss;
	ss << num;
	FOut(ss.str());
	return *this;
}

