#include "fsm_structs.h"


///////////////////////////////////////////////////
// message_queue
///////////////////////////////////////////////////

void message_queue::queue_thread_func()
{
	MSG msg;
	while (_is_processing)
	{
		//������� ��������� �� ����������� ������� �...
		wait_and_pop(msg);
		//���������� ��������� ���� �������� ������ ��� ������� -
		//"�� ����" = "����" = nullptr
		if ((!msg._sysm_to) && (!msg._sysm_from))
		{
			_is_processing = false;
			break;
		}
		else {
			//...����� ��������� ��������� ��������
			msg._sysm_to->PushToInsideQueues(msg);
			PushToLog(msg);

		}

	}
}

message_queue::message_queue(LogLEV level) : _log_lev(level)
{
	try
	{
		std::thread t{ &message_queue::queue_thread_func,this };
		_queue_handler = std::move(t);
		Log::Get().Add(_queue_handler.get_id(), "mq_handler");
	}
	catch (const std::exception&)
	{
		assert(!"Error when making new thread.");
	}
}
message_queue::~message_queue()
{
	//_is_processing = false - ����� �� ���� ���������� ��������� ���� ����� ����� �� ��������
	//�� ����� ���� ������� �� �������� ���������
	//_is_processing = false;
	if (_queue_handler.joinable())
		_queue_handler.join();
}

//����� IAmDone �������� ������� ���������� ������� ��� �� 
//�������� ��������� ����� ���������
bool message_queue::IAmDone(SysName sysmemname)
{
	auto member = GetMemberByName(sysmemname);
	if (member) {
		_ifdone[sysmemname] = true;
		//���� ��� ��������� ��������� ���������
		if (_ifdone.size() == _members.size()) {
			//����� ��� ��������� ������� ��������� ���� ���������
			//������� � ���� ������� �������� ��������� �� ��������� �������
			push(*new MSG(nullptr, nullptr, EV::empty_event, MSGTo::extl));
			return true;
		}
		else
			return false;
	}
	return false;
}

//�������� ��������� ���� ��������� ����������

void message_queue::WaitForMQDone()
{
	if (_queue_handler.joinable())	_queue_handler.join();
}

// � ������� AddSystemMember �������� ��������� ��������� � ����(������ � ���) � 
// ������� ��������� ���������
void message_queue::AddSystemMember(SysName name, system_member* member)
{
	try
	{
		auto temp = _members.at(name);
		assert(!"Error when trying to insert new system member. Member with the same name exists.");
	}
	catch (const std::exception&)
	{
		_members[name] = member;
	}
}
	
//��������� � ����� ������� ��������� ��������� ��������� ��� 
//������� ��������� �������(�� ����� ���������)
system_member* message_queue::MQPush(
	system_member* memfrom,
	SysName memname,
	EV event,
	MSGTo toque,
	MsgPars params
)
{
	auto member = GetMemberByName(memname);
	if (member) {
		push(*new MSG(memfrom, member, event, toque, params));
		return member;
	}
	else
		return nullptr;
}

//��������� � ����� ������� ��������� ��������� ��������� ��� 
//������� ��������� �������(�� ������ ���������)
system_member* message_queue::MQPush(
	system_member* memfrom,
	system_member* memto,
	EV event,
	MSGTo toque,
	MsgPars params
)
{
	try
	{
		push(*new MSG(memfrom, memto, event, toque, params));
		return memto;
	}
	catch (const std::exception&)
	{
		return nullptr;
	}
}

system_member* message_queue::GetMemberByName(SysName name)
{
	system_member* member{};
	try
	{
		member = _members.at(name);
		return member;
	}
	catch (const std::exception&)
	{
		return nullptr;
	}
}


void message_queue::StopNow()
{
	//!

}

inline void message_queue::PushToLog(MSG msg)
{
	if (_log_lev == LogLEV::log || _log_lev == LogLEV::log_debug) 
	{
		Log::Get() << LogOp::indent20 << (msg._sysm_from ? msg._sysm_from->GetName() : "null") <<
			" ------> " << LogOp::indent20 << (msg._sysm_to ? msg._sysm_to->GetName() : "null") <<
			" EV = " << SD::Get().gEventDescByEvent(msg._event) << "\n";
	}
}


