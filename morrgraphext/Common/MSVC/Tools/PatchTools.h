// AUTHOR: Vicshann

#pragma once

#ifndef PatchToolsH
#define PatchToolsH

#include "ToolsBase.h"
#include "ProcessTools.h"
#include "ConvertTools.h"
//===========================================================================	   

PVOID   _stdcall FindMemSignatureLocal(PVOID Address, LPSTR Signature);
DWORD   _stdcall MemoryPatchLocal(PVOID PatchAddr, PVOID PatchData, DWORD PatchSize, LPSTR Signature);
DWORD   _stdcall MemoryPatchMaskLocal(PVOID PatchAddr, LPSTR PatchMask, LPSTR Signature);
DWORD   _stdcall SetFunctionPatch(HMODULE Module, DWORD Offset, PVOID FuncAddr, BYTE CmdCode, DWORD AddNOP, LPSTR Signature);
DWORD   _stdcall SetProcRedirection(PVOID Address, PVOID TargetAddr, LPSTR Signature=NULL);
bool    _stdcall PatchBySignature(PBYTE Buffer, UINT BufSize, LPSTR OriginalSig, LPSTR PatchSig, bool PatchAll);
bool    _stdcall IsMemSignatureMatch(PVOID Address, LPSTR Signature, int SigLen=-1);

//===========================================================================	   
#endif
