#ifndef FSM_TEMPLATE_H
#define FSM_TEMPLATE_H

#include "fsm_structs.h"




// ������� ��� ������� FSM
////////////////////////////
//#define M_FSM_DEBUG
////////////////////////////
#ifdef M_FSM_DEBUG

//��� FSM
//������� �������, ������ � ��������� [F]unc, [L]ine, [M]essage
#define M_FSM_FLM(message) \
	Log::Get() << "***" << this->GetFSMName() << ", **FUNC = " << __func__ << " at LINE = " << __LINE__ << "  msg: " << message<<"\n";

//������� �������, ������ [F]unc, [L]ine 
#define M_FSM_FL \
	Log::Get() << "***" << this->GetFSMName() << ", **FUNC = " << __func__ << " at LINE = " << __LINE__ <<'\n';

//������� �������� ����������
#define M_FSM_VAR(varname, var) \
	Log::Get() << "***" << this->GetFSMName() << ", **FUNC = " << __func__ << " at LINE = " << __LINE__ << ", " \
		<< ##varname << " = " <<  var << "\n";

//������������� ��� FSM
#define M_FSM_STATES \
	Log::Get() << "***" << "_state" << " = " << GetStateId(_state) << "\n"; \
	Log::Get() << "***" << "_msgs_self" << " = " << _msgs_self.size() << "\n"; \
	Log::Get() << "***" << "_msgs_other" << " = " << _msgs_other.size() << "\n"; \
	Log::Get() << "***" << "__is_processing" << " = " << _is_processing << "\n";

#else
#define M_FSM_FLM(message)
#define M_FSM_FL
#define M_FSM_VAR(varname, var)
#define M_FSM_STATES 

#endif


//����� �������
//////////////////////////////////
//#define M_DEBUG_COMMON
//////////////////////////////////////
#ifdef M_DEBUG_COMMON

//������� ����������
#define M_VAR(varname, var) \
	Log::Get() << "***FUNC = " << __func__ << " at LINE = " << __LINE__ << ", " << ##varname << " = " <<  var << "\n";

//������� �������, ������ � ��������� [F]unc, [L]ine, [M]essage
#define M_FLM(message) \
	Log::Get() << "***FUNC = " << __func__ << " at LINE = " << __LINE__ << "  msg: " << message<<"\n";

//������� �������, ������ [F]unc, [L]ine 
#define M_FL \
	Log::Get() << "***FUNC = " << __func__ << " at LINE = " << __LINE__ <<'\n';

#else
#define M_VAR(varname, var)
#define M_FLM(message)
#define M_FL
#endif


// �����-������ ��������� ��������
// ����������� (����� ���������) � ��������� �������.
// (�������� ������� � ���� ������� ��������� system_member 
// 
// ������ ��������������� ����� (FSState -�������������) ��������� �� ������
// �������� ��������-��������� ��������.
// ���-������������ ����������� ����� ������� (EV) ������������ �� ������ ���������
// ������� � ������� �������� ����� ��� ���� ���������� ������� � ����������� � event_type.h



using namespace std;


template<typename FSState>
class fsm_async
{
	using FSHandler = std::map<FSState, std::tuple<std::function<void(MSG)>, std::string>>;
	using FSTransition = std::map<tuple<EV, FSState>, FSState>;

	static_assert(std::is_enum<FSState>::value, "Type used for FSState parameter is not enum!");
	static_assert(std::is_enum<EV>::value, "Type used for EV parameter is not enum!");

	enum class FSMFlag { wait_external, wait_internal };
	LogLEV _log_lev{ LogLEV::log };

private:
	SysName _fsm_name{};
	system_member* _owner{};
	message_queue* _mq{};
	std::atomic <FSMFlag> _fsm_flag{};

	std::atomic<bool> _is_processing{ true };
	//std::condition_variable _is_processing_var;
	//std::mutex _cond_var_mtx;

	FSState _state{};
	FSState _initial_state{};

	FSHandler _handlers{};
	std::thread _queue_handler{};

