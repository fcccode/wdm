unit main;

interface
  uses
    DDDK;
    
  const
    DEV_NAME = '\Device\MyDriver';
    SYM_NAME = '\DosDevices\MyDriver';
    IOCTL_START = (FILE_DEVICE_UNKNOWN shl 16) or (FILE_ANY_ACCESS shl 14) or ($800 shl 2) or (METHOD_BUFFERED);
    IOCTL_STOP = (FILE_DEVICE_UNKNOWN shl 16) or (FILE_ANY_ACCESS shl 14) or ($801 shl 2) or (METHOD_BUFFERED);

  function _DriverEntry(pOurDriver:PDRIVER_OBJECT; pOurRegistry:PUNICODE_STRING):NTSTATUS; stdcall;

implementation
var
  cnt: ULONG;
  pNextDevice: PDEVICE_OBJECT;

procedure OnTimer(pOurDevice:PDeviceObject; pContext:Pointer); stdcall;
begin
  cnt:= cnt + 1;
  DbgPrint('IoTimer: %d', [cnt]);
end;

procedure Unload(pOurDriver:PDRIVER_OBJECT); stdcall;
begin
end;

function IrpFile(pOurDevice:PDEVICE_OBJECT; pIrp:PIRP):NTSTATUS; stdcall;
var
  psk: PIO_STACK_LOCATION;
  
begin
  psk:= IoGetCurrentIrpStackLocation(pIrp);
  case psk^.MajorFunction of
  IRP_MJ_CREATE:
    DbgPrint('IRP_MJ_CREATE', []);
  IRP_MJ_CLOSE:
    DbgPrint('IRP_MJ_CLOSE', []);
  end;
  
  Result:= STATUS_SUCCESS;
  pIrp^.IoStatus.Status:= Result;
  pIrp^.IoStatus.Information:= 0;
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);
end;

function IrpIOCTL(pOurDevice:PDeviceObject; pIrp:PIrp):NTSTATUS; stdcall;
var
  code: ULONG;
  psk: PIO_STACK_LOCATION;
   
begin
  psk:= IoGetCurrentIrpStackLocation(pIrp);
  code:= psk^.Parameters.DeviceIoControl.IoControlCode;
  case code of
  IOCTL_START:begin
    DbgPrint('IOCTL_START', []);
    cnt:= 0;
    IoStartTimer(pOurDevice);
  end;
  IOCTL_STOP:begin
    DbgPrint('IOCTL_STOP', []);
    IoStopTimer(pOurDevice);
  end;
  end;
   
  Result:= STATUS_SUCCESS;
  pIrp^.IoStatus.Information:= 0;
  pIrp^.IoStatus.Status:= Result;
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);
end;

function IrpPnp(pOurDevice:PDEVICE_OBJECT; pIrp:PIRP):NTSTATUS; stdcall;
var
  psk: PIO_STACK_LOCATION;
  suSymName: UNICODE_STRING;
  
begin
  psk:= IoGetCurrentIrpStackLocation(pIrp);
  if psk^.MinorFunction = IRP_MN_REMOVE_DEVICE then
  begin
    RtlInitUnicodeString(@suSymName, SYM_NAME);
    IoDetachDevice(pNextDevice);
    IoDeleteDevice(pOurDevice);
    IoDeleteSymbolicLink(@suSymName);
  end;
  IoSkipCurrentIrpStackLocation(pIrp);
  Result:= IoCallDriver(pNextDevice, pIrp);
end;

function AddDevice(pOurDriver:PDRIVER_OBJECT; pPhyDevice:PDEVICE_OBJECT):NTSTATUS; stdcall;
var
  suDevName: UNICODE_STRING;
  suSymName: UNICODE_STRING;
  pOurDevice: PDEVICE_OBJECT;
  
begin
  RtlInitUnicodeString(@suDevName, DEV_NAME);
  RtlInitUnicodeString(@suSymName, SYM_NAME);
  IoCreateDevice(pOurDriver, 0, @suDevName, FILE_DEVICE_UNKNOWN, 0, FALSE, pOurDevice);
  pNextDevice:= IoAttachDeviceToDeviceStack(pOurDevice, pPhyDevice);
  pOurDevice^.Flags:= pOurDevice^.Flags or DO_BUFFERED_IO;
  pOurDevice^.Flags:= pOurDevice^.Flags and not DO_DEVICE_INITIALIZING;
  IoInitializeTimer(pOurDevice, OnTimer, Nil);
  Result:= IoCreateSymbolicLink(@suSymName, @suDevName);
end;

function _DriverEntry(pOurDriver:PDRIVER_OBJECT; pOurRegistry:PUNICODE_STRING):NTSTATUS; stdcall;
begin
  pOurDriver^.MajorFunction[IRP_MJ_PNP]:= @IrpPnp;
  pOurDriver^.MajorFunction[IRP_MJ_CREATE]:= @IrpFile;
  pOurDriver^.MajorFunction[IRP_MJ_CLOSE]:= @IrpFile;
  pOurDriver^.MajorFunction[IRP_MJ_DEVICE_CONTROL] := @IrpIOCTL;
  pOurDriver^.DriverExtension^.AddDevice:=@AddDevice;
  pOurDriver^.DriverUnload:=@Unload;
  Result:=STATUS_SUCCESS;
end;
end.
