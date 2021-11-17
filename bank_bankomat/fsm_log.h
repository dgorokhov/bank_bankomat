#ifndef _LOG_H_
#define _LOG_H_

#include <fstream>
#include <map>
#include <sstream>
#include <thread>
//#include <mutex>

#include "../../.libs/diglib.h"



// Log - ������ ��� �����������. 
// �������� (��������� �������. ����� ��� ���� �������� � ������������� �����
// ����� ��������� ������� ������, ������� ��� �������� � Log
// � ��������� ���� ���� <thread_id>_<description>
// ��� 
//   <thread_id> - ������������� ������ ���������� ������������� ������ 
//   <description> - ���������� ����������� �������� ��� ����� ������
// ����� ������ ��������������  - 
//		��� Log::Get().Add(thread.thread_id(), <description> )			- ����� (�� �������� ������, thread - ��� Std:thread ������� ������)
//		��� Log::Get().Add(std::this_thread::get_id(), <description> )	- ������� ������ ������
//   
// ������������� - �������  ������ ������� ������
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

//�������� (��������� �������. ����� ��� ���� �������� � ������������� �����
class Log
{
	using filename = std::string;
	using description = std::string;

	std::string _dirforlogs{ "logging" };
	std::map<filename, std::tuple<description, std::ofstream*, LogOp>> _map;

	auto GetThreadDesc(std::string filename);
	std::string GetFileNameByThreadId(std::thread::id id) const;
	std::string GetFileNameThisThread() const;
	//fout ����� ���������� ������� ������� ����������
	inline void FOut(std::string str);
public:
	static Log& Get();
	~Log();
	//��������� ���������� �� ����������� ������ 
	//������ ������� Log
	void Add(std::thread::id id, description desc);
	//��������� ����� �������������� ��� ����������� ������
	Log& operator<<(LogOp option);
	Log& operator<<(std::string str);
	Log& operator<<(int num);
	Log& operator<<(void* ptr);
};

#endif