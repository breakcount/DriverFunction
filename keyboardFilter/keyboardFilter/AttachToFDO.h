#pragma  once
#include <wdm.h>
#include<ntstrsafe.h>

/************************************************************************/
/* �������� 
/* ����������
	 bIsDelete ���Ƿ�ж�أ�TRUE: ж�ط���FALSE:��װ����
	 pServiceName ��������
	 pGuid : GUID
/* �������ܣ�
   ��ע����ض��豸�������֪ͨ����
 ���÷�ʽ��ʾ����
 GagOperClassUsbReg(FALSE,L"myScanDriver\0",L"{4d36e96b-e325-11ce-bfc1-08002be10318}");
 ��{4d36e96b-e325-11ce-bfc1-08002be10318}��ӷ���myScanDriver��
/************************************************************************/
VOID GagOperClassUsbReg(int bIsDelete,PWCHAR pServiceName,PWCHAR pGuid);
