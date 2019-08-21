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
#include <stdio.h>
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
  __try{
    __asm int 0x29
  }
  __except(EXCEPTION_EXECUTE_HANDLER){
    printf("SEH catched the exception!\n");
  }
  return 0;
}
