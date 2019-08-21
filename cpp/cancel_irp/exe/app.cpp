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
#include <DriverSpecs.h>
__user_code  
#define INITGUID
#include <windows.h>
#include <winioctl.h>
#include <strsafe.h>
#include <setupapi.h>
#include <stdio.h>
#include <stdlib.h>
#define WHILE(a) __pragma(warning(suppress:4127)) while(a)

#define IOCTL_QUEUE_IT    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_PROCESS_IT  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_NEITHER, FILE_ANY_ACCESS)
 
//*** main entry point
int __cdecl main(int argc, char* argv[])
{
  int i=0;
  HANDLE hFile = NULL;
  DWORD dwRet = 0;
  OVERLAPPED ov={0};

  hFile = CreateFile("\\\\.\\firstCancel", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE) {
    printf("failed to open driver\n");
    return 1;
  }
  for(i=0; i<3; i++){
    memset(&ov, 0, sizeof(ov));
    ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    DeviceIoControl(hFile, IOCTL_QUEUE_IT, NULL, 0, NULL, 0, &dwRet, &ov);
  }
  DeviceIoControl(hFile, IOCTL_PROCESS_IT, NULL, 0, NULL, 0, &dwRet, NULL);
  Sleep(1000);
  CancelIo(hFile);
  Sleep(10000);
  CloseHandle(hFile);
  return 0;
}