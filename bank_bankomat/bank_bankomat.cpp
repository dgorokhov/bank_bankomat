
#include <algorithm>
#include "fsm_template.h"


///////////////////////////
// TODO
///////////////////////////
// остальноев порядке убыв важности
// -сделать пример межпроцессным (каждый банкоман из своейго потока)
// -сделать пример межкомпютерным
//



#include "fsm_macros_define"





using namespace std;


void Bank_Bankomat()
{

/*
* Заготовка класса-участника (Member)
* 
*	//набор состояний $Member$
	enum class $State$
	{
		wrong_state = 0,
		$state1$,
		$state2$,
		...
	}
	

	class $Member$ : public system_member
	{
	private:
		fsm_async<$State$>* _fsm;
	protected:
		auto FSM() { return _fsm; };
	public:
		virtual ~$Member$()
		{
			delete _fsm;
		}
		void PushToInsideQueues(MSG msg) override
		{
			_fsm->PushToInsideQueues(msg);
		}
		void onStopEvent() override
		{
			//действия которые необходимо выполнит прежде 
			//чем закончить обработку своих сообщений
			...
			_mq->IAmDone(_name);
		}
		void ToInitialState() override
		{
			//Каждый чучастник системы д. вызвать это первой строкой
			FSM()->ToFSMInitialState();
			//привести участника в исходное состояние (initial_state)
			//....
		}

		//обработчик состояния
		HNDL($state1$)
		{
			...
		}
		//обработчик состояния
		HNDL($state2$)
		{
			...
		}
		...

		$Member$(SysName name, $State$ initial, message_queue& mq, LogLEV level) :
			system_member(name, mq, level)
		{
			_fsm = new fsm_async<$State$>(name, initial, this, level);
			mq.AddSystemMember(name, this);

			//добавить обработчики в КА (т.е. в fsm, принадлежащий данному классу)
			ADDHDL($State$, &$Member$, $state1$, "$state1$");
			ADDHDL($State$, &$Member$, $state2$, "$state2$");
			...
			this->ToInitialState();
		}
		
		...specific code for $Member$
		public:
		...
		private:

	};

	*/


	enum class bkST {
		wrong_state = 0,
		waiting_orders,
		checking_pin,
		checking_withdraw_ok,
		updating_account,
		shutting_down,
		on_stopping_bankomat,
	};

	class Bank : public system_member
	{
	private:
		fsm_async<bkST>* _fsm;
	protected:
		auto FSM() { return _fsm; };
	public:
		virtual ~Bank()
		{
			delete _fsm;
		}
		void PushToInsideQueues(MSG msg) override
		{
			_fsm->PushToInsideQueues(msg);
		}
		void onStopEvent() override
		{
			_mq->IAmDone(_name);
		}
		void ToInitialState() override
		{
			//Каждый чучастник системы д. вызвать это первой строкой
			FSM()->ToFSMInitialState();
			//инициализация для initial_state
			//....
		}
	private:
		///////////////////////////////////////////////
		//для примера -база данных карт 
		//////////////////////////////////////////////
		class CardDB {

			std::map<ParDesc, tuple<ParDesc, int>> _cards
			{	// номер карты + пинкод + сумма на счете
				{"1111-1111-1111-1111",make_tuple("1111",100)},
				{"2222-2222-2222-2222",make_tuple("2222",200)},
				{"3333-3333-3333-3333",make_tuple("3333",300)},
				{"4444-4444-4444-4444",make_tuple("4444",400)},
			};
		public:
			ParDesc GetPinByCard(ParDesc card)
			{
				try
				{
					auto tpl = _cards[card];
					return get<0>(tpl);
				}
				catch (exception&)
				{
					return "";
				}
			}
			int GetAccountSumByCard(ParDesc card)
			{
				try
				{
					auto tpl = _cards[card];
					return get<1>(tpl);
				}
				catch (exception&)
				{
					return -1;
				}
			}
			void WithdrawAmount(ParDesc card, int amount)
			{
				try
				{
					auto tpl = _cards[card];
					auto newtuple = make_tuple(get<0>(tpl), get<1>(tpl) - amount);
					_cards[card] = newtuple;
				}
				catch (exception&)
				{
				}
			}
		};

		//структуры для данного класса
		std::map<SysName, bool> _bnkmts;
		CardDB _db;

	public:

		void AddBankomat(system_member* bankomat)
		{
			//запомнить имя банкомата и пометить его работающим
			_bnkmts[bankomat->GetName()] = true;
		}

		//ОБРАБОТЧИКИ СОСТОЯНИЙ (ПО ОДНОМУ НА КАЖДОЕ СОСТОЯНИЕ)
		HNDL(waiting_orders)
		{
			M_TO_LOG;
			switch (msg._event)
			{
			case EV::got_pin:
				FSM()->HandleNewState(bkST::checking_pin, msg); //params=cardinfo+pin
				break;
			case EV::got_withdraw_amount:
				FSM()->HandleNewState(bkST::checking_withdraw_ok, msg);		//params=cardinfo+pin+amount
				break;
			case EV::client_took_money:
				FSM()->HandleNewState(bkST::updating_account, msg);			//params=cardinfo+pin+amount
				break;
			case EV::shutdown_processing:
				FSM()->HandleNewState(bkST::shutting_down, msg);
				break;
			case EV::bankomat_stopped:
				FSM()->HandleNewState(bkST::on_stopping_bankomat, msg);
				break;
			default:

				break;
			}
		}
		HNDL(checking_pin)
		{
			M_TO_LOG;
			if (GET_VAL(pin_code) == _db.GetPinByCard(GET_VAL(card_number)))
			{
				FSM()->PushAsServer(msg._sysm_from, EV::pin_correct, msg._params);
				ToInitialState();
			}
			else {
				FSM()->PushAsServer(msg._sysm_from, EV::pin_wrong, msg._params);
				ToInitialState();
			}
		}
		HNDL(checking_withdraw_ok)
		{
			M_TO_LOG;
			int withdraw_amount = dig::from_str<int>(GET_VAL(withdraw_amount));
			if (withdraw_amount <= _db.GetAccountSumByCard(GET_VAL(card_number)))
			{
				// enough money
				FSM()->PushAsServer(msg._sysm_from, EV::enough_money, msg._params);
				ToInitialState();
			}
			else {
				//not enough money
				FSM()->PushAsServer(msg._sysm_from, EV::not_enough_money, msg._params);
				ToInitialState();
			}
		}
		HNDL(updating_account)
		{
			M_TO_LOG;
			int withdraw_amount = dig::from_str<int>(GET_VAL(withdraw_amount));
			_db.WithdrawAmount(GET_VAL(card_number), withdraw_amount);
			std::cout << ("\n" + _name + " client account was changed, account sum = "
				+ dig::to_str<int>(_db.GetAccountSumByCard(GET_VAL(card_number))) + "\n");
			ToInitialState();

		}
		HNDL(shutting_down)
		{
			for (auto bankomat : _bnkmts)
			{
				M_TO_LOG;
				FSM()->PushAsClient(bankomat.first, EV::stop_processing);
				bankomat.second = false;
			}
			ToInitialState();
		}
		HNDL(on_stopping_bankomat)
		{
			M_TO_LOG;
			ToInitialState();

			//пометить банкомат остановленным
			_bnkmts[msg._sysm_from->GetName()] = false;
			//проверить все ли остановлены
			bool someworking = false;
			for (auto bnkmt : _bnkmts)
			{
				if (bnkmt.second) {
					someworking = true; break;
				}
			}
			if (!someworking)
				//если все банкоматы закончили обслуживание 
				//остановить процессинг банка
				FSM()->PushAsClient(_name, EV::stop_processing);
		}

		Bank(SysName name, bkST initial, message_queue& mq, LogLEV level = LogLEV::log) :
			system_member(name, mq, level)
		{
			_fsm = new fsm_async<bkST>(name, initial, this, level);
			mq.AddSystemMember(name, this);

			//добавить обработчики в КА (т.е. в fsm, принадлежащий данному классу)
			ADDHDL(bkST, &Bank, waiting_orders, "waiting_orders");
			ADDHDL(bkST, &Bank, checking_pin, "checking_pin");
			ADDHDL(bkST, &Bank, checking_withdraw_ok, "checking_withdraw_ok");
			ADDHDL(bkST, &Bank, updating_account, "updating_account");
			ADDHDL(bkST, &Bank, shutting_down, "shutting_down");
			ADDHDL(bkST, &Bank, on_stopping_bankomat, "on_stopping_bankomat");


			this->ToInitialState();
		}
	};



	////
	////Bankomat
	////

	enum class btST {
		wrong_state = 0,
		waiting_for_card,
		waiting_for_pin,
		waiting_bank_reply_pin,
		waiting_bank_reply_amount,
		ejecting_card,
		requesting_amount,
		waiting_if_money_avail,
		issuing_money,
		getting_money_back,
	};


	class Bankomat : public system_member
	{
	private:
		fsm_async<btST>* _fsm;
	protected:
		auto FSM() { return _fsm; }
	public:
		virtual ~Bankomat()
		{
			delete _fsm;
			//delete ifstr;
		}
		void PushToInsideQueues(MSG msg) override
		{
			_fsm->PushToInsideQueues(msg);
		}
		void onStopEvent() override
		{
			FSM()->PushAsClient(_bank, EV::bankomat_stopped);
			_mq->IAmDone(_name);
		}
		void ToInitialState() override
		{
			//Каждый участник должен вызвать этот метод в своем ToInitialState()
			FSM()->ToFSMInitialState();

			//вся нужная участнику инициализация для initial state
			_withdraw_amount = "";
			_pin = "";
			_card_number = "";
			_attempts = 3;
		}

	private:
		//т.к. банкомат обрабатывает 1 карту за раз
		//то можно сохранить промежуточные данные:
		string _pin{};
		string _card_number{};
		string _withdraw_amount{};
		int _attempts{ 3 };
		SysName _bank{ "bank" };
		//ifstr -для демонстрации - (объяснеие в конструкторе)
		ifstream* ifstr;

		//ОБРАБОТЧИКИ СОСТОЯНИЙ (ПО ОДНОМУ НА КАЖДОЕ СОСТОЯНИЕ)
		HNDL(waiting_for_card)
		{
			M_TO_LOG;
			_card_number = GET_VAL(card_number);
			FSM()->HandleNewState(btST::waiting_for_pin, msg);
		}

		HNDL(waiting_for_pin)
		{
			M_TO_LOG;
			if (_attempts) {
				_card_number = GET_VAL(card_number);
				std::cout << ("\n" + _name + " Processing card : " + GET_VAL(card_number) + "\nType pin code : \n");
				getline((*ifstr), _pin);
				std::cout << ("\n" + _name + " Checking pin... \n");
				FSM()->ChangeStateAndWait(btST::waiting_bank_reply_pin);
				FSM()->PushAsClient(_bank, EV::got_pin, SET_KEY_VAL(card_number, _card_number) + SET_KEY_VAL(pin_code, _pin));
				--_attempts;
			}
			else
			{
				std::cout << "\n" + _name + " Attempts ran out!\n";
				FSM()->HandleNewState(btST::ejecting_card);
			}

		}
		HNDL(waiting_bank_reply_pin)
		{
			M_TO_LOG;
			switch (msg._event)
			{
			case EV::pin_correct:
				FSM()->HandleNewState(btST::requesting_amount);

				break;
			case EV::pin_wrong:
				if (_attempts)
					FSM()->HandleNewState(btST::waiting_for_pin);
				else
				{
					std::cout << "\n" + _name + " Pin check attempts ran out!\n";
					FSM()->HandleNewState(btST::ejecting_card);
				}
				break;
			default:
				break;
			}
		}
		HNDL(requesting_amount)
		{
			M_TO_LOG;
			std::cout << ("\n" + _name + " Type amount : ");
			getline((*ifstr), _withdraw_amount);
			FSM()->ChangeStateAndWait(btST::waiting_bank_reply_amount);
			FSM()->PushAsClient(_bank, EV::got_withdraw_amount,
				SET_KEY_VAL(card_number, _card_number) +
				SET_KEY_VAL(pin_code, _pin) +
				SET_KEY_VAL(withdraw_amount, _withdraw_amount));
		}
		HNDL(waiting_bank_reply_amount)
		{
			M_TO_LOG;
			switch (msg._event)
			{
			case EV::enough_money:
				FSM()->HandleNewState(btST::issuing_money);
				break;
			case EV::not_enough_money:
				FSM()->HandleNewState(btST::ejecting_card);
				break;
			default:
				break;
			}
		}
		HNDL(issuing_money)
		{
			M_TO_LOG;
			std::cout << ("\n" + _name + " Issuing money...\n");
			string iftook{};
			getline((*ifstr), iftook);
			if (iftook == "t") {
				FSM()->PushAsClient(_bank, EV::client_took_money,
					SET_KEY_VAL(card_number, _card_number) +
					SET_KEY_VAL(pin_code, _pin) +
					SET_KEY_VAL(withdraw_amount, _withdraw_amount));
				FSM()->HandleNewState(btST::ejecting_card);
			}
			else {
				std::cout << ("\n" + _name + " You forgot money\nGet the card back!\n");
				FSM()->HandleNewState(btST::getting_money_back);
			}
		}
		HNDL(getting_money_back)
		{
			M_TO_LOG;
			std::cout << ("\n" + _name + " Taking money back...\n");
			FSM()->HandleNewState(btST::ejecting_card);
		}
		HNDL(ejecting_card)
		{
			M_TO_LOG;
			std::cout << "\n" + _name + " Get the card back!";
			std::cout << "\n" + _name + " Card ejected.\n";
			ToInitialState();
		}

	public:

		Bankomat(SysName name, btST initial, message_queue& mq, LogLEV level = LogLEV::log) :
			system_member(name, mq, level)
		{
			_fsm = new fsm_async<btST>(name, initial, this, level);
			mq.AddSystemMember(name, this);

			ADDHDL(btST, &Bankomat, waiting_for_card, "waiting_for_card");
			ADDHDL(btST, &Bankomat, waiting_for_pin, "waiting_for_pin");
			ADDHDL(btST, &Bankomat, waiting_bank_reply_pin, "waiting_bank_reply_pin");
			ADDHDL(btST, &Bankomat, requesting_amount, "requesting_amount");
			ADDHDL(btST, &Bankomat, waiting_bank_reply_amount, "waiting_bank_reply_amount");
			ADDHDL(btST, &Bankomat, ejecting_card, "ejecting_card");
			ADDHDL(btST, &Bankomat, issuing_money, "issuing_money");
			ADDHDL(btST, &Bankomat, getting_money_back, "getting_money_back");

			this->ToInitialState();

			//строка внизу - исключительно для целей демонстрации.
			//Чтобы исключить мешанину на экране от одновременного вывода сообщений
			//от разных банкоматов, каждый из них считывает ответы клиента банка из "своего" потока (файла)
			// (это bankomat1.in и bankmat2.in)
			
			ifstr = new ifstream(_name + ".in");
		}
	};

	Log::Get().Add(std::this_thread::get_id(), "main");
	message_queue mq{ LogLEV::log };
	Bank bank("bank", bkST::waiting_orders, mq);
	Bankomat bankomat1("bankomat1", btST::waiting_for_card, mq);
	Bankomat bankomat2("bankomat2", btST::waiting_for_card, mq);

	bank.AddBankomat(&bankomat1);
	bank.AddBankomat(&bankomat2);

	mq.MQPush(nullptr, &bankomat1, EV::card_inserted, MSGTo::extl, "&card_number=1111-1111-1111-1111");
	mq.MQPush(nullptr, &bankomat2, EV::card_inserted, MSGTo::extl, "&card_number=2222-2222-2222-2222");

	mq.MQPush(nullptr, &bank, EV::shutdown_processing, MSGTo::extl);
	mq.WaitForMQDone();

	Log::Get() << "main is over";
	std::cout << "\n" << "program end" << '\n';
}



int main()
{
	Bank_Bankomat();

}

#include "fsm_macros_undef"