	dig::threadsafe_queue<MSG> _msgs_self{};
	dig::threadsafe_queue<MSG> _msgs_other{};

//��������� �������� ����������� � �� ���������
//////////////////////////////////
private:
	//���������, ���������� � �� �������������� � ��������� ������
	//_queue_handler_func - ��� �����������
	void _queue_handler_func()
	{
		while (_is_processing)
		{
			MSG msg{};
			switch (_fsm_flag)
			{
			//��������� ������� �������� ���������
			case FSMFlag::wait_external:
				M_FSM_FLM("WAIT_EXTERNAL");
				M_FSM_STATES;
				_msgs_other.wait_and_pop(msg);
				M_FSM_FL;
				M_FSM_STATES;
				if (msg._event == EV::stop_processing)
				{
					M_FSM_FLM("STOPPING");
					_is_processing = false;
					_owner->onStopEvent();
					//_is_processing_var.notify_one();
					break;
				}
				_fsm_flag = FSMFlag::wait_internal;
				msg._to = MSGTo::intl;
				_msgs_self.push(msg);
				break;

			//��������� ���������� �������� ���������
			case FSMFlag::wait_internal:
				M_FSM_FL;
				_msgs_self.wait_and_pop(msg);
				if (msg._event == EV::stop_processing)
				{
					M_FSM_FLM("STOPPING");
					_is_processing = false;
					_owner->onStopEvent();
					//_is_processing_var.notify_one();
					break;
				}
				if (!_is_state_handling_ok(msg))
				{
					M_FSM_FLM("STOPPING");
					_is_processing = false;
					//_is_processing_var.notify_one();
					break;

				}
				break;
			default:
				break;
			}

		}
	}
	
	bool _is_state_handling_ok(MSG msg)
	{
		try
		{
			M_FSM_FL;
			M_FSM_VAR("MSG._EVENT", GetEventId(msg._event));
			M_FSM_VAR("_STATE", GetStateId(_state));
			M_FSM_FL;
			auto funcdesc = _handlers.at(_state);
			auto func = get<0>(funcdesc);
			auto funcname= get<1>(funcdesc);
			M_FSM_FL;
			if ((!func))
			{
				SetStateWrong();
				return false;
			}
			func(msg);
			return true;
		}
		catch (exception&)
		{
			SetStateWrong();
			M_FSM_FLM("EXCEPTION, WRONG STATE");
			return false;
		}
	}

//������ � ���������
////////////////////////////////
public:
	
	//���� ��������� ������������� �� ��� ����� ���������
	inline bool IsMsgWrong(MSG msg)
	{
		if (this->_owner != msg._sysm_to)
		{
			SetStateWrong();
			return false;
		}
		else
			return true;

	}
	inline bool PushToInsideQueues(MSG msg)
	{
		switch (msg._to)
		{
		case MSGTo::intl:
			_msgs_self.push(msg);
			break;
		case MSGTo::extl:
			_msgs_other.push(msg);
			break;
		default:
			return false;
			break;
		}
		return true;
	}
	//������� � ������ ������� ������\������
	void PushAsClient(system_member* sm_to, EV ev, MsgPars params = "")
	{
		_mq->MQPush(_owner, sm_to, ev, MSGTo::extl, params);
	}
	void PushAsClient(SysName sm_to, EV ev, MsgPars params = "")
	{
		_mq->MQPush(_owner, sm_to, ev, MSGTo::extl, params);
	}

	//������� � ������ ������� ����� �� ��������� ����� ������\������
	void PushAsServer(system_member* sm_to, EV ev, MsgPars params = "")
	{
		_mq->MQPush(_owner, sm_to, ev, MSGTo::intl, params);
	}
	void PushAsServer(SysName sm_to, EV ev, MsgPars params = "")
	{
		_mq->MQPush(_owner, sm_to, ev, MSGTo::intl, params);
	}


	//��������� � ��������� ���������
	void ToFSMInitialState()
	{
		_state = _initial_state;
		_fsm_flag = FSMFlag::wait_external;
		M_FSM_STATES;
	}


	//��������� ��������� (���������) ���������� ������ ��� ���������(�� ���� ����)
	void HandleNewState(FSState state, MSG msg)
	{
		_state = state;
		//������� ��������� ���� (�������� ��������� �� ������� ������������) 
		//����������� �����-�� ���������� � ��� ���������� ���������
		PushToInsideQueues(*new MSG(msg._sysm_from, msg._sysm_from, msg._event, msg._to, msg._params));
	
	}

