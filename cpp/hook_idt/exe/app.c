#define INITGUID
#include <stdio.h>
#include <windows.h>
#include <winioctl.h>
#include <strsafe.h>
#include <setupapi.h>
#include <stdio.h>
#include <stdlib.h>

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
