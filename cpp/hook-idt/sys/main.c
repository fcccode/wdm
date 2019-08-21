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

#define DEV_NAME        L"\\Device\\firstIDT"
#define SYM_NAME        L"\\DosDevices\\firstIDT"
#define MSG             "WDM driver tutorial for IDT"
#define MAKELONG(a, b) ((unsigned long) (((unsigned short) (a)) | ((unsigned long) ((unsigned short) (b))) << 16)) 

UINT32 oldISRAddress=0;
PDEVICE_OBJECT gNextDevice=NULL;

#pragma pack(1)
typedef struct _DESC {
  UINT16 offset00;
  UINT16 segsel;
  CHAR unused:5;
  CHAR zeros:3;
  CHAR type:5;
  CHAR DPL:2;
  CHAR P:1;
  UINT16 offset16;
} DESC, *PDESC;
#pragma pack()

#pragma pack(1)
typedef struct _IDTR {
  UINT16 bytes;
  UINT32 addr;
} IDTR;
#pragma pack()

IDTR GetIDTAddress()
{
  IDTR idtraddr;
  __asm {
    cli;
    sidt idtraddr;
    sti;
  }
	return idtraddr;
}

PDESC GetDescriptorAddress(UINT16 service)
{
  IDTR idtraddr;
  PDESC descaddr;
	
	idtraddr = GetIDTAddress();
	descaddr = (PDESC)(idtraddr.addr + (service * 0x8));
	return descaddr;
}

UINT32 GetISRAddress(UINT16 service)
{
	PDESC descaddr;
	UINT32 israddr;
  
	descaddr = GetDescriptorAddress(service);
	israddr = descaddr->offset16;
	israddr = israddr << 16;
	israddr+= descaddr->offset00;
	oldISRAddress = israddr;
	return israddr;
}

void MyHook(void)
{
}

UINT32 HookISR(UINT16 service, UINT32 hookaddr)
{
	UINT32 israddr;
	UINT16 hookaddr_low;
	UINT16 hookaddr_high;
	PDESC descaddr;
	
	israddr = GetISRAddress(service);
  descaddr  = GetDescriptorAddress(service);
  hookaddr_low = (UINT16)hookaddr;
  hookaddr = hookaddr >> 16;
  hookaddr_high = (UINT16)hookaddr;
  __asm { cli }
  descaddr->offset00 = hookaddr_low;
  descaddr->offset16 = hookaddr_high;
  __asm { sti }
  DbgPrint("hook complete");
  return israddr;
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
  HookISR(0x29, (UINT32)oldISRAddress);
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

//*** driver entry
NTSTATUS DriverEntry(PDRIVER_OBJECT pOurDriver, PUNICODE_STRING pOurRegistry)
{
  oldISRAddress = HookISR(0x29, (UINT32)MyHook);
  pOurDriver->MajorFunction[IRP_MJ_PNP] = IrpPnp;
  pOurDriver->DriverExtension->AddDevice = AddDevice;
  pOurDriver->DriverUnload = Unload;
  return STATUS_SUCCESS;
}