	//������ ���������� ����������� � �������� ������� ���������
	//������������ ��� �������� - ���� ����� ��������� ���������� �� ��� ���� ��� 
	//������� ������������ ����������
	void HandleNewState(FSState state)
	{
		_state = state;
		//������� ������ ��������� ����. ������ ��������� �� ����� �����������.
		//������� ��������� ����� ��� ���� ����� ��������� ������������
		PushToInsideQueues(*new MSG(_owner, _owner, EV::empty_event, MSGTo::intl));
	}

	//����� ��������� �� ������� ��������� �������������� � ��������� ���������
	void ChangeStateAndWait(FSState state)
	{
		_state = state;
	}


	// constructing & destructing
	/////////////////////////////////////////////
	fsm_async(SysName fsmname, FSState initial_state, system_member* owner, LogLEV level = LogLEV::log ) :
		_fsm_name(fsmname),
		_state(initial_state),
		_initial_state(initial_state),
		_fsm_flag(FSMFlag::wait_external),
		_owner(owner),
		_log_lev (level)
	{
		_mq = _owner->GetMessageQueue();
		assert(static_cast<int>(initial_state) == 1 && "FSM initial state should be = 1, wrong state = 0");
		try
		{
			std::thread t{ &fsm_async::_queue_handler_func,this };
			_queue_handler = std::move(t);
			Log::Get().Add(_queue_handler.get_id(), "fsm_" + fsmname);
		}
		catch (const std::exception&)
		{
			assert(!"Error when making new thread.");
		}
	}
	~fsm_async()
	{
		_is_processing = false;
		M_FSM_FL;
		M_FSM_STATES;
		if (_queue_handler.joinable())	_queue_handler.join();
	}

	fsm_async(const fsm_async& other) = delete;
	fsm_async(fsm_async&& other) = delete;

public:
	void AddHNDL(FSState state, std::function<void(MSG)> func, std::string funcname)
	{
		auto tpl = make_tuple(func, funcname);
		try
		{
			auto temp = _handlers.at(state);
			assert(!"Error when trying to insert handler. This handler already exists.");
		}
		catch (const std::exception&)
		{
			//Ok, no handler in map, inserting
			_handlers[state] = tpl;
		}
	}

public:
	
	void StopNow()
	{
		//!
	}

//Other 
///////////////////////////////
protected:
	inline void SetState(FSState state) { _state = state; }
	inline void SetStateFlag(FSMFlag state_flag) { _fsm_flag = state_flag; }
	inline void SetStateWrong() { _state = static_cast<FSState>(0); }
	inline bool IsProcessing() const { return _is_processing; }
public:
	FSState GetState() const { return _state; }
	inline bool IsWrongState(FSState state) const { return !static_cast<bool>(state); }
	inline FSMFlag GetStateFlag() const { return _fsm_flag; }
	inline int GetStateId(FSState state) const { return static_cast<int>(state); }
	inline int GetEventId(EV event) const { return static_cast<int>(event); }
	SysName GetFSMName() const { return _fsm_name; }
	void SetFSMName(SysName name) { _fsm_name = name; }

	fsm_async& operator<<(tuple<EV, MsgPars> tpl)
	{
		EV event = get<0>(tpl);
		MsgPars params = get<1>(tpl);
		this->_fs_send_event(event, params);
		return *this;
	}
	fsm_async& operator<<(EV event)
	{
		this->_fs_send_event(event);
		return *this;
	}

	friend ostream& operator << (ostream& stream, const fsm_async& fsm_async)
	{
		stream << fsm_async.StateId();
		return stream;
	}

	void PushToLog(MSG msg)
	{
		if (_log_lev == LogLEV::log || _log_lev == LogLEV::log_debug)
		{
			try
			{
				auto funcname = get<1>(_handlers.at(_state));
				Log::Get() <<
					LogOp::indent10 << _owner->GetName() <<
					"** HANDLER = " << LogOp::indent25 << funcname <<
					"** EV = " << LogOp::indent25 << SD::Get().gEventDescByEvent(msg._event) <<
					"** PARS = " << msg._params << "\n";
			}
			catch (const std::exception&)
			{
				assert(!"Error when pushing to log.");
			}
		}
	}
};




#endif




