;//========================================================================================================
;//  Basically, all of files downloaded from my website can be modified or redistributed for any purpose.
;//  It is my honor to share my interesting to everybody.
;//  If you find any illeage content out from my website, please contact me firstly.
;//  I will remove all of the illeage parts.
;//  Thanks :)
;//  
;//  Steward Fu
;//  g9313716@yuntech.edu.tw
;//  https://steward-fu.github.io/website/index.htm
;//========================================================================================================*/
.386p
.model flat, stdcall
option casemap:none

include c:\masm32\include\w2k\ntstatus.inc
include c:\masm32\include\w2k\ntddk.inc
include c:\masm32\include\w2k\ntoskrnl.inc
include c:\masm32\include\w2k\ntddkbd.inc
include c:\masm32\Macros\Strings.mac
includelib c:\masm32\lib\wxp\i386\ntoskrnl.lib

.const
DEV_NAME word "\","D","e","v","i","c","e","\","f","i","r","s","t","W","D","M",0
SYM_NAME word "\","D","o","s","D","e","v","i","c","e","s","\","f","i","r","s","t","W","D","M",0    

.data
g_pNextDevice PDEVICE_OBJECT 0
 
.code
;//*** system will vist this routine when it needs to add new device
AddDevice proc pOurDriver:PDRIVER_OBJECT, pPhyDevice:PDEVICE_OBJECT
	local pOurDevice:PDEVICE_OBJECT
	local suDevName:UNICODE_STRING
	local szSymName:UNICODE_STRING
	
	invoke DbgPrint, $CTA0("MASM32 WDM driver tutorial for Hello, world")
	invoke RtlInitUnicodeString, addr suDevName, offset DEV_NAME
	invoke RtlInitUnicodeString, addr szSymName, offset SYM_NAME
	invoke IoCreateDevice, pOurDriver, 0, addr suDevName, FILE_DEVICE_UNKNOWN, 0, FALSE, addr pOurDevice
	invoke IoAttachDeviceToDeviceStack, pOurDevice, pPhyDevice
	push eax
	pop g_pNextDevice
	mov eax, pOurDevice
	or (DEVICE_OBJECT PTR [eax]).Flags, DO_BUFFERED_IO
	and (DEVICE_OBJECT PTR [eax]).Flags, not DO_DEVICE_INITIALIZING
	mov eax, STATUS_SUCCESS
	invoke IoCreateSymbolicLink, addr szSymName, addr suDevName
	ret
AddDevice endp

;//*** it is time to unload our driver
Unload proc pOurDriver:PDRIVER_OBJECT
	xor eax, eax
	ret
Unload endp

;//*** process pnp irp
IrpPnp proc uses ebx pOurDevice:PDEVICE_OBJECT, pIrp:PIRP
	local szSymName:UNICODE_STRING
	
	IoGetCurrentIrpStackLocation pIrp
	movzx ebx, (IO_STACK_LOCATION PTR [eax]).MinorFunction
	.if ebx == IRP_MN_START_DEVICE
		mov eax, pIrp
		mov (_IRP PTR [eax]).IoStatus.Status, STATUS_SUCCESS
	.elseif ebx == IRP_MN_REMOVE_DEVICE
		invoke RtlInitUnicodeString, addr szSymName, offset SYM_NAME
		invoke IoDeleteSymbolicLink, addr szSymName     
		mov eax, pIrp
		mov (_IRP PTR [eax]).IoStatus.Status, STATUS_SUCCESS
		invoke IoDetachDevice, g_pNextDevice
		invoke IoDeleteDevice, pOurDevice
	.endif
	IoSkipCurrentIrpStackLocation pIrp
	invoke IoCallDriver, g_pNextDevice, pIrp
	ret
IrpPnp endp

;//*** driver entry
DriverEntry proc pOurDriver:PDRIVER_OBJECT, pOurRegistry:PUNICODE_STRING
	mov eax, pOurDriver
	mov (DRIVER_OBJECT PTR [eax]).MajorFunction[IRP_MJ_PNP * (sizeof PVOID)], offset IrpPnp
	mov (DRIVER_OBJECT PTR [eax]).DriverUnload, offset Unload
	mov eax, (DRIVER_OBJECT PTR [eax]).DriverExtension
	mov (DRIVER_EXTENSION PTR [eax]).AddDevice, AddDevice
	mov eax, STATUS_SUCCESS
	ret
DriverEntry endp
end DriverEntry
.end
