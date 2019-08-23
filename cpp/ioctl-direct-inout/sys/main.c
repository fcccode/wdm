#include <wdm.h>

#define IOCTL_SET CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_IN_DIRECT, FILE_ANY_ACCESS)
#define IOCTL_GET CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define DEV_NAME L"\\Device\\MyDriver"
#define SYM_NAME L"\\DosDevices\\MyDriver"

PDEVICE_OBJECT pNextDevice=NULL;
char szBuffer[255]={0};

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
  pOurDevice->Flags|= DO_BUFFERED_IO;
  return STATUS_SUCCESS;
}
  
void Unload(PDRIVER_OBJECT pOurDriver)
{
  pOurDriver = pOurDriver;
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

NTSTATUS IrpIOCTL(PDEVICE_OBJECT pOurDevice, PIRP pIrp)
{
  ULONG Len=0;
  PUCHAR pBuf;
  PIO_STACK_LOCATION psk = IoGetCurrentIrpStackLocation(pIrp);

  switch(psk->Parameters.DeviceIoControl.IoControlCode){
  case IOCTL_SET:
    DbgPrint("IOCTL_SET");
    Len = psk->Parameters.DeviceIoControl.InputBufferLength;
    memcpy(szBuffer, pIrp->AssociatedIrp.SystemBuffer, Len);
    DbgPrint("Buffer: %s, Length: %d", szBuffer, Len);
    break;
  case IOCTL_GET:
    DbgPrint("IOCTL_GET");
    pBuf = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, LowPagePriority);
    Len = strlen(szBuffer)+1;
    memcpy(pBuf, szBuffer, Len);
    break;
  }
  pIrp->IoStatus.Status = STATUS_SUCCESS;
  pIrp->IoStatus.Information = Len;
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);
  return STATUS_SUCCESS;
}

NTSTATUS IrpFile(PDEVICE_OBJECT pOurDevice, PIRP pIrp)
{
  PIO_STACK_LOCATION psk = IoGetCurrentIrpStackLocation(pIrp);

  switch(psk->MajorFunction){
  case IRP_MJ_CREATE:
    DbgPrint("IRP_MJ_CREATE");
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
  pOurDriver->MajorFunction[IRP_MJ_CLOSE] = IrpFile;
  pOurDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IrpIOCTL;
  pOurDriver->DriverExtension->AddDevice = AddDevice;
  pOurDriver->DriverUnload = Unload;
  return STATUS_SUCCESS;
}
