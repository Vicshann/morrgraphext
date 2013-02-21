// AUTHOR: Vicshann

#pragma once

#ifndef FileToolsH
#define FileToolsH

#include "ToolsBase.h"
//===========================================================================
	
#define PATHDLMR 0x2F
#define PATHDLML 0x5C


static _inline bool IsFilePathDelim(BYTE val){return ((val == PATHDLML)||(val == PATHDLMR));} 
											 
void    _stdcall CreateDirectoryPath(LPSTR Path);
UINT    _stdcall TrimFilePath(LPSTR FullPath);
LPSTR   _stdcall GetFileName(LPSTR FullPath);
LPSTR   _stdcall GetFileExt(LPSTR FullPath);
DWORD   _stdcall GetFileNameByHandle(HANDLE hFile, LPSTR Name);
bool    _stdcall FindFile(LPSTR FilePath, LPSTR OutBuffer=NULL);

//===========================================================================	
#endif
