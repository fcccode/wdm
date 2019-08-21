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

//*** main entry point
int __cdecl main(int argc, char* argv[])
{
  HANDLE hFile = NULL;
  DWORD dwRet = 0;
  char pucBuffer[255]={0};
 
  hFile = CreateFile("\\\\.\\firstStartIO", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
  if (hFile == INVALID_HANDLE_VALUE) {
    printf("failed to open driver\n");
    return 1;
  }
 
  // write data to driver
  sprintf_s(pucBuffer, sizeof(pucBuffer), "This is test message !");
  WriteFile(hFile, pucBuffer, strlen(pucBuffer) + 1, &dwRet, NULL);
  printf("write to driver: %s\n", pucBuffer);
 
  // read data from driver
  ReadFile(hFile, pucBuffer, sizeof(pucBuffer), &dwRet, NULL);
  printf("read from driver: %s\n", pucBuffer);
  CloseHandle(hFile);
  return 0;
}
