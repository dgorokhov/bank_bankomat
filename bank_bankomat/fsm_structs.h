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
/// ������ ����� ��� mq - ��� ������ ���� ������� �� ����� ���� ������� �� ��������� ���������
/// ����� ���������
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
	//� ����� ������� ���������� ���������
	MSGTo _to{};
	//������������
	/////////////////////
	MSG() {}
	~MSG() {}
	//��� ��������� �����������
	MSG(system_member* sysm_from, system_member* sysm_to, EV event, MSGTo to, MsgPars params = "")
		: _sysm_from(sysm_from), _sysm_to(sysm_to), _event(event), _to(to), _params(params)
	{}
	//���� �������� = �������
	MSG(system_member* selfptr, EV event, MSGTo to, MsgPars params = "")
		: _sysm_from(selfptr), _sysm_to(selfptr), _event(event), _to(to), _params(params)
	{}
	//������ ��������� - ���� ��� ���� ����� �������� ���������� ��������� � ��������� �����-��
	MSG(EV event, MSGTo to)
		: _sysm_from(nullptr), _sysm_to(nullptr), _event(event), _to(to), _params("")
	{}
};


//
// system_member - ����������� �����-������ ���� 
// ���������� ������� (����� �������)
//
class system_member
{
protected:
	//������ �� ����� ������� ��������� ����� ������� ���� �����
	message_queue* _mq;
	//��� ��������� (system_member)
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

	//���������������� � ���������� ���������-�������
	virtual void PushToInsideQueues(MSG msg) = 0;
	//���������������� � ���������� ���������-�������
	virtual void ToInitialState() = 0;
	//onStopEvent - ����� ���������� �������� ��������� ������� ���������
	//� ����� �� ��������� stop_processing
	//���������������� � ���������� ��������� �������.
	virtual void onStopEvent() = 0;
};



//����� ������� ����� ������� ��� ��������� �������
//������������ �����������.
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
	//����� IAmDone �������� ������� ���������� ������� ��� �� 
	//�������� ��������� ����� ���������
	bool IAmDone(SysName sysmemname);
	// � ������� AddSystemMember �������� ��������� ��������� � ����(������ � ���) � 
	// ������� ��������� ���������
	void AddSystemMember(SysName name, system_member* member);
	//��������� � ����� ������� ��������� ��������� ��������� ��� 
	//������� ��������� �������(�� ����� ���������)
	system_member* MQPush(
		system_member* memfrom,
		SysName memname,
		EV event,
		MSGTo toque,
		MsgPars params = ""
	);
	//��������� � ����� ������� ��������� ��������� ��������� ��� 
	//������� ��������� �������(�� ������ ���������)
	system_member* MQPush(
		system_member* memfrom,
		system_member* memto,
		EV event,
		MSGTo toque,
		MsgPars params = ""
	);
	system_member* GetMemberByName(SysName name);
	//�������� ��������� ���� ��������� ����������
	void WaitForMQDone();
	void StopNow();
	inline void PushToLog(MSG msg);
};




#endif
