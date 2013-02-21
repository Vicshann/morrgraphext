// AUTHOR: Vicshann

#pragma once

#ifndef ConvertToolsH
#define ConvertToolsH

#include "ToolsBase.h"
//===========================================================================	

long    _stdcall CharToHex(BYTE CharValue); 
WORD    _stdcall HexToChar(BYTE Value, bool UpCase=false);   
DWORD   _stdcall HexStrToDW(LPSTR String);   // Fast, but do not safe
DWORD   _stdcall DecStrToDW(LPSTR String);
DWORD   _stdcall AddrToRelAddr(PVOID CmdAddr, DWORD CmdLen, PVOID TgtAddr);
PVOID   _stdcall RelAddrToAddr(PVOID CmdAddr, DWORD CmdLen, DWORD TgtOffset);
LPSTR   _stdcall ConvertToHexDW(DWORD Value, DWORD MaxDigits, LPSTR Number, bool UpCase);
LPSTR   _stdcall ConvertToDecDW(DWORD Value, LPSTR Number);

//===========================================================================	   
#endif
