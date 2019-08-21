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

#define IOCTL_THREAD_START  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_THREAD_STOP   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_NEITHER, FILE_ANY_ACCESS)

#define DEV_NAME  L"\\Device\\firstThread"
#define SYM_NAME  L"\\DosDevices\\firstThread"
#define MSG       "WDM driver tutorial for Thread"

PDEVICE_OBJECT gNextDevice=NULL;
volatile BOOLEAN bExitThread=FALSE;
PVOID pThread=NULL;

//*** thread routine
VOID ThreadRoutine(PVOID pParam)
{
  LARGE_INTEGER stTime;

  stTime.HighPart|= -1;
  stTime.LowPart = -10000000;
  DbgPrint("Current process: %s", (char*)((ULONG)IoGetCurrentProcess() + 0x174));
 
  while(bExitThread != TRUE){
      KeDelayExecutionThread(KernelMode, FALSE, &stTime);
      DbgPrint("Sleep 1s");
  }
  DbgPrint("Thread exit");
  PsTerminateSystemThread(STATUS_SUCCESS);
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

//*** process ioctl irp
NTSTATUS IrpIOCTL(PDEVICE_OBJECT pOurDevice, PIRP pIrp)
{
  NTSTATUS status;
  ULONG hThread;
  PIO_STACK_LOCATION psk = IoGetCurrentIrpStackLocation(pIrp);

  switch(psk->Parameters.DeviceIoControl.IoControlCode){
  case IOCTL_THREAD_START:
    DbgPrint("IrpIOCTL, IOCTL_THREAD_START\n");
    bExitThread = FALSE;
    // NtCurrentProcess equ -1
    // NtCurrentProcess will run on user thread area
    // Null wiil run on system area
    status = PsCreateSystemThread(&hThread, THREAD_ALL_ACCESS, NULL, -1, NULL, ThreadRoutine, pOurDevice);
    if(status == STATUS_SUCCESS){
      ObReferenceObjectByHandle(hThread, THREAD_ALL_ACCESS, NULL, KernelMode, &pThread, NULL);
      ZwClose(hThread);
    }
    break;
  case IOCTL_THREAD_STOP:
    DbgPrint("IrpIOCTL, IOCTL_THREAD_STOP\n");
    bExitThread = TRUE;
    if(pThread != NULL){
      KeWaitForSingleObject(pThread, Executive, KernelMode, FALSE, NULL);
      ObDereferenceObject(pThread);
    }
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
