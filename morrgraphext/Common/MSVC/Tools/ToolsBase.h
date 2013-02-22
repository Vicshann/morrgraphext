// AUTHOR: Vicshann

#pragma once

#ifndef ToolsBaseH
#define ToolsBaseH

#define WIN32_LEAN_AND_MEAN		 // Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT  0x0500     // For CoInitializeEx support

#include <Windows.h>
#include <tlhelp32.h>   // needed for tool help declarations
#include "LogTools.h"
//===========================================================================	   

#ifndef TOOLCALL
#define TOOLCALL  _stdcall
#endif

#define DBGMSGINF(msg,...) _LogInfLine(msg,__VA_ARGS__) 
#define DBGMSGERR(msg,...) _LogErrLine(msg,__VA_ARGS__)
#define DBGMSGWRN(msg,...) _LogWrnLine(msg,__VA_ARGS__)     

//===========================================================================	   
#endif
