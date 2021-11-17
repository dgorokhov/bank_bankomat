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
	//���������\���������� ���� ���������� �� ������ (���������) � �������
	////////////////////////////////////////////////////////////////////////
	//����������� �������� ����� <���_���������>, <��������>
	std::map<PAR, ParDesc> GetUnpackedPars(MsgPars msg);
	//���������� ��� ���� <����>,<��������> � ������ ���������
	MsgPars PackPars(std::map<PAR, ParValue>& params);
	/////////////////////////////////////////////////////////////////////
	// 	   ������ � ���������� ����������� 
	////////////////////////////////////////////////////////////////////
	//������� ���� ����=�������� �� ����������� ������
	//�������� ��������� ������� (���� �������� ��������� ����=��������
	//...
	ParDesc GetKeyNValueFromMsg(PAR par, MsgPars msg);
	ParDesc GetValueFromMsg(PAR par, MsgPars msg);
	MsgPars SetKeyNValueForMsg(PAR par, ParValue value);

private:
	char _delimiter{ '&' };
	std::map<PAR, ParDesc> _par_pardesc;
	std::map<EV, EVDesc> _ev_evdesc;

	std::vector<std::string> Split(MsgPars msgpars);
	//�������� ���� <��� ���������>, <�������� ���-��> � fsm_packer
	//���������� ������ �� ������������, ������������ ������������� � fsm_packer.cpp
	void AddParAndDesc(PAR par, ParDesc paramdesc);
	//�������� ���� <��� �������>, <�������� �������> � fsm_packer
	//���������� ������ �� ������������, ������������ ������������� � fsm_packer.cpp
	void AddEventAndDesc(EV event, EVDesc eventdesc);
	fsm_packer();

};



#endif

