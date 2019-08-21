#include <wdm.h>

#define IOCTL_DIRECT_SET CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
#define IOCTL_DIRECT_GET CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define DEV_NAME  L"\\Device\\firstIOCTL-Direct-Out"
#define SYM_NAME  L"\\DosDevices\\firstIOCTL-Direct-Out"
#define MSG       "WDM driver tutorial for IOCTL-Direct-Out"

PDEVICE_OBJECT gNextDevice=NULL;
char szBuffer[255]={0};

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
    IoDetachDevice(gNextDevice);
    IoDeleteDevice(pOurDevice);
  }
  IoSkipCurrentIrpStackLocation(pIrp);
  return IoCallDriver(gNextDevice, pIrp);
}

NTSTATUS IrpIOCTL(PDEVICE_OBJECT pOurDevice, PIRP pIrp)
{
  ULONG Len=0;
  PUCHAR pBuf;
  PIO_STACK_LOCATION psk = IoGetCurrentIrpStackLocation(pIrp);

  switch(psk->Parameters.DeviceIoControl.IoControlCode){
  case IOCTL_DIRECT_SET:
    DbgPrint("IrpIOCTL, IOCTL_DIRECT_SET\n");
    Len = psk->Parameters.DeviceIoControl.InputBufferLength;
    memcpy(szBuffer, pIrp->AssociatedIrp.SystemBuffer, Len);
    DbgPrint("IrpIOCTL, Buf: %s, Len: %d\n", szBuffer, Len);
    break;
  case IOCTL_DIRECT_GET:
    DbgPrint("IrpIOCTL, IOCTL_DIRECT_GET\n");
    pBuf = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, LowPagePriority);
    Len = psk->Parameters.DeviceIoControl.OutputBufferLength;
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
    DbgPrint("IrpFile, IRP_MJ_CREATE\n");
    break;
  case IRP_MJ_CLOSE:
    DbgPrint("IrpFile, IRP_MJ_CLOSE\n");
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
