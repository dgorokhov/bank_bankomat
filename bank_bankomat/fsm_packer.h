#ifndef SYS_DESCRIPTION_H_
#define SYS_DESCRIPTION_H_

//#include <type_traits>
//#include <string>
//#include <queue>
//#include <memory>
//#include <mutex>
//#include <condition_variable>

#include <iostream>
#include <assert.h>
#include <map>
#include <vector>
#include <sstream>

#include "user_events.h"


using namespace std;

//using Desc = std::string;
using SysName = std::string;
using MsgPars = std::string;
using ParValue = std::string;


class fsm_packer;
using SD = fsm_packer;

class fsm_packer
{
public:
	static fsm_packer& Get();
	ParDesc gParDescByPar(PAR par);
	PAR gParByParDesc(ParDesc pardesc);
	EVDesc gEventDescByEvent(EV ev);

	////////////////////////////////////////////////////////////////////////
	//запаковка\распаковка всех параметров из строки (сообщения) и обратно
	////////////////////////////////////////////////////////////////////////
	//распаковать ивернуть набор <имя_параметра>, <значение>
	std::map<PAR, ParDesc> GetUnpackedPars(MsgPars msg);
	//запаковать все пары <ключ>,<значение> в строку сообщения
	MsgPars PackPars(std::map<PAR, ParValue>& params);
	/////////////////////////////////////////////////////////////////////
	// 	   работа с отдельными параметрами 
	////////////////////////////////////////////////////////////////////
	//извлечь пару ключ=значение из упакованной строки
	//добавить обработку ошиибок (если нарушена структура ключ=значение
	//...
	ParDesc GetKeyNValueFromMsg(PAR par, MsgPars msg);
	ParDesc GetValueFromMsg(PAR par, MsgPars msg);
	MsgPars SetKeyNValueForMsg(PAR par, ParValue value);

private:
	char _delimiter{ '&' };
	std::map<PAR, ParDesc> _par_pardesc;
	std::map<EV, EVDesc> _ev_evdesc;

	std::vector<std::string> Split(MsgPars msgpars);
	//добавить пару <код параметра>, <описание пар-ра> в fsm_packer
	//вызывается только из конструктора, определяемго пользователем в fsm_packer.cpp
	void AddParAndDesc(PAR par, ParDesc paramdesc);
	//добавить пару <код события>, <описание события> в fsm_packer
	//вызывается только из конструктора, определяемго пользователем в fsm_packer.cpp
	void AddEventAndDesc(EV event, EVDesc eventdesc);
	fsm_packer();

};



#endif

