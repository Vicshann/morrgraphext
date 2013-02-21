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
// DialogEngine.cpp
// -------------------------------------------------------------------
// $Id: DialogEngine.cpp,v 1.1 2005-09-06 19:02:30 AngelD Exp $ 
// Rewritten: 2013:10:02 Vicshann
//////////////////////////////////////////////////////////////////////

#include "ExtensionMod.h"


using namespace MODULENAME;

extern MODULENAME::PSModData  ModInfo;
//====================================================================================

const unsigned char TranslateTable[] = "јЅ¬√ƒ≈∆«» ЋћЌќѕ–—“”‘’÷„ЎўЏ";   //Alphavit           // Zero terminated!
const char ALPHAVIT[] = "јЅ¬√ƒ≈∆«»… ЋћЌќѕ–—“”‘’÷„ЎўЏџ№Ёёяабвгдежзийклмнопрстуфхцчшщъыьэю€"; // Zero terminated!
const char sPage[] = "\n<P>\n";                                                             // Zero terminated!

//====================================================================================
//                                   STUBS
//====================================================================================
//====================================================================================
// ebx - string
//
//====================================================================================
BOOL _cdecl DictStrSTUB(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD Reg_EDI,HVL DWORD Reg_ESI,HVL DWORD Reg_EBP,HVL DWORD Reg_ESP,HVL LPSTR dstr,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL DWORD Reg_EAX,...)
{
 if(dstr && dstr[0] && (dstr[0] < 0xC0) && (lstrlen(dstr) <= 990))
  {
   //LogMsg("Searching for: %s", dstr);
   if(CMiniStr* str = ModInfo->cells.FindKey(dstr,ModInfo->StrLstCase))
    {
     if(ModInfo->LogTxtSubst)LogMsg("Replacing string '%s' with '%s' from CELLS list", dstr, str->c_str());
     dstr = str->c_str();   // It is safe to pass here a pointer to original string
    }
     else   
      {
       // Add here a new phrases into dictionary
      }
  }
 return TRUE;
}
//====================================================================================
// esi - topic
// edx - dialog str
//
// —троки в ESM и ESP должны уже быть переведены. 
// ѕолучаем русскую строку на входе, ищем в списке соответствующую ей английскую и подмен€ем
//
//====================================================================================
BOOL _cdecl MarkTopicSTUB(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD Reg_EDI,HVL LPSTR topic,HVL DWORD Reg_EBP,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL DWORD Reg_EAX,...)
{
 if(topic && topic[0])
  {
   //LogMsg("Searching for: %s", topic);
   if(CMiniStr* str = ModInfo->marktopic.FindKey(topic,ModInfo->StrLstCase))
    {
     if(ModInfo->LogTxtSubst)LogMsg("Replacing string '%s' with '%s' from MARKTOPICS list", topic, str->c_str());
	 topic = str->c_str();   // It is safe to pass here a pointer to original string
    }
  }
 return TRUE;
}
//====================================================================================
// подмена линка
//
// eax -text
// eax+edx -buff for topic
// return:
//  esi длина топика в тексте без 0x0a
//  edi длина топика в buff
//====================================================================================
BOOL _cdecl CopyTopicFromDialogSTUB(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD Reg_EDI,HVL DWORD Reg_ESI,HVL DWORD Reg_EBP,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL LPSTR Reg_EAX,...)
{
 char c;

 int  esi_len = 0;
 int  edi_len = 0;
 char *buff   = Reg_EAX + Reg_EDX;  // EDX+EAX
 LPSTR text   = Reg_EAX;            // EAX
 while ( ( c = text[edi_len] ) != '#' )
  {
   if( c >= 'A' && c <= 'Z' )c += 0x20;
   if( c == 0x0A )buff[edi_len] = 0x20;
     else {buff[edi_len] = c; esi_len++;}
   edi_len++;
  }
 buff[edi_len] = 0;
 //LogMsg("Searching for: %s", buff);
 if(CMiniStr* str = ModInfo->top_data.FindKey(buff,ModInfo->StrLstCase))
  {
   if(ModInfo->LogTxtSubst)LogMsg("Replacing string '%s' with '%s' from TOPICS list", buff, str->c_str());
   lstrcpy(buff, str->c_str());
   edi_len = str->Length();
  }

 Reg_ESI += esi_len;
 Reg_EDI += edi_len; 
 return FALSE;
}
//====================================================================================
// ebp - journal; edi curroff
// !!! Was set at '00432561 mov edx, [esp+10h]' with CALL redirection without modification of stack offset !!! // vicshann
//====================================================================================
BOOL _cdecl QuestsParseSTUB(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD CurOff,HVL DWORD Reg_ESI,HVL LPSTR Journal,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL LPSTR Reg_EAX,...)
{
 if(CurOff && (Journal[CurOff-1] == 0x0A))CurOff--;
 return TRUE;
}
//====================================================================================
//                                  HOOKED
//====================================================================================
//====================================================================================
//
//====================================================================================
int _cdecl travel_printf( char *str, char *fspec, char *s1, char *s2, int nn, char *s3 )  // Hook: 006194B1  call sprintf  // 'wsprintf' and 'sprintf' are compatible
{
 if ( s1 == NULL )return wsprintf( str, fspec, s1, s2, nn, s3 );
 if ( s1[0] == 0 )return wsprintf( str, fspec, s1, s2, nn, s3 );
 if ( strlen( s1 ) > 990 )return wsprintf( str, fspec, s1, s2, nn, s3 );
 if ( s1[0] >= 0xc0 )return wsprintf( str, fspec, s1, s2, nn, s3 );
 if(CMiniStr* vstr = ModInfo->cells.FindKey(s1,ModInfo->StrLstCase))return wsprintf( str, fspec, vstr->c_str(), s2, nn, s3 );
   else return wsprintf( str, fspec, s1, s2, nn, s3 );	
}
//====================================================================================
//
//====================================================================================
int _cdecl alphavit_cmp(char *bukva, char *sTopic, int i)    // _strnicmp hook
{
 if(i != 1)return _strnicmp( bukva, sTopic, i );

 BYTE cTop, cBukva;

 cTop   = ( BYTE ) * sTopic;
 cBukva = ( BYTE ) * bukva;
 if((cTop  <  0xC0) && (cBukva == 'A'))return 0;
 if((cTop  >= 0xE0) && (cTop <= 0xFF))cTop -= 0x20;
 if(cTop   == TranslateTable[cBukva - 'A'])return 0;
 if((cTop  == 201)  && (cBukva == 'I'))return 0;
 if(cBukva == 'Z')
  {
   if( cTop == 219 )return 0;	//џ
   if( cTop == 220 )return 0;	//№
   if( cTop == 221 )return 0;	//Ё
   if( cTop == 222 )return 0;	//ё
   if( cTop == 223 )return 0;	//я
  }
 return ( cTop > TranslateTable[cBukva - 'A'] ) ? (1) : (-1);
}
//====================================================================================
//
//====================================================================================
int _cdecl alphavit_prn( char *b, char *sFormat, int i )  // sprintf hook
{
 if(i == 'I')return wsprintf( b, "(»-…)" );
 if(i == 'Z')return wsprintf( b, "(Ё-я)" );
 return wsprintf( b, "( %c )", TranslateTable[i - 'A'] );
}
//====================================================================================
//                                  DIALOGUE ENGINE
//====================================================================================
//====================================================================================
//
//====================================================================================
int _stdcall LoadStringLists(void)   
{
 int  Err = 0;
 char KeyName[20], fname[256];
 BYTE IniPath[MAX_PATH];
 BYTE FilePath[MAX_PATH];

 wsprintf((LPSTR)&IniPath, "%sMorrowind.ini", &ModInfo->GameCurDir);
 for(int i = 0;;i++)
  {
   wsprintf(KeyName, "GameFile%d", i);
   if(0 == GetPrivateProfileString("Game Files", KeyName, "", fname, sizeof(fname), (LPSTR)&IniPath))break;
   LogMsg("Found a Game File: %s", &fname);

   wsprintf((LPSTR)&FilePath,"%sData Files\\%s",&ModInfo->GameCurDir,&fname);
   LPSTR Ext = GetFileExt((LPSTR)&FilePath);
   if(Ext == (LPSTR)&FilePath)continue;
   if(lstrcmpi(Ext,"esp") == 0){if(!ModInfo->LoadTopicsForPlugins)continue;}
     else if(lstrcmpi(Ext,"esm") != 0)continue;  

   lstrcpy(Ext,"top");     // загрузка *.top файлов дл€ файлов из списка GameFile       // Format: Link phrase \t Alternate phrase
   if(!ModInfo->top_data.LoadFromFile((LPSTR)&FilePath, '\t', (SLOpt)(soNoDups | ModInfo->StrLstCase))){Err++;LogMsg("Cannot load topdata file '%s'", (LPSTR)&FilePath);}
   lstrcpy(Ext,"mrk");     // Ќазвани€ тем в журнале. ”же переведены во всех ESM        // Format: Rus \t Eng
   if(!ModInfo->marktopic.LoadFromFile((LPSTR)&FilePath, '\t', (SLOpt)(soNoDups | ModInfo->StrLstCase))){Err++;LogMsg("Cannot load marktopic file '%s'", (LPSTR)&FilePath);}
   lstrcpy(Ext,"cel");     // Ќазвани€ всех уникальных €чеек не переведены в ESM и ESP  // Format: Eng \t Rus
   if(!ModInfo->cells.LoadFromFile((LPSTR)&FilePath, '\t', (SLOpt)(soNoDups | ModInfo->StrLstCase))){Err++;LogMsg("Cannot load celldata file '%s'", (LPSTR)&FilePath);}
  }
 for(CStringMap::SEntry* cent = ModInfo->top_data.First();cent;cent  = ModInfo->top_data.Next(cent)){ReplaceChar(cent->key.c_str(),'*',0x7F);ReplaceChar(cent->val.c_str(),'*',0x7F);}   // Better to do this in the files themselves 
 //for(CStringMap::SEntry* cent = ModInfo->marktopic.First();cent;cent = ModInfo->marktopic.Next(cent)){ReplaceChar(cent->key.c_str(),'*',0x7F);}   

 //ModInfo->cells.SaveToFile("CellsList.txt", '\t');
 //ModInfo->top_data.SaveToFile("TopicsList.txt", '\t');
 //ModInfo->marktopic.SaveToFile("MarkTopicsList.txt", '\t');
 return Err;
}
//====================================================================================
//
//====================================================================================
void _stdcall UnloadDialogEngine(void)
{
 ModInfo->cells.Clear();    
 ModInfo->top_data.Clear();  
 ModInfo->marktopic.Clear(); 
}
//====================================================================================
//
//====================================================================================
int _stdcall InitDialogEngine(void)
{
 int Result = 0;
 if(LoadStringLists())Result += 1000;

 Result += SetFunctionPatch(ModInfo->ExeModuleBase,0x002194B1,&travel_printf,OPC_CALL_R32,1,"FF15????????8B4C243C");      // sprintf
 Result += SetFunctionPatch(ModInfo->ExeModuleBase,0x001D783C,&alphavit_prn,OPC_CALL_R32,1,"FF15????????83C40C55");       // sprintf
 Result += SetFunctionPatch(ModInfo->ExeModuleBase,0x001D798E,&alphavit_prn,OPC_CALL_R32,1,"FF15????????83C40C55");       // sprintf
 Result += SetFunctionPatch(ModInfo->ExeModuleBase,0x001D75A8,&alphavit_cmp,OPC_CALL_R32,1,"FF15????????83C40C85C0");     // _strnicmp
 Result += ModInfo->CodeHooks[ModInfo->HookIndex++].SetCodeHook(&ModInfo->PatchBase[0x0017C6DE],GetAddress<CODEHOOK32::CODEHOOKPROC32>(DictStrSTUB),"83C9FF33C08BFB");  
 Result += ModInfo->CodeHooks[ModInfo->HookIndex++].SetCodeHook(&ModInfo->PatchBase[0x000B1FF5],GetAddress<CODEHOOK32::CODEHOOKPROC32>(MarkTopicSTUB),"83C9FF8BFEF2AEF7D1");   
 Result += ModInfo->CodeHooks[ModInfo->HookIndex++].SetCodeHook(&ModInfo->PatchBase[0x00032561],GetAddress<CODEHOOK32::CODEHOOKPROC32>(QuestsParseSTUB),"8B54241043894C2434"); 
 Result += ModInfo->CodeHooks[ModInfo->HookIndex++].SetCodeHook(&ModInfo->PatchBase[0x0000B872],GetAddress<CODEHOOK32::CODEHOOKPROC32>(CopyTopicFromDialogSTUB),"8A0880F90A7406880C02",27);  

 Result += MemoryPatchLocal(&ModInfo->PatchBase[0x00375AFC], (PVOID)&ALPHAVIT, sizeof(ALPHAVIT)-1, "414141418E8F92804590454549494949");  
 Result += MemoryPatchLocal(&ModInfo->PatchBase[0x003934B8], (PVOID)&ALPHAVIT, sizeof(ALPHAVIT)-1, "414141414141C6C74545454549494949");  
 Result += MemoryPatchLocal(&ModInfo->PatchBase[0x00376CD0], (PVOID)&sPage,    sizeof(sPage), "3C503E0A00000000");      // <P>\n
 
 Result += MemoryPatchMaskLocal(&ModInfo->PatchBase[0x000B2C50], "9090????EB",   "74043C237501");   // remove @# from journal
 Result += MemoryPatchMaskLocal(&ModInfo->PatchBase[0x000B2C71], "9090??????EB", "740580F9237503"); // remove @# from journal
 if(Result)LogMsg("Errors while initializing: %u", Result);	
 return Result;                                            
}                                                        
//====================================================================================