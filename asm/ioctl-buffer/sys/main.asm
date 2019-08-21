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
  
public DriverEntry
 
OurDeviceExtension struct
  pNextDev PDEVICE_OBJECT ?
  szBuffer byte 1024 dup(?)
  dwBufferLength dword ?
OurDeviceExtension ends

IOCTL_BUFFER_GET  equ CTL_CODE(FILE_DEVICE_UNKNOWN, 800h, METHOD_BUFFERED, FILE_ANY_ACCESS)
IOCTL_BUFFER_SET  equ CTL_CODE(FILE_DEVICE_UNKNOWN, 801h, METHOD_BUFFERED, FILE_ANY_ACCESS)
 
.const
DEV_NAME word "\","D","e","v","i","c","e","\","f","i","r","s","t","I","O","C","T","L","-","B","u","f","f","e","r",0
SYM_NAME word "\","D","o","s","D","e","v","i","c","e","s","\","f","i","r","s","t","I","O","C","T","L","-","B","u","f","f","e","r",0

.code
;// process file irp
IrpOpenClose proc uses ebx pOurDevice:PDEVICE_OBJECT, pIrp:PIRP
  IoGetCurrentIrpStackLocation pIrp
  movzx ebx, (IO_STACK_LOCATION PTR [eax]).MajorFunction
  .if ebx == IRP_MJ_CREATE
    invoke DbgPrint, $CTA0("IRP_MJ_CREATE")
  .elseif ebx == IRP_MJ_CLOSE
    invoke DbgPrint, $CTA0("IRP_MJ_CLOSE")
  .endif

  mov eax, pIrp
  and (_IRP PTR [eax]).IoStatus.Information, 0
  mov (_IRP PTR [eax]).IoStatus.Status, STATUS_SUCCESS
  fastcall IofCompleteRequest, pIrp, IO_NO_INCREMENT
  mov eax, STATUS_SUCCESS
  ret
IrpOpenClose endp

;// process ioctl irp
IrpIOCTL proc uses ebx ecx pOurDevice:PDEVICE_OBJECT, pIrp:PIRP
  local dwLen: DWORD
  local pdx:PTR OurDeviceExtension
  local pBuf:DWORD

  push 0
  pop dwLen

  mov eax, pOurDevice
  push (DEVICE_OBJECT PTR [eax]).DeviceExtension
  pop pdx

  IoGetCurrentIrpStackLocation pIrp
  mov eax, (IO_STACK_LOCATION PTR [eax]).Parameters.DeviceIoControl.IoControlCode
  .if eax == IOCTL_BUFFER_GET
    invoke DbgPrint, $CTA0("_IOCTL_BUFFER_GET")
    mov eax, pdx
    push (OurDeviceExtension PTR [eax]).dwBufferLength
    pop dwLen

    mov eax, pIrp
    push (_IRP PTR [eax]).AssociatedIrp.SystemBuffer
    pop pBuf

    mov eax, pdx
    invoke memcpy, pBuf, addr (OurDeviceExtension PTR [eax]).szBuffer, dwLen
  .elseif eax == IOCTL_BUFFER_SET
    invoke DbgPrint, $CTA0("_IOCTL_BUFFER_SET")

    IoGetCurrentIrpStackLocation pIrp
    push (IO_STACK_LOCATION PTR [eax]).Parameters.DeviceIoControl.InputBufferLength 
    pop dwLen

    mov eax, pIrp
    push (_IRP PTR [eax]).AssociatedIrp.SystemBuffer
    pop pBuf

    mov eax, pdx
    invoke memcpy, addr (OurDeviceExtension PTR [eax]).szBuffer, pBuf, dwLen

    mov eax, pdx
    push dwLen
    pop (OurDeviceExtension PTR [eax]).dwBufferLength
    invoke DbgPrint, $CTA0("Buf: %s, Len: %d"), pBuf, dwLen
  .endif

  mov eax, pIrp
  mov (_IRP PTR [eax]).IoStatus.Status, STATUS_SUCCESS
  push dwLen
  pop (_IRP PTR [eax]).IoStatus.Information 
  fastcall IofCompleteRequest, pIrp, IO_NO_INCREMENT
  mov eax, STATUS_SUCCESS
  ret
