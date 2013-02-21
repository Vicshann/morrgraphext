/*

Copyright (C) 2004-2005 Angel Death 

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the included (LICENSE.txt) GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
//////////////////////////////////////////////////////////////////////
// Console.cpp
// -------------------------------------------------------------------
// $Id: Console.cpp,v 1.1 2005-09-06 19:02:30 AngelD Exp $ 
// Rewritten: 2013:10:10 Vicshann
//////////////////////////////////////////////////////////////////////

#include "ExtensionMod.h"
#include <dinput.h>

using namespace MODULENAME;

extern MODULENAME::PSModData  ModInfo;
//====================================================================================
const DWORD Version = MAKELONG(3,00);

struct GAMECOMMANDSTRUCT
{
 char*	szName;
 BOOL	(_stdcall *pFunc)(char** argv, int argc);
 char*	szUsage;
};
//====================================================================================

GAMECOMMANDSTRUCT* _stdcall FindGameCommand(char* name);
BOOL _stdcall GameCommandLineHelp(char** argv, int argc);
BOOL _stdcall GameCommandLineVersion(char** argv, int argc);
BOOL _stdcall GameCommandLineCredits(char** argv, int argc);
BOOL _stdcall GameCommandLinePriority(char** argv, int argc);
BOOL _stdcall GameCommandLineFPS(char** argv, int argc);
BOOL _stdcall GameCommandLineDump(char** argv, int argc);
BOOL _stdcall GameCommandLinePrn(char** argv, int argc);
//====================================================================================

GAMECOMMANDSTRUCT GameCommands[] = {
	{
	 "help",
	 GameCommandLineHelp,
	 " .help Выдает список команд"},
	{
	 "version",
	 GameCommandLineVersion,
	 " .version Displays detailed version information."},
	{
	 "dump",
	 GameCommandLineDump,
	 " .dump <f|d|u|w|b|s> <adress>"},
	{
	 "priority",
	 GameCommandLinePriority,
	 " .priority <1|2|3> Set process priority to <Normal|High|Realtime>"},
	{
	 "fps",
	  GameCommandLineFPS,
	 " .fps max <maxfps> Set maxfps\n"
	 },
	{
	 NULL,
	 }
};
//====================================================================================
static BYTE TBL_KEY[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xdd, 0x00, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x00, 0x00, 0x00, 0xe1, 0x00, 0xfe, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC6, 0xE6, 0xc1, 0x00, 0xde, 0x00,
	0x00, 0xd4, 0xc8, 0xd1, 0xc2, 0xd3, 0xc0, 0xcf, 0xd0, 0xd8, 0xce, 0xcb, 0xc4, 0xdc, 0xd2, 0xd9,
	0xc7, 0xc9, 0xca, 0xdb, 0xc5, 0xc3, 0xcc, 0xd6, 0xd7, 0xcd, 0xdf, 0xf5, 0x00, 0xfa, 0x00, 0x00,
	0x00, 0xf4, 0xe8, 0xf1, 0xe2, 0xf3, 0xe0, 0xef, 0xf0, 0xf8, 0xee, 0xeb, 0xe4, 0xfc, 0xf2, 0xf9,
	0xe7, 0xe9, 0xea, 0xfb, 0xe5, 0xe3, 0xec, 0xf6, 0xf7, 0xed, 0xff, 0xd5, 0x00, 0xda, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
//====================================================================================
// Returns pointer to a COMMANDSTRUCT
//====================================================================================
GAMECOMMANDSTRUCT* _stdcall FindGameCommand( char *name )
{
 GAMECOMMANDSTRUCT *gcs = GameCommands;
 while ( gcs->szName )
  {
   if ( !lstrcmpi( gcs->szName, name ) )return gcs;
   gcs++;
  }
 return NULL;
}
//====================================================================================
// This is where we take a command line, tokenize it, and send it to the right destination :)
//====================================================================================
BOOL _stdcall GameCommandLine( char *raw )
{
 char   *p;
 char    buf[1024];
 char   *argv[1024];
 int     argc = 0;
 GAMECOMMANDSTRUCT *gcs;

 if ( 0 == raw )return FALSE;

 // Parse through the command line for tokens, string characters, and the command separator.  
 wsprintf( buf, "%s", raw );  // Copy the command line to a temporary buffer so we can mangle it.

 // Initialize a few pointers to it.
 argv[0] = p = buf;
 while ( *p <= ' ' && *p != 0 )p++;	
 if ( *p != '.' )return FALSE;  //esli ne '.' to ne nasha komanda
 p++;	
 if ( *p == 0 )return FALSE; // Did we reach the end of the command line already?
	
 // Make sure we're starting with argc count properly set.
 // This is important if we've encountered multiple commands via the command separator.
 argc = 0;
 argv[argc++] = p;

 // Reset the loop flag.  This will only get set to true when we find another command to execute i.e. via the command separator.
 
 //game_con_history(raw,1,console_hren);
 while ( *p != 0 )
  {	
   if ( *p <= ' ' )  // Spaces are our token separator.
	{	 
	 *( p++ ) = 0; // Set the first space encountered to null.
	 while ( *p <= ' ' && *p != 0 )p++; // Parse past the rest until we get to the start of the next token.			
	 if ( *p != 0 )argv[argc++] = p; // If we're not at the end of cmdline then this must be the start of the next token.
	} 
     else p++; // Parse through the token.
  }
 gcs = FindGameCommand( argv[0] );
	
 // Is this a built-in function ?
 if ( gcs )
  {		
   if ( !gcs->pFunc( argv, argc ) )GameConOut( "Usage:%s", gcs->szUsage ); // If functions returns false, show usage help
  } 
   else GameConOut( "Unknown command:%s", argv[0] );

 return TRUE;
}
//====================================================================================
// Handles command line help
//====================================================================================
BOOL _stdcall GameCommandLineHelp( char **argv, int argc )
{
 if ( argc > 1 )return FALSE;
 GameConOut( "Available AngelConsole commands:" );   // Header
 for ( int i = 0; GameCommands[i].szName != NULL; i++ )GameConOut( " .%s", GameCommands[i].szName );	
 return TRUE;
}
//====================================================================================
//
//====================================================================================
BOOL _stdcall GameCommandLineVersion( char **argv, int argc )
{
 GameConOut( "Version %d.%.3d (%s@%s)", LOWORD( Version ), HIWORD( Version ), __DATE__, __TIME__ );
 return TRUE;
}
//====================================================================================
//
//====================================================================================
BOOL _stdcall GameCommandLineCredits( char **argv, int argc )
{
 return FALSE;
}
//====================================================================================
//
//====================================================================================
BOOL _stdcall GameCommandLinePriority( char **argv, int argc )
{
 if ( argc != 2 )return FALSE;
 switch ( atoi( argv[1] ) )
  {
   case 1:
	 if ( SetPriorityClass( GetCurrentProcess(), NORMAL_PRIORITY_CLASS ) )GameConOut( "Priority set to Normal" );
	   else GameConOut( "Cannon set priority" );
	break;
   case 2:
	 if ( SetPriorityClass( GetCurrentProcess(), HIGH_PRIORITY_CLASS ) )GameConOut( "Priority set to High" );
	   else GameConOut( "Cannon set priority" );
	break;
   case 3:
	 if ( SetPriorityClass( GetCurrentProcess(), REALTIME_PRIORITY_CLASS ) )GameConOut( "Priority set to Realtime" );
	   else GameConOut( "Cannon set priority" );
	break;
   default: return FALSE;
  }
 return TRUE;
}
//====================================================================================
//
//====================================================================================
BOOL _stdcall GameCommandLineDump(char** argv, int argc)
{
 if ( argc == 2 && !lstrcmp( argv[1], "help" ) )return FALSE;
 if ( argc != 3 ){GameConOut( "game_state adress: %8.8X", *ModInfo->MWGlobals.ppGameScene );return TRUE;}
	
 char   *endptr;
 DWORD  adress = strtol( argv[2], &endptr, 16 );

 if ( endptr - argv[2] == 0 || adress < 0x400000 ){GameConOut( "incorrect adress: %s", argv[2] );return TRUE;}
 switch ( argv[1][0] )
  {
   case 'f':
	 GameConOut( "%8.8X: %f", adress, *( float * ) adress );
	break;
   case 'd':
	 GameConOut( "%8.8X: %8.8X", adress, *( double * ) adress );
	break;
   case 'u':
	 GameConOut( "%8.8X: %8.8X", adress, *( DWORD * ) adress );
	break;
   case 'w':
	 GameConOut( "%8.8X: %8.8X", adress, *( WORD * ) adress );
	break;
   case 'b':
	 GameConOut( "%8.8X: %2.2X", adress, *( BYTE * ) adress );
	break;
   case 's':
	 GameConOut( "%8.8X: %s", adress, ( char * ) adress );
	break;
   default: return FALSE;
  }
 return TRUE;
}
//====================================================================================
//
//====================================================================================
BOOL _stdcall GameCommandLinePrn( char **argv, int argc )
{
 if ( argc != 4 )return FALSE;
 ModInfo->MWGlobals.ShowPopUpMessage( argv[1], atoi( argv[2] ), atoi( argv[3] ) );
 return TRUE;
}
//====================================================================================
//
//====================================================================================
BOOL _stdcall GameCommandLineFPS( char **argv, int argc )
{
 if ( argc == 1 )return FALSE;
 if ( !lstrcmpi( argv[1], "max" ) )
  {
   MORROWIND::PCGameScene gscene = *ModInfo->MWGlobals.ppGameScene; 
   if ( argc == 3 )
	{
	 gscene->MaxFPS = atof( argv[2] );   
	 GameConOut( "Set max fps to %f", gscene->MaxFPS );
	} 
     else GameConOut( "maxfps=%f, currentfps=%f", gscene->MaxFPS, gscene->CurrentFPS );
   return TRUE;
  }
 return FALSE;
}
//====================================================================================
// incializaciya console
//====================================================================================
int _stdcall InitConsole(void)
{
 int Result = 0;
 Result += ModInfo->CodeHooks[ModInfo->HookIndex++].SetCmdValue(&ModInfo->PatchBase[0x0008C6B6],GetAddress<PVOID>(&CConsoleInput::__ParseInputString),"E8A563F7FFA1");  // ReplaceCode( INST_JMP, hook_addr_console, ( DWORD ) & ParseConsoleSTUB, 6 );	//fix in pathes  
 if(Result)LogMsg("Errors while initializing: %u", Result);
 return Result; 
}
//====================================================================================
//
//====================================================================================
bool MWTHISCALL CConsoleInput::__ParseInputString(void)
{
 if(this->_ParseInputString())return false;
 if(GameCommandLine(this->InputStr2))return false;   // InputStr2 is a copy done by '_ParseInputString'!
 return true;  
}  
//====================================================================================
//                                  INPUT
//====================================================================================
//====================================================================================
// ebx - string // Char? vicshann
//====================================================================================
BOOL _cdecl EnterCharSTUB(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD Reg_EDI,HVL DWORD Reg_ESI,HVL DWORD Reg_EBP,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL DWORD Reg_EAX,...)
{
 if(!ModInfo->EnglishInput && (Reg_EBX < 0x100) && TBL_KEY[Reg_EBX])Reg_EBX = TBL_KEY[Reg_EBX];
 return TRUE;
}
//====================================================================================
// ebx - char
//====================================================================================
BOOL _cdecl EnterCharMultiStringSTUB(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD Reg_EDI,HVL DWORD Reg_ESI,HVL DWORD Reg_EBP,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL DWORD Reg_EAX,...)
{
 if(!ModInfo->EnglishInput && (Reg_EBX < 0x100) && TBL_KEY[Reg_EBX])Reg_EBX = TBL_KEY[Reg_EBX];
 return TRUE;
}
//====================================================================================
// ebp - data
//====================================================================================
BOOL _cdecl GetKeyDataSTUB(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD Reg_EDI,HVL DWORD Reg_ESI,HVL PBYTE data,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL DWORD Reg_EAX,...)
{
 static int state = 0;
 int newstate = data[DIK_SCROLL] & 0x80;

 if ( ( state == 0 ) & ( newstate != 0 ) )
  {
   if ( ModInfo->EnglishInput )
	{
	 ModInfo->MWGlobals.ShowPopUpMessage( "Русский", 0, 1 );
	 ModInfo->EnglishInput = 0;
	} 
     else
	  {
	   ModInfo->MWGlobals.ShowPopUpMessage( "English", 0, 1 );
	   ModInfo->EnglishInput = 1;
	  }
  }
 state = newstate;
 return TRUE;
}
//====================================================================================
//
//====================================================================================
int _stdcall InitMWKeyboard(void)
{
 int Result = 0;
 Result += ModInfo->CodeHooks[ModInfo->HookIndex++].SetCodeHook(&ModInfo->PatchBase[0x0024B319],GetAddress<CODEHOOK32::CODEHOOKPROC32>(EnterCharSTUB),"83C9FF33C08D");     
 Result += ModInfo->CodeHooks[ModInfo->HookIndex++].SetCodeHook(&ModInfo->PatchBase[0x00244D72],GetAddress<CODEHOOK32::CODEHOOKPROC32>(EnterCharMultiStringSTUB),"A1????????6A01508D"); 
 Result += ModInfo->CodeHooks[ModInfo->HookIndex++].SetCodeHook(&ModInfo->PatchBase[0x000066AF],GetAddress<CODEHOOK32::CODEHOOKPROC32>(GetKeyDataSTUB),"8B43288B0855");  // 0x004066AF, previous 0x4066b4 : After some function call  - Vicshann 
 if(Result)LogMsg("Errors while initializing: %u", Result);
 return Result;
}
//====================================================================================
