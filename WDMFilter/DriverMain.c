#pragma  once
#include<wdm.h>
#include "zgUnknow.h"
#include"AttachToFDO.h"
#include"Log.h"

#define KBDCLASS_DRIVER_NAME	L"\\driver\\kbdclass"

typedef struct _Device_Extension_{
	PDEVICE_OBJECT LowerDeviceObject;
}*PDEVICE_EXTENSION,DEVICE_EXTENSION;
//���豸����
NTSTATUS AttachToKeyBoard(PDRIVER_OBJECT pDriverObject)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDRIVER_OBJECT kbdclassDrvObj;
	UNICODE_STRING kdbclassDrvName;
	RtlInitUnicodeString(&kdbclassDrvName,KBDCLASS_DRIVER_NAME);
	status = ObReferenceObjectByName(&kdbclassDrvName,OBJ_CASE_INSENSITIVE,NULL,0,*IoDriverObjectType,KernelMode,NULL,(PVOID*)&kbdclassDrvObj);
	if(NT_SUCCESS(status))
	{
		ObDereferenceObject(kbdclassDrvObj);
	}else{
		KdPrint(("ObReferenceObjectByName error \n"));
		goto EndFun;
	}
	
	{//�������豸
		PDEVICE_OBJECT pTargetDeviceObject = kbdclassDrvObj->DeviceObject;
		PDEVICE_OBJECT pFilterDeviceObject = NULL;
		PDEVICE_EXTENSION devExt = NULL;
		while(NULL != pTargetDeviceObject)
		{
			status = IoCreateDevice(
				pDriverObject,
				sizeof(DEVICE_EXTENSION),
				NULL,
				pTargetDeviceObject->DeviceType,
				pTargetDeviceObject->Characteristics,
				FALSE,
				&pFilterDeviceObject
				);
			if(!NT_SUCCESS(status))
			{
				goto EndFun;
			}
			devExt = (PDEVICE_EXTENSION)pFilterDeviceObject->DeviceExtension;
			devExt->LowerDeviceObject = IoAttachDeviceToDeviceStack(pFilterDeviceObject,pTargetDeviceObject);
			if(!(devExt->LowerDeviceObject))
			{
				goto EndFun;
			}

			pFilterDeviceObject->DeviceType=devExt->LowerDeviceObject->DeviceType; 
			pFilterDeviceObject->Characteristics=devExt->LowerDeviceObject->Characteristics; 
			pFilterDeviceObject->StackSize=devExt->LowerDeviceObject->StackSize+1; 
			pFilterDeviceObject->Flags |= devExt->LowerDeviceObject->Flags & (DO_BUFFERED_IO | DO_DIRECT_IO | DO_POWER_PAGABLE) ; 

			pFilterDeviceObject->Flags=pFilterDeviceObject->Flags & ~DO_DEVICE_INITIALIZING;

			pTargetDeviceObject = pTargetDeviceObject->NextDevice;
		}
	}

EndFun:
	return status;

}

