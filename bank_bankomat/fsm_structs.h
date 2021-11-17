#ifndef FSM_STRUCTS_H
#define FSM_STRUCTS_H


#include <fstream>
#include <iostream>
#include <sstream>

#include <thread>
#include <mutex>
#include <condition_variable>

#include <assert.h>
#include <map>
#include <functional>
#include <type_traits>
#include <string>
#include <queue>
#include <memory>


#include "../../.libs/diglib.h"
#include "fsm_packer.h"
#include "fsm_log.h"
#include "user_events.h"


/// <summary>
/// TODO
/// ввести опцию для mq - что делать если учасник по какой либо причние не закончили обработку
/// своих сообщений
/// </summary>

class system_member;
class message_queue;


//intl - internal queue
//extl - external queue
enum class MSGTo { intl, extl };


struct MSG
{
	system_member* _sysm_from{ nullptr };
	system_member* _sysm_to{ nullptr };
	EV _event{};
	MsgPars _params{};
	//в какую очередь передается сообщение
	MSGTo _to{};
	//конструкторы
	/////////////////////
	MSG() {}
	~MSG() {}
	//все параметры установлены
	MSG(system_member* sysm_from, system_member* sysm_to, EV event, MSGTo to, MsgPars params = "")
		: _sysm_from(sysm_from), _sysm_to(sysm_to), _event(event), _to(to), _params(params)
	{}
	//если источник = адресат
	MSG(system_member* selfptr, EV event, MSGTo to, MsgPars params = "")
		: _sysm_from(selfptr), _sysm_to(selfptr), _event(event), _to(to), _params(params)
	{}
	//пустое сообщение - лишь для того чтобы передать продолжить обработку в следующем обраб-ке
	MSG(EV event, MSGTo to)
		: _sysm_from(nullptr), _sysm_to(nullptr), _event(event), _to(to), _params("")
	{}
};


//
// system_member - абстрактный класс-предок всех 
// участников системы (кроме очереди)
//
class system_member
{
protected:
	//ссылка на общую очередь сообщений через которую идет обмен
	message_queue* _mq;
	//имя участника (system_member)
	SysName _name;
	LogLEV _log_lev;
public:
	//...check links &
	system_member(SysName name, message_queue& pmq, LogLEV level = LogLEV::log) : 
		_mq(&pmq), 
		_name(name),
		_log_lev(level)
	{	}
	virtual ~system_member() { }
	message_queue* GetMessageQueue() const { return _mq; }
	SysName GetName() const { return _name; }

	//переопределяется в конкретном участнике-системы
	virtual void PushToInsideQueues(MSG msg) = 0;
	//переопределяется в конкретном участнике-системы
	virtual void ToInitialState() = 0;
	//onStopEvent - метод вызываемый Конечным Автоматом данного участника
	//в ответ на сообщение stop_processing
	//переопределяется в конкретном участнике системы.
	virtual void onStopEvent() = 0;
};



//общая очередь через которую все участники системы
//обмениваются сообщениями.
class message_queue : dig::threadsafe_queue<MSG>
{

	std::atomic<bool> _is_processing{ true };
	std::thread _queue_handler{};
	std::map <SysName, system_member*> _members;
	std::map<SysName, bool> _ifdone;
	LogLEV _log_lev{ LogLEV::log };

	void queue_thread_func();
public:
	message_queue(LogLEV level = LogLEV::log);
	~message_queue();
	//через IAmDone участник системы уведомляет очередь что он 
	//закончил обработку своих сообщений
	bool IAmDone(SysName sysmemname);
	// с помощью AddSystemMember участник добавляет инфорацию о себе(ссылку и имя) в 
	// очередь обработки сообщений
	void AddSystemMember(SysName name, system_member* member);
	//поместить в общую очередь обработки сообщений сообщение для 
	//другого участника системы(по имени участника)
	system_member* MQPush(
		system_member* memfrom,
		SysName memname,
		EV event,
		MSGTo toque,
		MsgPars params = ""
	);
	//поместить в общую очередь обработки сообщений сообщение для 
	//другого участника системы(по ссылке участника)
	system_member* MQPush(
		system_member* memfrom,
		system_member* memto,
		EV event,
		MSGTo toque,
		MsgPars params = ""
	);
	system_member* GetMemberByName(SysName name);
	//ожидание обработки всех сообщений участников
	void WaitForMQDone();
	void StopNow();
	inline void PushToLog(MSG msg);
};




#endif