IrpIOCTL endp

;//*** process pnp irp
IrpPnP proc uses ebx pOurDevice:PDEVICE_OBJECT, pIrp:PIRP
  local pdx:PTR OurDeviceExtension
  local szSymName:UNICODE_STRING

  mov eax, pOurDevice
  push (DEVICE_OBJECT PTR [eax]).DeviceExtension
  pop pdx
   
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

    mov eax, pdx
    invoke IoDetachDevice, (OurDeviceExtension PTR [eax]).pNextDev
    invoke IoDeleteDevice, pOurDevice
  .endif
  IoSkipCurrentIrpStackLocation pIrp

  mov eax, pdx
  invoke IoCallDriver, (OurDeviceExtension PTR [eax]).pNextDev, pIrp
  ret
IrpPnP endp

;//*** system will vist this routine when it needs to add new device
AddDevice proc pOurDriver:PDRIVER_OBJECT, pPhyDevice:PDEVICE_OBJECT
	local pOurDevice:PDEVICE_OBJECT
	local suDevName:UNICODE_STRING
	local szSymName:UNICODE_STRING
	
	invoke DbgPrint, $CTA0("MASM32 WDM driver tutorial for Device IOCTL-Buffer")
	invoke RtlInitUnicodeString, addr suDevName, offset DEV_NAME
	invoke RtlInitUnicodeString, addr szSymName, offset SYM_NAME
	
	invoke IoCreateDevice, pOurDriver, sizeof OurDeviceExtension, addr suDevName, FILE_DEVICE_UNKNOWN, 0, FALSE, addr pOurDevice
	.if eax == STATUS_SUCCESS
		invoke IoAttachDeviceToDeviceStack, pOurDevice, pPhyDevice
		.if eax != NULL
			push eax
			mov eax, pOurDevice
			mov eax, (DEVICE_OBJECT PTR [eax]).DeviceExtension
			pop (OurDeviceExtension PTR [eax]).pNextDev

			mov eax, pOurDevice
			or (DEVICE_OBJECT PTR [eax]).Flags, DO_BUFFERED_IO
			and (DEVICE_OBJECT PTR [eax]).Flags, not DO_DEVICE_INITIALIZING
			mov eax, STATUS_SUCCESS
		.else
		mov eax, STATUS_UNSUCCESSFUL
		.endif      
		invoke IoCreateSymbolicLink, addr szSymName, addr suDevName
	.endif
	ret
AddDevice endp
  
;//*** it is time to unload our driver
Unload proc pOurDriver:PDRIVER_OBJECT
	xor eax, eax
	ret
Unload endp

;//*** driver entry
DriverEntry proc pOurDriver:PDRIVER_OBJECT, pOurRegistry:PUNICODE_STRING
	mov eax, pOurDriver
	mov (DRIVER_OBJECT PTR [eax]).MajorFunction[IRP_MJ_PNP    * (sizeof PVOID)], offset IrpPnP
	mov (DRIVER_OBJECT PTR [eax]).MajorFunction[IRP_MJ_CREATE * (sizeof PVOID)], offset IrpOpenClose
	mov (DRIVER_OBJECT PTR [eax]).MajorFunction[IRP_MJ_CLOSE  * (sizeof PVOID)], offset IrpOpenClose
	mov (DRIVER_OBJECT PTR [eax]).MajorFunction[IRP_MJ_DEVICE_CONTROL * (sizeof PVOID)], offset IrpIOCTL
	mov (DRIVER_OBJECT PTR [eax]).DriverUnload, offset Unload
	mov eax, (DRIVER_OBJECT PTR [eax]).DriverExtension
	mov (DRIVER_EXTENSION PTR [eax]).AddDevice, AddDevice
	mov eax, STATUS_SUCCESS
	ret
DriverEntry endp
end DriverEntry
.end
