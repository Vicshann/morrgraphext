// AUTHOR: Vicshann

#pragma once

#ifndef ProcessToolsH
#define ProcessToolsH

#include "ToolsBase.h"
#include "ConvertTools.h"
#include "StringTools.h"
#include "ModuleTools.h"    
//====================================================================================

#undef PUNICODE_STRING
struct UNICODE_STRING 
{
 USHORT Length;
 USHORT MaximumLength;
 PWSTR  Buffer;
}; 
typedef	UNICODE_STRING *PUNICODE_STRING;

/*struct UNISTRING
{
 USHORT Length;
 USHORT MaximumLength;
 PWSTR  Buffer;
};*/

typedef	UNICODE_STRING UNISTRING;

struct PROCPARAMS
{
 ULONG     AllocationSize;
 ULONG     ActualSize;
 ULONG     Flags;
 ULONG     Unknown1;
 ULONG     Unknown2;
 ULONG     Unknown3;
 HANDLE    InputHandle;
 HANDLE    OutputHandle;
 HANDLE    ErrorHandle;
 UNISTRING CurrentDirectory;
 HANDLE    CurrentDir;
 UNISTRING SearchPaths;
 UNISTRING ApplicationName;
 UNISTRING CommandLine;
 PVOID     EnvironmentBlock;
 ULONG     Unknown[9];
 UNISTRING Unknown4;
 UNISTRING Unknown5;
 UNISTRING Unknown6;
 UNISTRING Unknown7;
};

struct PEBL
{
 ULONG       AllocationSize;
 ULONG       Unknown1;
 HANDLE      ProcessInstance;
 PVOID       DllList;
 PROCPARAMS* ProcessParameters;
 ULONG       Unknown2;
 HANDLE      DefaultHeap;
};

struct PROCBASICINFO
{
 PVOID  Reserved1;
 PEBL*  PebBaseAddress;
 PVOID  Reserved2[2];
 ULONG* UniqueProcessId;
 PVOID  Reserved3;
};
//---------------------------------------------------------------------------
typedef long (_stdcall *LDR_LOAD_DLL)(PWCHAR PathToFile, ULONG Flags, PUNICODE_STRING ModuleFileName, PHANDLE ModuleHandle);
//====================================================================================

HMODULE _stdcall GetOwnerModule(PVOID Address);
PVOID   _stdcall FindProcEntry(HMODULE Module, LPSTR ProcName);
PVOID   _stdcall FindProcAddress(HMODULE Module, LPSTR ProcName);
DWORD   _stdcall GetRealModuleSize(HANDLE hProcess, DWORD ModuleBase);
DWORD   _stdcall WriteLocalProtectedMemory(PVOID Address, PVOID Data, DWORD DataSize, bool RestoreProt);
DWORD   _stdcall WriteProtectedMemory(DWORD ProcessID, HANDLE hProcess, PVOID Address, PVOID Data, DWORD DataSize, bool RestoreProt);
DWORD   _stdcall UpdateProcessImports(PVOID OldProcAddr, PVOID NewProcAddr, BOOL OnlyLocalModules);
DWORD   _stdcall UpdateModuleImports(PVOID OldProcAddr, PVOID NewProcAddr, HMODULE hTgtModule);
PVOID   _stdcall FindLocalModule(PVOID *ModuleBase, DWORD *ModuleSize);
BOOL    _stdcall SetDbgFlag(BOOL Flag);
long    _stdcall GetProcessPathByID(DWORD ProcessID, LPSTR PathBuffer, long BufferLngth);
long    _stdcall GetProcessPathByHandle(HANDLE hProcess, LPSTR PathBuffer, long BufferLngth);

//====================================================================================
#endif
