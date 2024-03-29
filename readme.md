  
## **FSM (Finite State Machine) REST-like framework.**   

��������� ��������� ��� ���������� ������, ��������� ��� �������� �������� (��)  
(Finite State Machines) ����������������� ����� ����� �� ����� ������-������.  
�������������� ���������� ����� ������ ���������� ����������� ����� ����� ������� ���������.  
������ �� ���������� ������� � ������� �������� � ����� �������,  
��� ��������� �������� ���������������� �.�. �� ����� ����� ������.    

� ����� ������ ��������� ����� ��������� ��������� ��� ������������� � ������ ������  
��������, �� ��������������� �������� ������������� � ��������������� �������  
�.�. ����������� �������� ������ ����:  
**&param1=value1&param2=value2...**. 
����������� ������ ��� �������� ��������� ��������� ��   
���� ������������� ������������ (enum).    

## **��������**

����� ���������� (������ ����������):  
|���������|��������  
|--|--|
|**fsm_template.h**| ������ ��������� �������� ��������������� �����-������� ��������� ��  
|**fsm_structs.h,** **fsm_structs.cpp**| �������� �������� ��������� ���������:  
||**system_member** - �����-������ �� ��� ���������� �������  
||**message_queue** - ���������������� ������� ���������  
|| **MSG** - ��������� ����������� ������ ���������  
|**fsm_log.h, fsm_log.cpp**| ������� ��� ������� ��������� ������� ���� ��� � ������  
||**��� ������ ���������� ��--��� ���������.log**  
|**fsm_packer.h**, **fsm_packer.cpp**| �����-���������\����������� ��������� ���������� ���������   
|**user_events.h**|������������ (enum), ����������� �������, �������� ������������ ���������  
|**user_descriptions**| ��������� ������������ �����:
|| �������� (����� enum) � ��� ��������� ��������������  
||������������ ����� ���������� ������� (����� enum) � ��� ��������� ��������������    

� **�������� ����� �������** (����������������) ����� ������� ���������� �������  
��� ����������� ������ system, ���������� ����� ��������� ��� ������� ���������,  
� �������� ���������� ��� ������� ���������. ���������� ���������� ���   
�������� ������� ��������� �� �������� ���������, �������� � ���� ���������.    

���������� ������, ����������� � ���������������� ���� **(bank_bankomat.cpp)**,  
��������� ������� ������������� **������� ���� + ��������1 + ��������2**,  
���������� � ����� ��������.  

![������� ����-��������](https://github.com/dgorokhov/sandbox/raw/master/bank-bankomat.gif)


**������� ����-��������**
