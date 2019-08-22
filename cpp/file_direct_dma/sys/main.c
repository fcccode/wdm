#include <wdm.h>

#define DEV_NAME L"\\Device\\MyDriver"
#define SYM_NAME L"\\DosDevices\\MyDriver"

char szBuffer[255]={0};
PDEVICE_OBJECT pNextDevice=NULL;

NTSTATUS AddDevice(PDRIVER_OBJECT pOurDriver, PDEVICE_OBJECT pPhyDevice)
{
  PDEVICE_OBJECT pOurDevice=NULL;
  UNICODE_STRING usDeviceName;
  UNICODE_STRING usSymboName;

  RtlInitUnicodeString(&usDeviceName, DEV_NAME);
  IoCreateDevice(pOurDriver, 0, &usDeviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &pOurDevice);
  RtlInitUnicodeString(&usSymboName, SYM_NAME);
  IoCreateSymbolicLink(&usSymboName, &usDeviceName);
  pNextDevice = IoAttachDeviceToDeviceStack(pOurDevice, pPhyDevice);
  pOurDevice->Flags&= ~DO_DEVICE_INITIALIZING;
  pOurDevice->Flags|= DO_DIRECT_IO;
  return STATUS_SUCCESS;
}

void Unload(PDRIVER_OBJECT pOurDriver)
{
}

NTSTATUS IrpPnp(PDEVICE_OBJECT pOurDevice, PIRP pIrp)
{
  PIO_STACK_LOCATION psk = IoGetCurrentIrpStackLocation(pIrp);
  UNICODE_STRING usSymboName;

  if(psk->MinorFunction == IRP_MN_REMOVE_DEVICE){
    RtlInitUnicodeString(&usSymboName, SYM_NAME);
    IoDeleteSymbolicLink(&usSymboName);
    IoDetachDevice(pNextDevice);
    IoDeleteDevice(pOurDevice);
  }
  IoSkipCurrentIrpStackLocation(pIrp);
  return IoCallDriver(pNextDevice, pIrp);
}

NTSTATUS IrpFile(PDEVICE_OBJECT pOurDevice, PIRP pIrp)
{
  ULONG Len;
  PUCHAR pBuf;
  PIO_STACK_LOCATION psk = IoGetCurrentIrpStackLocation(pIrp);

  switch(psk->MajorFunction){
  case IRP_MJ_CREATE:
    memset(szBuffer, 0, sizeof(szBuffer));
    DbgPrint("IRP_MJ_CREATE");
    break;
  case IRP_MJ_READ:
    pBuf = MmGetMdlVirtualAddress(pIrp->MdlAddress);
    Len = MmGetMdlByteCount(pIrp->MdlAddress);
    memcpy(pBuf, szBuffer, Len);
    DbgPrint("IRP_MJ_READ");
    break;
  case IRP_MJ_WRITE:
    pBuf = MmGetMdlVirtualAddress(pIrp->MdlAddress);
    Len = MmGetMdlByteCount(pIrp->MdlAddress);
    memcpy(szBuffer, pBuf, Len);
    DbgPrint("IRP_MJ_WRITE");
    DbgPrint("Buffer: %s, Length: %d", szBuffer, Len);
    break;
  case IRP_MJ_CLOSE:
    DbgPrint("IRP_MJ_CLOSE");
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
