#include <wdm.h>
 
#define DEV_NAME  L"\\Device\\firstFile-Neither"
#define SYM_NAME  L"\\DosDevices\\firstFile-Neither"
#define MSG       "File Operation (DO_NEITHER_IO)"
 
char szBuffer[255]={0};
PDEVICE_OBJECT gNextDevice=NULL;
   
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
  return STATUS_SUCCESS;
}
 
void Unload(PDRIVER_OBJECT pOurDriver)
{
}
 
NTSTATUS IrpPnp(PDEVICE_OBJECT pOurDevice, PIRP pIrp)
{
  UNICODE_STRING usSymboName;
  PIO_STACK_LOCATION psk = IoGetCurrentIrpStackLocation(pIrp);
 
  if(psk->MinorFunction == IRP_MN_REMOVE_DEVICE){
    RtlInitUnicodeString(&usSymboName, SYM_NAME);
    IoDeleteSymbolicLink(&usSymboName);
    IoDetachDevice(gNextDevice);
    IoDeleteDevice(pOurDevice);
  }
  IoSkipCurrentIrpStackLocation(pIrp);
  return IoCallDriver(gNextDevice, pIrp);
}
 
NTSTATUS IrpFile(PDEVICE_OBJECT pOurDevice, PIRP pIrp)
{
  ULONG Len;
  PUCHAR pBuf;
  PIO_STACK_LOCATION psk = IoGetCurrentIrpStackLocation(pIrp);
 
  switch(psk->MajorFunction){
  case IRP_MJ_CREATE:
    memset(szBuffer, 0, sizeof(szBuffer));
    DbgPrint("IRP_MJ_CREATE\n");
    break;
  case IRP_MJ_READ:
    pBuf = pIrp->UserBuffer;
    Len = psk->Parameters.Read.Length;
    memcpy(pBuf, szBuffer, Len);
    DbgPrint("IRP_MJ_READ\n");
    DbgPrint("Buf: %s, Len: %d\n", szBuffer, Len);
    break;
  case IRP_MJ_WRITE:
    pBuf = pIrp->UserBuffer;
    Len = psk->Parameters.Write.Length;
    memcpy(szBuffer, pBuf, Len);
    DbgPrint("IRP_MJ_WRITE\n");
    DbgPrint("Buf: %s, Len: %d\n", szBuffer, Len);
    break;
  case IRP_MJ_CLOSE:
    DbgPrint("IRP_MJ_CLOSE\n");
    break;
  }
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);
  return STATUS_SUCCESS;
}
 
NTSTATUS DriverEntry(PDRIVER_OBJECT pOurDriver, PUNICODE_STRING pOurRegistry)
{
  pOurDriver->MajorFunction[IRP_MJ_PNP] = IrpPnp;
  pOurDriver->MajorFunction[IRP_MJ_CREATE] =
  pOurDriver->MajorFunction[IRP_MJ_READ] =
  pOurDriver->MajorFunction[IRP_MJ_WRITE] =
  pOurDriver->MajorFunction[IRP_MJ_CLOSE] = IrpFile;
  pOurDriver->DriverExtension->AddDevice = AddDevice;
  pOurDriver->DriverUnload = Unload;
  return STATUS_SUCCESS;
}