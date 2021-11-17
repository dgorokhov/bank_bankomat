
#include "fsm_packer.h"





//////////////////////////////////////////////
// fsm_packer
//////////////////////////////////////////////

using namespace std;

fsm_packer& fsm_packer::Get()
{
	static fsm_packer sd;
	return sd;
}
ParDesc fsm_packer::gParDescByPar(PAR par)
{
	try
	{
		return _par_pardesc.at(par);
	}
	catch (const std::exception&)
	{
		return "Error when trying to get parameter description. No parameter!";
	};
}

PAR fsm_packer::gParByParDesc(ParDesc pardesc)
{
	auto it = find_if(
		_par_pardesc.cbegin(),
		_par_pardesc.cend(),
		[=](const pair<PAR, ParDesc>& elem)
		{ if (elem.second == pardesc) return true; else return false; });

	return (it != _par_pardesc.cend() ? it->first : PAR::no_param);
}

EVDesc fsm_packer::gEventDescByEvent(EV ev)
{
	try
	{
		return _ev_evdesc.at(ev);
	}
	catch (const std::exception&)
	{
		assert(!"Error when trying to get event desription. No event!");
	}
}

////////////////////////////////////////////////////////////////////////
//запаковка\распаковка всех параметров из строки (сообщения) и обратно
////////////////////////////////////////////////////////////////////////
//распаковать ивернуть набор <имя_параметра>, <значение>
std::map<PAR, ParDesc> fsm_packer::GetUnpackedPars(MsgPars msg)
{
	std::map<PAR, ParDesc> returnmap;
	vector<string> keyvalues = Split(msg);
	for (const auto elem : keyvalues)
	{
		string pardesc{};
		string value{};
		auto pos = elem.find('=');
		if (pos != std::string::npos)
		{
			pardesc = elem.substr(0, pos);
			value = elem.substr(pos + 1, elem.length() - pos);
			returnmap[gParByParDesc(pardesc)] = value;
		}
	}
	return returnmap;
}

//запаковать все пары <ключ>,<значение> в строку сообщения
MsgPars fsm_packer::PackPars(std::map<PAR, ParValue>& params)
{
	MsgPars msgpars{};
	for (auto i : params)
	{
		msgpars += _delimiter + gParDescByPar(i.first) + "=" + i.second;
	}
	return msgpars;
}

/////////////////////////////////////////////////////////////////////
// 	   работа с отдельными параметрами 
////////////////////////////////////////////////////////////////////

//извлечь пару ключ=значение из упакованной строки
//добавить обработку ошиибок (если нарушена структура ключ=значение
//...
ParDesc fsm_packer::GetKeyNValueFromMsg(PAR par, MsgPars msg)
{
	//auto pardesc = gParDescByPar(par);
	auto pos = msg.find(gParDescByPar(par));
	if (pos != std::string::npos)
	{
		auto reststr = msg.substr(pos, msg.length() - pos);
		auto endpos = reststr.find(_delimiter);
		if (endpos != std::string::npos)
			//параметр не последний в строке
			return reststr.substr(0, endpos);
		else
			//параметр последний в строке
			return reststr.substr(0, reststr.length());
	}
	else
		return "";
}

ParDesc fsm_packer::GetValueFromMsg(PAR par, MsgPars msg)
{
	auto keyvalue = GetKeyNValueFromMsg(par, msg);
	if (keyvalue == "") return "";
	auto pos = keyvalue.find('=');
	if (pos != std::string::npos)
		return keyvalue.substr(pos + 1, keyvalue.length());
	else
		return "";
}

MsgPars fsm_packer::SetKeyNValueForMsg(PAR par, ParValue value)
{
	return (_delimiter + gParDescByPar(par) + "=" + value);
}

std::vector<std::string> fsm_packer::Split(MsgPars msgpars)
{
	std::stringstream ss(msgpars);
	std::string item;
	std::vector<std::string> keyvalues{};
	while (std::getline(ss, item, _delimiter))
	{
		keyvalues.push_back(item);
	}
	return keyvalues;
}

//добавить пару <код параметра>, <описание пар-ра> в fsm_packer
//вызывается только из конструктора, определяемго пользователем в fsm_packer.cpp
void fsm_packer::AddParAndDesc(PAR par, ParDesc paramdesc)
{
	try
	{	//ошибка если есть такой par 
		auto temp = _par_pardesc.at(par);
		assert(!"Error when trying to insert event parameter. Parameter exists.");
	}
	catch (const std::exception&)
	{
		//ок, добавляем
		_par_pardesc[par] = paramdesc;
	}
	
}

//добавить пару <код события>, <описание события> в fsm_packer
//вызывается только из конструктора, определяемго пользователем в fsm_packer.cpp
void fsm_packer::AddEventAndDesc(EV event, EVDesc eventdesc)
{
	try
	{	//ошибка если есть такой par 
		auto temp = _ev_evdesc.at(event);
		assert(!"Error when trying to insert event with description. Event exists.");
	}
	catch (const std::exception&)
	{
		//ок, добавляем
		_ev_evdesc[event] = eventdesc;
	}
}


#define ADDEVENT(ev, evdesc) \
	AddEventAndDesc(EV::##ev, ##evdesc);

#define ADDPARAM(par, pardesc) \
	AddParAndDesc(PAR::##par, ##pardesc);

fsm_packer::fsm_packer()
{
#include "user_descriptions"
}

#define ADDEVENT(ev, evdesc) 
#define ADDPARAM(par, pardesc)





