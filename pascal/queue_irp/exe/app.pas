program main;

{$APPTYPE CONSOLE}

uses
  Windows,
  Messages,
  SysUtils,
  Variants,
  Classes,
  Graphics,
  Controls,
  Forms,
  DIALOGS;

const
  METHOD_BUFFERED = 0;
  METHOD_IN_DIRECT = 1;
  METHOD_OUT_DIRECT = 2;
  METHOD_NEITHER = 3;
  FILE_ANY_ACCESS = 0;
  FILE_DEVICE_UNKNOWN = $22;
  IOCTL_QUEUE = (FILE_DEVICE_UNKNOWN shl 16) or (FILE_ANY_ACCESS shl 14) or ($800 shl 2) or (METHOD_BUFFERED);
  IOCTL_PROCESS = (FILE_DEVICE_UNKNOWN shl 16) or (FILE_ANY_ACCESS shl 14) or ($801 shl 2) or (METHOD_BUFFERED);

var
  fd: DWORD;
  ret: DWORD;
  cnt: DWORD;
  ov: array [0..2] of OVERLAPPED;
  
begin
  fd:= CreateFile('\\.\MyDriver', GENERIC_READ or GENERIC_WRITE, FILE_SHARE_READ, Nil, OPEN_EXISTING, FILE_FLAG_OVERLAPPED or FILE_ATTRIBUTE_NORMAL, 0);
  if (fd <> INVALID_HANDLE_VALUE) then
  begin
    for cnt:= 0 to 2 do
    begin
      ov[cnt].hEvent:= CreateEvent(Nil, TRUE, FALSE, Nil);
      WriteLn(Output, 'queue event');
      DeviceIoControl(fd, IOCTL_QUEUE, Nil, 0, Nil, 0, ret, @ov[cnt]);
    end;
    
    WriteLn(Output, 'process all of events');
    DeviceIoControl(fd, IOCTL_PROCESS, Nil, 0, Nil, 0, ret, Nil);
    for cnt:= 0 to 2 do
    begin
      WaitForSingleObject(ov[cnt].hEvent, INFINITE);
      CloseHandle(ov[cnt].hEvent);
      WriteLn(Output, 'wait complete');
    end;
    CloseHandle(fd);
  end else
  begin
    WriteLn(Output, 'failed to open mydriver');
  end;
end.
