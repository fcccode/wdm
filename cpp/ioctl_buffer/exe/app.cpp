#define INITGUID
#include <windows.h>
#include <winioctl.h>
#include <strsafe.h>
#include <setupapi.h>
#include <stdio.h>
#include <stdlib.h>

#define IOCTL_BUFFER_SET CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_BUFFER_GET CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
 
int __cdecl main(int argc, char* argv[])
{
  HANDLE hFile = NULL;
  DWORD dwRet = 0;
  char szBuffer[255]={"Testing Only"};

  hFile = CreateFile("\\\\.\\firstIOCTL-Buffer", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
  if (hFile == INVALID_HANDLE_VALUE) {
    printf("failed to open driver\n");
    return 1;
  }
  DeviceIoControl(hFile, IOCTL_BUFFER_SET, szBuffer, strlen(szBuffer), NULL, 0, &dwRet, NULL);
  memset(szBuffer, 0, sizeof(szBuffer));
  DeviceIoControl(hFile, IOCTL_BUFFER_GET, NULL, 0, szBuffer, sizeof(szBuffer), &dwRet, NULL);
  printf("IOCTL_BUFFER_GET: %s\n", szBuffer);
  CloseHandle(hFile);
  return 0;
}