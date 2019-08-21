/*========================================================================================================
  Basically, all of files downloaded from my website can be modified or redistributed for any purpose.
  It is my honor to share my interesting to everybody.
  If you find any illeage content out from my website, please contact me firstly.
  I will remove all of the illeage parts.
  Thanks :)
   
  Steward Fu
  g9313716@yuntech.edu.tw
  https://steward-fu.github.io/website/index.htm
========================================================================================================*/
#include <wdm.h>

#define DEV_NAME  L"\\Device\\firstStartIO"
#define SYM_NAME  L"\\DosDevices\\firstStartIO"
#define MSG       "WDM driver tutorial for StartIO"

PDEVICE_OBJECT gNextDevice=NULL;
char szBuffer[1024]={0};
ULONG dwBufferLength=0;

VOID StartIo(struct _DEVICE_OBJECT *pOurDevice, struct _IRP *pIrp)
{
  ULONG dwLen=0;
  PIO_STACK_LOCATION psk = IoGetCurrentIrpStackLocation(pIrp);
  switch(psk->MajorFunction){
  case IRP_MJ_READ:
    dwLen = psk->Parameters.Read.Length;
    strcpy(pIrp->AssociatedIrp.SystemBuffer, szBuffer);
    DbgPrint("StartIo, IRP_MJ_READ\n");
    break;
  case IRP_MJ_WRITE:
    dwLen = psk->Parameters.Write.Length;
    strcpy(szBuffer, pIrp->AssociatedIrp.SystemBuffer);
    DbgPrint("StartIo, IRP_MJ_WRITE\n");
    DbgPrint("StartIo, Buffer: %s\n", szBuffer);
    break;
  }
  pIrp->IoStatus.Status = STATUS_SUCCESS;
  pIrp->IoStatus.Information = dwLen;
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);
  IoStartNextPacket(pOurDevice, FALSE);
}

//*** system will vist this routine when it needs to add new device
NTSTATUS AddDevice(PDRIVER_OBJECT pOurDriver, PDEVICE_OBJECT pPhyDevice)
{
  PDEVICE_OBJECT pOurDevice=NULL;
  UNICODE_STRING usDeviceName;
  UNICODE_STRING usSymboName;

  DbgPrint(MSG);
  RtlInitUnicodeString(&usDeviceName, DEV_NAME);
  IoCreateDevice(pOurDriver, 0, &usDeviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &pOurDevice);
  RtlInitUnicodeString(&usSymboName, SYM_NAME);
  IoCreateSymbolicLink(&usSymboName, &usDeviceName);
  gNextDevice = IoAttachDeviceToDeviceStack(pOurDevice, pPhyDevice);
  pOurDevice->Flags&= ~DO_DEVICE_INITIALIZING;
  pOurDevice->Flags|= DO_BUFFERED_IO;
  return STATUS_SUCCESS;
}
  
//*** it is time to unload our driver
void Unload(PDRIVER_OBJECT pOurDriver)
{
  pOurDriver = pOurDriver;
}

//*** process pnp irp
NTSTATUS IrpPnp(PDEVICE_OBJECT pOurDevice, PIRP pIrp)
{
  PIO_STACK_LOCATION psk = IoGetCurrentIrpStackLocation(pIrp);
  UNICODE_STRING usSymboName;

  if(psk->MinorFunction == IRP_MN_REMOVE_DEVICE){
    RtlInitUnicodeString(&usSymboName, SYM_NAME);
    IoDeleteSymbolicLink(&usSymboName);
    IoDetachDevice(gNextDevice);
    IoDeleteDevice(pOurDevice);
  }
  IoSkipCurrentIrpStackLocation(pIrp);
  return IoCallDriver(gNextDevice, pIrp);
}

//*** process file irp
NTSTATUS IrpFile(PDEVICE_OBJECT pOurDevice, PIRP pIrp)
{
  PIO_STACK_LOCATION psk = IoGetCurrentIrpStackLocation(pIrp);

  switch(psk->MajorFunction){
  case IRP_MJ_CREATE:
    DbgPrint("IrpFile, IRP_MJ_CREATE\n");
    break;
  case IRP_MJ_WRITE:
    DbgPrint("IrpFile, IRP_MJ_WRITE\n");
    IoMarkIrpPending(pIrp);
    IoStartPacket(pOurDevice, pIrp, 0, NULL);
    return STATUS_PENDING;
  case IRP_MJ_READ:
    DbgPrint("IrpFile, IRP_MJ_READ\n");
    IoMarkIrpPending(pIrp);
    IoStartPacket(pOurDevice, pIrp, 0, NULL);
    return STATUS_PENDING;
  case IRP_MJ_CLOSE:
    DbgPrint("IrpFile, IRP_MJ_CLOSE\n");
    break;
  }
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);
  return STATUS_SUCCESS;
}
  
//*** driver entry
NTSTATUS DriverEntry(PDRIVER_OBJECT pOurDriver, PUNICODE_STRING pOurRegistry)
{
  pOurDriver->MajorFunction[IRP_MJ_PNP] = IrpPnp;
  pOurDriver->MajorFunction[IRP_MJ_CREATE] =
  pOurDriver->MajorFunction[IRP_MJ_WRITE] =
  pOurDriver->MajorFunction[IRP_MJ_READ] =
  pOurDriver->MajorFunction[IRP_MJ_CLOSE] = IrpFile;
  pOurDriver->DriverExtension->AddDevice = AddDevice;
  pOurDriver->DriverStartIo = StartIo;
  pOurDriver->DriverUnload = Unload;
  return STATUS_SUCCESS;
}
