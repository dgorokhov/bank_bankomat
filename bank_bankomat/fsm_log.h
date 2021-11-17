#ifndef _LOG_H_
#define _LOG_H_

#include <fstream>
#include <map>
#include <sstream>
#include <thread>
//#include <mutex>

#include "../../.libs/diglib.h"



// Log - Объект для логирования. 
// Синглтон (релизация Мейерса. Пишут что есть проблемы в многопоточной среде
// Пишет сообщения каждого потока, который был добавлен в Log
// в отдельный файл вида <thread_id>_<description>
// где 
//   <thread_id> - присваиваемый средой исполнения идентификатор потока 
//   <description> - задаваемое пользоватем описание для этого потока
// Перед первым использованием  - 
//		или Log::Get().Add(thread.thread_id(), <description> )			- извне (из внешнего потока, thread - ЭТО Std:thread нужного потока)
//		или Log::Get().Add(std::this_thread::get_id(), <description> )	- изнутри самого потока
//   
// Использование - вызвать  внутри нужного потока
//  Log::Get() << "message" << ...


enum class LogOp
{
	no_option,
	start_timer,
	time_now,
	indent10,
	indent15,
	indent20,
	indent25,
	indent30,
	indent35,
	indent40,
	//uppercase,
};

enum class LogLEV
{
	no_log,
	log,
	log_debug,
};

//Синглтон (релизация Мейерса. Пишут что есть проблемы в многопоточной среде
class Log
{
	using filename = std::string;
	using description = std::string;

	std::string _dirforlogs{ "logging" };
	std::map<filename, std::tuple<description, std::ofstream*, LogOp>> _map;

	auto GetThreadDesc(std::string filename);
	std::string GetFileNameByThreadId(std::thread::id id) const;
	std::string GetFileNameThisThread() const;
	//fout будет вызываться потоком который логируется
	inline void FOut(std::string str);
public:
	static Log& Get();
	~Log();
	//сохранить информацию по логируемому потоку 
	//внутри объекта Log
	void Add(std::thread::id id, description desc);
	//запомнить опции форматирования для логируемого потока
	Log& operator<<(LogOp option);
	Log& operator<<(std::string str);
	Log& operator<<(int num);
	Log& operator<<(void* ptr);
};

#endif