NTSTATUS SyPassThrough(IN PDEVICE_OBJECT pDeviceObject,IN PIRP Irp)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_EXTENSION pdevExt = (PDEVICE_EXTENSION)pDeviceObject->DeviceExtension;
	IoSkipCurrentIrpStackLocation(Irp);
	status = IoCallDriver(pdevExt->LowerDeviceObject,Irp);
	return status;
}
NTSTATUS DispatchPnp(IN PDEVICE_OBJECT pDeviceObject,IN PIRP Irp)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_EXTENSION pdevExt = (PDEVICE_EXTENSION)pDeviceObject->DeviceExtension;
	PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
	switch(irpStack->MinorFunction)
	{
	case  IRP_MN_REMOVE_DEVICE:
		IoSkipCurrentIrpStackLocation(Irp);
		IoCallDriver(pdevExt->LowerDeviceObject,Irp);
		IoDetachDevice(pdevExt->LowerDeviceObject);
		pdevExt->LowerDeviceObject = NULL;
		IoDeleteDevice(pDeviceObject);
		status = STATUS_SUCCESS;
		break;
	default:
		IoSkipCurrentIrpStackLocation(Irp);
		status = IoCallDriver(pdevExt->LowerDeviceObject,Irp);
	}
	return status;

}
VOID scanUnload( IN PDRIVER_OBJECT pDriverObject )
{
	PDEVICE_EXTENSION pDev = NULL;
	PDEVICE_OBJECT pDeviceObject = NULL;
	//GagOperClassUsbReg(TRUE,L"myScanDriver\0",L"{4d36e96b-e325-11ce-bfc1-08002be10318}");
	//ж�ذ󶨵��豸����
	pDeviceObject = pDriverObject->DeviceObject;
	while (pDeviceObject)
	{
		PDEVICE_OBJECT pTempDeviceObject = pDeviceObject->NextDevice;
		pDev = (PDEVICE_EXTENSION)pDeviceObject->DeviceExtension;
		IoDetachDevice(pDev->LowerDeviceObject);
		pDev->LowerDeviceObject = NULL;
		IoDeleteDevice(pDeviceObject);
		pDeviceObject = pTempDeviceObject;
	}

	KdPrint(("DriverUnload \n"));
}
NTSTATUS scanAddDevice(
				   __in struct _DRIVER_OBJECT *DriverObject,
				   __in struct _DEVICE_OBJECT *PhysicalDeviceObject
				   )
{
	NTSTATUS status = STATUS_SUCCESS;
	WCHAR buffer[128] = {0};
	int retlen = 0;
	KdPrint(("keykeykeykeykeykeykeykeykeykeykey\n"));
	if(PhysicalDeviceObject == NULL || DriverObject == NULL)
		goto EndFun;
	memset(buffer,0,sizeof(WCHAR)*128);
	status	= IoGetDeviceProperty(PhysicalDeviceObject,DevicePropertyClassName,
		128*sizeof(WCHAR),(PVOID)buffer,&retlen);
	if(!NT_SUCCESS(status))
	{
		goto EndFun;
	}
	KdPrint(("--------- %ws\n",buffer));
	//���µ��豸����
	{
		//����´����豸�Ĺ��˲���
	}
	//�鿴physicalDeviceObject��Ӧ���豸���ơ�
EndFun:
	return status;

}
//�󶨺�ж��

NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject,IN PUNICODE_STRING pRegistryPath) 
{
	NTSTATUS status  = STATUS_SUCCESS;
	int i = 0;
	KdPrint(("gggggggggggggggDriverEntry\n"));
	for(i =0;i<IRP_MJ_MAXIMUM_FUNCTION;i++)
		pDriverObject->MajorFunction[i] = SyPassThrough;

	pDriverObject->MajorFunction[IRP_MJ_READ] ;
	pDriverObject->MajorFunction[IRP_MJ_CLEANUP];
	pDriverObject->MajorFunction[IRP_MJ_WRITE];
	pDriverObject->MajorFunction[IRP_MJ_SHUTDOWN];
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL];
	pDriverObject->MajorFunction[IRP_MJ_POWER];
	pDriverObject->MajorFunction[IRP_MJ_PNP] = DispatchPnp;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE];
	
	pDriverObject->DriverExtension->AddDevice = scanAddDevice; 
	pDriverObject->DriverUnload = scanUnload;

	//���豸����
	//�󶨼��̵��豸����
	status = AttachToKeyBoard(pDriverObject);
	//
	//�ڼ��������棬��ӹ����豸��(���м����࣬������ʱ�򣬻�֪ͨ����������scanAddDevice������
	//�ں���scanAddDevice�У������豸���ﵽ�����豸�Ĺ��ˡ�
	//
	GagOperClassUsbReg(FALSE,L"myScanDriver\0",L"{4d36e96b-e325-11ce-bfc1-08002be10318}");
	return status;
}