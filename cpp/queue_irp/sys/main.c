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

#define IOCTL_QUEUE_IT    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_PROCESS_IT  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_NEITHER, FILE_ANY_ACCESS)

#define DEV_NAME  L"\\Device\\firstQueue"
#define SYM_NAME  L"\\DosDevices\\firstQueue"
#define MSG       "WDM driver tutorial for Queue"

PDEVICE_OBJECT gNextDevice=NULL;
LIST_ENTRY stQueue={0};
KDPC stTimeDPC={0};
KTIMER stTime={0};

//*** timer routine
VOID OnTimer(struct _KDPC *Dpc, PVOID DeferredContext, PVOID SystemArgument1, PVOID SystemArgument2)
{
  PIRP pIrp;
  PLIST_ENTRY plist;
  if(IsListEmpty(&stQueue) == TRUE){
    KeCancelTimer(&stTime);
    DbgPrint("OnTimer: Process successfully");
  }
  else{
    plist = RemoveHeadList(&stQueue);
    pIrp = CONTAINING_RECORD(plist, IRP, Tail.Overlay.ListEntry);
    pIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    DbgPrint("OnTimer: Processing Irp");
  }
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
  
  InitializeListHead(&stQueue);
  KeInitializeTimer(&stTime);
  KeInitializeDpc(&stTimeDPC, OnTimer, pOurDevice);
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

//*** process ioctl irp
NTSTATUS IrpIOCTL(PDEVICE_OBJECT pOurDevice, PIRP pIrp)
{
  LARGE_INTEGER stTimePeriod;
  PIO_STACK_LOCATION psk = IoGetCurrentIrpStackLocation(pIrp);

  switch(psk->Parameters.DeviceIoControl.IoControlCode){
  case IOCTL_QUEUE_IT:
    DbgPrint("IrpIOCTL, IOCTL_QUEUE_IT\n");
    InsertHeadList(&stQueue, &pIrp->Tail.Overlay.ListEntry);
    IoMarkIrpPending(pIrp);
    return STATUS_PENDING;
  case IOCTL_PROCESS_IT:
    DbgPrint("IrpIOCTL, IOCTL_PROCESS_IT\n");
    stTimePeriod.HighPart|= -1;
    stTimePeriod.LowPart = -1000000;
    KeSetTimerEx(&stTime, stTimePeriod, 1000, &stTimeDPC);
    break;
  }
  pIrp->IoStatus.Information = 0;
  pIrp->IoStatus.Status = STATUS_SUCCESS;
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);
  return STATUS_SUCCESS;
}

//*** process file irp
NTSTATUS IrpFile(PDEVICE_OBJECT pOurDevice, PIRP pIrp)
{
  PIO_STACK_LOCATION psk = IoGetCurrentIrpStackLocation(pIrp);

  switch(psk->MajorFunction){
  case IRP_MJ_CREATE:
    DbgPrint("IrpFile, IRP_MJ_CREATE\n");
    break;
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
  pOurDriver->MajorFunction[IRP_MJ_CLOSE] = IrpFile;
  pOurDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IrpIOCTL;
  pOurDriver->DriverExtension->AddDevice = AddDevice;
  pOurDriver->DriverUnload = Unload;
  return STATUS_SUCCESS;
}
