// AUTHOR: Vicshann

#pragma once

#ifndef StringToolsH
#define StringToolsH

#include "ToolsBase.h"
//===========================================================================
	   
bool    _stdcall IsUnicodeString(PVOID String);
int     _stdcall GetCharCount(LPSTR String, char Par);
int     _stdcall FindEndOfLine(LPSTR Line, int* NextLine=NULL);
int     _stdcall ReplaceChar(LPSTR str, BYTE what, BYTE with);
int     _stdcall CharPosFromLeft(LPSTR String, char Letter, int Index, int Offset);
int     _stdcall CharPosFromRight(LPSTR String, char Letter, int Index, int Offset);

//===========================================================================	   
#endif
