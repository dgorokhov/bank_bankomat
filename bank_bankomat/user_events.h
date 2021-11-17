#ifndef EVENT_TYPE_H
#define EVENT_TYPE_H

// Тип перечисление (СОБЫТИЯ СИСТЕМЫ)
// является общим для всех участников обмена
// Какие-то из событий используются только одним классом
// Некоторые несколькими и пр

enum class BankBankomatEVENT {
	//reserved - dont change
	stop_processing = 0,
	stop_now = 1,
	empty_event = 2,
	//back_to_initial = 2,
	//user eventS
	card_inserted = 10,		//bankomat
	got_pin,				//bank
	pin_correct,			//bankomat
	pin_wrong,				//bankomat
	got_withdraw_amount,	//bank
	client_took_money,		//bank
	client_forgot_money,	//bank
	enough_money,			//bankomat
	not_enough_money,		//bankomat
	account_updated	,		//
	shutdown_processing,
	bankomat_stopped,
	
	//money_saved,
	//card_ejected,			
};

enum class BankBankomatPars
{
	no_param = 0,
	pin_code,
	card_number,
	withdraw_amount,
};


using EV = BankBankomatEVENT;
using PAR = BankBankomatPars;

using EVDesc = std::string;
using ParDesc = std::string;


#endif