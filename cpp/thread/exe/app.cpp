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

#define IOCTL_THREAD_START  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_THREAD_STOP   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_NEITHER, FILE_ANY_ACCESS)
 
//*** main entry point
int __cdecl main(int argc, char* argv[])
{
  HANDLE hFile = NULL;
  DWORD dwRet = 0;

  hFile = CreateFile("\\\\.\\firstThread", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
  if (hFile == INVALID_HANDLE_VALUE) {
    printf("failed to open driver\n");
    return 1;
  }
  DeviceIoControl(hFile, IOCTL_THREAD_START, NULL, 0, NULL, 0, &dwRet, NULL);
  Sleep(3000);
  DeviceIoControl(hFile, IOCTL_THREAD_STOP, NULL, 0, NULL, 0, &dwRet, NULL);
  CloseHandle(hFile);
  return 0;
}