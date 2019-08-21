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
	pNextDev        PDEVICE_OBJECT ?
	szBuffer        BYTE 1024 dup(?)
  dwBufferLength  DWORD ?
OurDeviceExtension ends

.const
DEV_NAME word "\","D","e","v","i","c","e","\","f","i","r","s","t","S","t","a","r","t","I","O",0
SYM_NAME word "\","D","o","s","D","e","v","i","c","e","s","\","f","i","r","s","t","S","t","a","r","t","I","O",0

.code
;//*** startio routine
StartIo proc uses ebx pOurDevice:PDEVICE_OBJECT, pIrp:PIRP
	local pdx:PTR OurDeviceExtension
	local dwLen:DWORD
	local pBuf:DWORD
	
	mov eax, pOurDevice
	push (DEVICE_OBJECT PTR [eax]).DeviceExtension
	pop pdx
	
	mov eax, pIrp
	push (_IRP PTR [eax]).AssociatedIrp.SystemBuffer
	pop pBuf
	
	IoGetCurrentIrpStackLocation pIrp
	Push (IO_STACK_LOCATION PTR [eax]).Parameters.Write._Length
	pop dwLen
	movzx ebx, (IO_STACK_LOCATION PTR [eax]).MajorFunction
	.if ebx == IRP_MJ_WRITE
		invoke DbgPrint, $CTA0("StartIo: IRP_MJ_WRITE")
		mov eax, pdx
		push dwLen
		pop (OurDeviceExtension PTR [eax]).dwBufferLength
		invoke memcpy, addr (OurDeviceExtension PTR [eax]).szBuffer, pBuf, dwLen
		invoke DbgPrint, $CTA0("len: %d"), dwLen
	.elseif ebx == IRP_MJ_READ
		invoke DbgPrint, $CTA0("StartIo: IRP_MJ_READ")
		mov eax, pdx
		push (OurDeviceExtension PTR [eax]).dwBufferLength
		pop dwLen
		invoke memcpy, pBuf, addr (OurDeviceExtension PTR [eax]).szBuffer, dwLen
		invoke DbgPrint, $CTA0("buf: %s"), pBuf
	.endif
	
	mov eax, pIrp
	mov (_IRP PTR [eax]).IoStatus.Status, STATUS_SUCCESS
	push dwLen
	pop (_IRP PTR [eax]).IoStatus.Information
	fastcall IofCompleteRequest, pIrp, IO_NO_INCREMENT
	invoke IoStartNextPacket, pOurDevice, FALSE
	mov eax, STATUS_SUCCESS
	ret
StartIo endp

;//*** process file irp
IrpFile proc uses ebx pOurDevice:PDEVICE_OBJECT, pIrp:PIRP
  IoGetCurrentIrpStackLocation pIrp
  movzx ebx, (IO_STACK_LOCATION PTR [eax]).MajorFunction
  .if (ebx == IRP_MJ_CREATE) || (ebx == IRP_MJ_CLOSE) 
    .if (ebx == IRP_MJ_CREATE)
      invoke DbgPrint, $CTA0("IrpFile: IRP_MJ_CREATE")
    .else
      invoke DbgPrint, $CTA0("IrpFile: IRP_MJ_CLOSE")
    .endif
		mov eax, pIrp
		and (_IRP PTR [eax]).IoStatus.Information, 0
		mov (_IRP PTR [eax]).IoStatus.Status, STATUS_SUCCESS
		fastcall IofCompleteRequest, pIrp, IO_NO_INCREMENT
    mov eax, STATUS_SUCCESS
  .elseif (ebx == IRP_MJ_READ) || (ebx == IRP_MJ_WRITE)
		IoGetCurrentIrpStackLocation pIrp
    .if (ebx == IRP_MJ_READ)
      invoke DbgPrint, $CTA0("IrpFile: IRP_MJ_READ")
    .else
      invoke DbgPrint, $CTA0("IrpFile: IRP_MJ_WRITE")
    .endif
		IoMarkIrpPending pIrp
		invoke IoStartPacket, pOurDevice, pIrp, 0, NULL
		mov eax, STATUS_PENDING
  .endif
  ret
IrpFile endp

;//*** process pnp irp
IrpPnP proc uses ebx pDevObj:PDEVICE_OBJECT, pIrp:PIRP
  local pdx:PTR OurDeviceExtension
  local szSymName:UNICODE_STRING

  mov eax, pDevObj
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
    invoke IoDeleteDevice, pDevObj
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

  invoke DbgPrint, $CTA0("MASM32 WDM driver tutorial for StartIO")
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
	mov (DRIVER_OBJECT PTR [eax]).MajorFunction[IRP_MJ_CREATE * (sizeof PVOID)], offset IrpFile
	mov (DRIVER_OBJECT PTR [eax]).MajorFunction[IRP_MJ_CLOSE  * (sizeof PVOID)], offset IrpFile
	mov (DRIVER_OBJECT PTR [eax]).MajorFunction[IRP_MJ_WRITE  * (sizeof PVOID)], offset IrpFile
  mov (DRIVER_OBJECT PTR [eax]).MajorFunction[IRP_MJ_READ   * (sizeof PVOID)], offset IrpFile
  mov (DRIVER_OBJECT PTR [eax]).DriverStartIo, offset StartIo
	mov (DRIVER_OBJECT PTR [eax]).DriverUnload, offset Unload
	mov eax, (DRIVER_OBJECT PTR [eax]).DriverExtension
	mov (DRIVER_EXTENSION PTR [eax]).AddDevice, AddDevice
	mov eax, STATUS_SUCCESS
	ret
DriverEntry endp
end DriverEntry
.end
