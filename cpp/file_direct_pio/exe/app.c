#define INITGUID
#include <windows.h>
#include <strsafe.h>
#include <setupapi.h>
#include <stdio.h>
#include <stdlib.h>

int __cdecl main(int argc, char* argv[])
{
  HANDLE hFile = NULL;
  DWORD dwRet = 0;
  char pucBuffer[255]={0};

  hFile = CreateFile("\\\\.\\firstFile-Direct-PIO", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
  if (hFile == INVALID_HANDLE_VALUE) {
    printf("failed to open driver\n");
    return 1;
  }

  strncpy(pucBuffer, "Testing Only", sizeof(pucBuffer));
  WriteFile(hFile, pucBuffer, strlen(pucBuffer) + 1, &dwRet, NULL);
  printf("WR: %s\n", pucBuffer);

  ReadFile(hFile, pucBuffer, sizeof(pucBuffer), &dwRet, NULL);
  printf("RD: %s\n", pucBuffer);
  CloseHandle(hFile);
  return 0;
}
