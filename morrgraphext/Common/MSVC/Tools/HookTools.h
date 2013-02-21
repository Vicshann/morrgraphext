// AUTHOR: Vicshann

#pragma once

#ifndef HookToolsH
#define HookToolsH

#include "ToolsBase.h"
#include "PatchTools.h"
#include "ConvertTools.h"
#include "ProcessTools.h"
//===========================================================================

#define DBGMSGOUT(msg,...)

#define HOOKREL32SIZE 5
#define OPC_NOP       0x90
#define OPC_CALL_R32  0xE8
#define OPC_JUMP_R32  0xE9

//===========================================================================

#define HVL volatile              // Mark as 'Must never be removed by optimizer'

//---------------------------------------------------------------------------
#define SETANYADDRESS(DstAddr,TakeFrom) __asm push EAX         \
                                        __asm mov EAX,TakeFrom \
                                        __asm mov DstAddr,EAX  \
                                        __asm pop EAX
//---------------------------------------------------------------------------
#define TAKEANYADDRESS(DstAddr,TakeFrom) __asm push EAX         \
                                         __asm lea EAX,TakeFrom \
                                         __asm mov DstAddr,EAX  \
                                         __asm pop EAX
//---------------------------------------------------------------------------
// Compiler always generates Stack Frame Pointers for member functions - used 'pop EBP' to restore it
// How to force the compiler do not make stack frame pointer ?
// How to force the compiler do not push ECX ?
// For __thiscall
#define JUMPEXTMEMBER(Base,Offset) __asm pop  ECX                \
                                   __asm pop  EBP                \
                                   __asm mov  EAX, Base          \
                                   __asm lea  EAX, [EAX+Offset]  \
	                               __asm jmp  EAX
//---------------------------------------------------------------------------
// Compiler always generates Stack Frame Pointers for member functions - used 'pop EBP' to restore it
// How to force the compiler do not make stack frame pointer ?
// How to force the compiler do not push ECX ?
// For __thiscall
#define JUMPVFTMEMBER(Index) __asm pop  ECX           \
                             __asm pop  EBP           \
                             __asm mov  EAX, [ECX]    \
                             __asm add  EAX, Index*4  \
                             __asm jmp  [EAX] 
//===========================================================================

#define APIWRAPPER(LibPathName,NameAPI) extern "C" _declspec(dllexport) _declspec(naked) void __cdecl NameAPI(void) \
{ \
 static void* Address; \
 if(!Address)Address = GetProcAddress(LoadLibrary(LibPathName),#NameAPI); \
 __asm mov EAX, [Address] \
 __asm jmp EAX \
}

//===========================================================================
#pragma pack(push,1)
struct HDE_STRUCT
{
 BYTE  Len;                // length of command
 BYTE  P_rep;              // rep/repnz/.. prefix: 0xF2 or 0xF3
 BYTE  P_lock;             // lock prefix 0xF0
 BYTE  P_seg;              // segment prefix: 0x2E, 0x36, 0x3E, 0x26, 0x64, 0x65
 BYTE  P_66;               // prefix 0x66
 BYTE  P_67;               // prefix 0x67
 BYTE  Opcode;             // opcode
 BYTE  Opcode2;            // second opcode, if first opcode equal 0x0F
 BYTE  Modrm;              // ModR/M byte
 BYTE  Modrm_mod;          //   - mod byte of ModR/M
 BYTE  Modrm_reg;          //   - reg byte of ModR/M
 BYTE  Modrm_rm;           //   - r/m byte of ModR/M
 BYTE  Sib;                // SIB byte
 BYTE  Sib_scale;          //   - scale (ss) byte of SIB
 BYTE  Sib_index;          //   - index byte of SIB
 BYTE  Sib_base;           //   - base byte of SIB
 BYTE  Imm8;               // immediate imm8
 WORD  Imm16;              // immediate imm16
 DWORD Imm32;              // immediate imm32
 BYTE  Disp8;              // displacement disp8
 WORD  Disp16;             // displacement disp16, if prefix 0x67 exist
 DWORD Disp32;             // displacement disp32
 BYTE  Rel8;               // relative address rel8
 WORD  Rel16;              // relative address rel16, if prefix 0x66 exist
 DWORD Rel32;              // relative address rel32
 BYTE  Immpresent;		   // It`s need, because value can be '0'
 BYTE  Dispresent;		   // It`s need, because value can be '0'
 BYTE  Relpresent;		   // It`s need, because value can be '0'
};
#pragma pack(pop)
extern "C" void  _stdcall  hde_disasm(PVOID Address, HDE_STRUCT *HdeInfo);
//=============================================================================================================

DWORD _stdcall SetProcRedirection(PVOID Address, PVOID TargetAddr, LPSTR Signature);
DWORD _stdcall SetFunctionPatch(HMODULE Module, DWORD Offset, PVOID FuncAddr, BYTE CmdCode, DWORD AddNOP, LPSTR Signature);

//=============================================================================================================
//
//                                     HOOKS IMPORT TABLE OF MODULES
//
//-------------------------------------------------------------------------------------------------------------
struct IMPORTHOOK
{
 PVOID   HookAddress;
 PVOID   OriginalAddr;
 HMODULE hCurModule;
 //---------------------------------------
 DWORD _stdcall SetHook(PVOID HookProc, LPSTR ModuleName, LPSTR ProcName, HMODULE hTargetMod, BOOL LoadIfNotExist)
  {
   HMODULE Module;
 
   DBGMSGINF("Trying to set export hook: %s::%s to %08X.",ModuleName,ProcName,(DWORD)HookProc);
   HookAddress  = NULL;
   OriginalAddr = NULL;
   Module       = GetModuleHandle(ModuleName);
   if(!Module && LoadIfNotExist)Module = LoadLibrary(ModuleName);
   if(!Module)return 1;
   
   hCurModule   = hTargetMod;
   OriginalAddr = FindProcAddress(Module, ProcName);
   if(!OriginalAddr)return 2;
   HookAddress  = HookProc;
   return UpdateModuleImports(OriginalAddr, HookProc, hCurModule);  
  }
 //---------------------------------------  
 DWORD _stdcall RemoveHook(void)
  {
   if(!HookAddress || !OriginalAddr)return 1;
   DBGMSGINF("Restoring..."); 
   return UpdateModuleImports(HookAddress, OriginalAddr, hCurModule);  
  }
};
//=============================================================================================================
//
//                                        HOOKS EXPORT TABLE OF MODULES
//
//-------------------------------------------------------------------------------------------------------------
struct EXPORTHOOK
{
 PVOID EntryAddr;
 PVOID ProcAddr;
 PVOID HookAddr;
 BOOL  OnlyLocal;
 //---------------------------------------
 DWORD _stdcall SetHook(PVOID HookProc, LPSTR ModuleName, LPSTR ProcName, BOOL LoadIfNotExist, BOOL HookOnlyLocal)
  {
   HMODULE Module;
 
   DBGMSGINF("Trying to set export hook: %s::%s to %08X.",ModuleName,ProcName,(DWORD)HookProc);
   EntryAddr = NULL;
   ProcAddr  = NULL;
   HookAddr  = NULL;
   OnlyLocal = HookOnlyLocal;
   Module    = GetModuleHandle(ModuleName);
   if(!Module && LoadIfNotExist)Module = LoadLibrary(ModuleName);
   if(!Module)return 1;
   
   EntryAddr = FindProcEntry(Module, ProcName);
   if(!EntryAddr)return 2;
   
   ProcAddr  = &((BYTE*)Module)[(((DWORD*)EntryAddr)[0])];
   if(!ProcAddr)return 3;
   
   if(WriteLocalProtectedMemory(EntryAddr, &HookProc, 4, TRUE))DBGMSGINF("Export %s::%s replaced to %08X.",ModuleName,ProcName,(DWORD)HookProc);          
     else DBGMSGERR("Replacing export %s::%s FAILED!",ModuleName,ProcName); 			
    
   DBGMSGINF("Hook is set for ET_ENTRY=%08X, PROC=%08X.",(DWORD)EntryAddr,(DWORD)ProcAddr);   
   HookAddr = HookProc; 
   return UpdateProcessImports(ProcAddr, HookAddr, HookOnlyLocal);
  } 
 //---------------------------------------  
 DWORD _stdcall RemoveHook(void)
  {
   if(!EntryAddr || !ProcAddr || !HookAddr)return 1;
   if(WriteLocalProtectedMemory(EntryAddr, &ProcAddr, 4, TRUE))DBGMSGINF("Export restored at %08X.",(DWORD)ProcAddr);          
     else DBGMSGERR("Export restoring FAILED!"); 			 
   return UpdateProcessImports(HookAddr, ProcAddr, OnlyLocal);
  }
};
//=============================================================================================================
//
//
//-------------------------------------------------------------------------------------------------------------
// !!! MUST BE SET ONLY ON BEGIN OF A FUNCTION (Checks return address to detect a caller module) !!!
// HookProc do not called, when call to function comes from Module which setted the hook
//
//               PUSHAD
//               MOV EAX, [ESP+20]
//               MOV ECX, 0   // Hooker Module Begin Addr
//               MOV EDX, 0   // Hooker Module End Addr
//               CMP EAX, ECX
//               JB SHORT GoToHookProc
//               CMP EAX, EDX
//               JA SHORT GoToHookProc
//               POPAD
//               NOP         // From here begins original code
//               NOP
//               NOP
//               NOP
//               NOP
//               NOP
//               NOP
//               NOP
//               NOP
//               NOP
//               JMP AfterReplacedCode
//GoToHookProc:  POPAD
//               JMP HookProcAddr
//
struct CODEHOOK
{
 PVOID ContAddr;         // To where return from 'HookCall' (rigth after replaced code)
 PVOID ProcAddr;         // Where must be placed 'JMP' to 'HookCall'
 PVOID HookAddr;         // Addr of a hook proc, called from 'HookCall'
 PVOID JumpAddr;         // Where really was placed 'JMP' to 'HookCall'
 DWORD BytesTaked;       // Taked from target code
 BYTE  HookCall[64];
 //---------------------------------------

 DWORD _stdcall SetHookToAddress(PVOID AddressToHook, PVOID HookProc)
  {
   DWORD      Value;
   DWORD      Offset;
   PVOID      Address;
   BYTE       CodePatch[9];
   BYTE       CodeBlock[] = {0x60,0x8B,0x44,0x24,0x20,0xB9,0x00,0x00,0x00,0x00,0xBA,0x00,0x00,0x00,0x00,0x3B,0xC1,0x72,0x14,0x3B,0xC2,0x77,0x10,0x61,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0xE9,0x5F,0xDB,0x3E,0xFF,0x61,0xE9,0x59,0xDB,0x3E,0xFF};
   HDE_STRUCT HdeInfo;
 
   Offset   = 0;
   ContAddr = NULL;
   JumpAddr = NULL;   
   HookAddr = HookProc;
   ProcAddr = AddressToHook;
   do
    {
     hde_disasm(&((BYTE*)AddressToHook)[Offset],&HdeInfo);
     if(HdeInfo.Relpresent)
      {   
       // WARNING: Possible wrong hook - jump followed     
       AddressToHook = RelAddrToAddr(&((BYTE*)AddressToHook)[Offset], HdeInfo.Len, (HdeInfo.Rel8+HdeInfo.Rel16+HdeInfo.Rel32));
	   Offset = 0;  
      }
       else Offset += HdeInfo.Len;
    }
     while(Offset < HOOKREL32SIZE);
  
   BytesTaked = Offset;
   JumpAddr   = AddressToHook;
   ContAddr   = &((BYTE*)AddressToHook)[Offset];    
   MoveMemory(&HookCall,&CodeBlock,sizeof(CodeBlock));                       // FastMoveMemory
   MoveMemory(&HookCall[24],AddressToHook,Offset);	  // Save Previous Code  // FastMoveMemory
  
   Value        = AddrToRelAddr(AddressToHook, HOOKREL32SIZE, &HookCall); 
   CodePatch[0] = OPC_JUMP_R32; 
   CodePatch[1] = ((BYTE*)&Value)[0]; 
   CodePatch[2] = ((BYTE*)&Value)[1]; 
   CodePatch[3] = ((BYTE*)&Value)[2]; 
   CodePatch[4] = ((BYTE*)&Value)[3]; 
   
   Value        = AddrToRelAddr(&HookCall[34], HOOKREL32SIZE, ContAddr); 
   HookCall[35] = ((BYTE*)&Value)[0];
   HookCall[36] = ((BYTE*)&Value)[1];          
   HookCall[37] = ((BYTE*)&Value)[2];
   HookCall[38] = ((BYTE*)&Value)[3];              
 
   Value        = AddrToRelAddr(&HookCall[40], HOOKREL32SIZE, HookProc); 
   HookCall[41] = ((BYTE*)&Value)[0];
   HookCall[42] = ((BYTE*)&Value)[1];          
   HookCall[43] = ((BYTE*)&Value)[2];
   HookCall[44] = ((BYTE*)&Value)[3];              

   Address      = GetOwnerModule(&WriteLocalProtectedMemory); 
   HookCall[6]  = ((BYTE*)&Address)[0];
   HookCall[7]  = ((BYTE*)&Address)[1];          
   HookCall[8]  = ((BYTE*)&Address)[2];
   HookCall[9]  = ((BYTE*)&Address)[3];              
  
   Value = (DWORD)FindLocalModule(&Address, &Value)+Value;    
   HookCall[11] = ((BYTE*)&Value)[0];
   HookCall[12] = ((BYTE*)&Value)[1];          
   HookCall[13] = ((BYTE*)&Value)[2];
   HookCall[14] = ((BYTE*)&Value)[3];              

   if(WriteLocalProtectedMemory(AddressToHook, &CodePatch, HOOKREL32SIZE, TRUE))DBGMSGINF("Hook setted to %08X.",(DWORD)AddressToHook);          
     else DBGMSGERR("Replacing code FAILED!"); 			

 return Offset;
}
//---------------------------------------------------------------------------
 DWORD _stdcall SetHook(PVOID HookProc, LPSTR ModuleName, LPSTR ProcName, BOOL LoadIfNotExist)
  {
   HMODULE Module;
   PVOID   Address;
   
   DBGMSGINF("Trying to set code hook: %s::%s to %08X.",ModuleName,ProcName,(DWORD)HookProc);
   ContAddr = NULL;
   ProcAddr = NULL;
   HookAddr = NULL;
   JumpAddr = NULL; 
   Module   = GetModuleHandle(ModuleName);
   if(!Module && LoadIfNotExist)Module = LoadLibrary(ModuleName);
   if(!Module)return 1;
     
   Address = FindProcAddress(Module, ProcName);
   if(!Address)return 2;
  
   if(!SetHookToAddress(Address, HookProc))return 3;
   return 0;
  }
//---------------------------------------------------------------------------
 DWORD _stdcall RemoveHook(void)
  {
   if(!JumpAddr || !ProcAddr || !HookAddr)return 1;
   if(WriteLocalProtectedMemory(JumpAddr, &HookCall[24], BytesTaked, TRUE))DBGMSGINF("Original code restored at %08X.",(DWORD)JumpAddr);          
     else DBGMSGERR("Restoring code FAILED!"); 			
   return 0;
  }
};
//===========================================================================
// !!! Calls to code from 'Hooker Module' will be also intercepted !!!
//                   !!! THREAD SAFE HOOK !!! 
//
// Stack ESP+NN - Used space
// Stack ESP-NN - Unused space
// 60 9C FF 35 00 00 40 00 E8 C8 00 00 00 5A 8B 4C 24 10 89 51 B0 84 C0 75 06 9D 61 FF 64 24 B0 9D 61
//---------------------------------------------------------------------------
struct HOOKREGS
{
 DWORD Reg_EDI;
 DWORD Reg_ESI;
 DWORD Reg_EBP;
 DWORD Reg_ESP;
 DWORD Reg_EBX;
 DWORD Reg_EDX;
 DWORD Reg_ECX;
 DWORD Reg_EAX;
 DWORD Reg_EFL; 
 DWORD Reg_EIP;
};
class CODEHOOK32
{
#pragma pack(push,1)
//---------------------------------------
//
//
 struct FHOOKCALL
  {
   BYTE  PUSHFD_9C;                        // pushfd
   BYTE  PUSHAD_60;                        // pushad                      // Stored ESP is not changed
   BYTE  CALL_E8;   DWORD HookProcRel;     // call   HookProc
   BYTE  POPAD_61;                         // popad
   BYTE  POPFD_9D;                         // popfd
   BYTE  OldCode[64];        // Original code, modified if need to make it work here   // NO BOUNDS CHECK WHEN COPYING!
  };
//---------------------------------------
// Allows to intercept any fragment of code. Original code can be executed or skipped
//
 struct CHOOKCALL
  {
   BYTE  PUSHAD_60;                        // pushad                      // Stored ESP is not changed
   BYTE  PUSHFD_9C;                        // pushfd
   WORD  PUSH_FF35; DWORD RetAddr;         // push   [0x00000000]         // RETURN ADDRESS [EIP](Allowed to modify as argument)
   BYTE  CALL_E8;   DWORD HookProcRel;     // call   HookProc
   BYTE  POP_EDX_5A;                       // pop  EDX                    // Get return address (possibly modified)
   DWORD MOV_ECX_8B4C2410;                 // mov  ECX, [ESP+0x0010]      // Get stored ESP (possibly modified)
   BYTE  MOV_StackRet_89; WORD _EDX_51B0;  // mov  [ECX-0x0050], EDX      // Write ret address to unused stack slot (For possibly modified ESP)
   WORD  TEST_AL_AL_84C0;                  // test AL, AL
   WORD  JNZ_7506;                         // jnz OldCodeStart
   BYTE  POPFD_9D;                         // popfd
   BYTE  POPAD_61;                         // popad
   DWORD JMP_FF6424B0;                     // jmp [ESP-0x0050]            // Go to RETURN ADDRESS (In Unused stack slot)   [FF 64 24 B0]
   BYTE  POPFD__9D;                        // OldCodeStart: popfd
   BYTE  POPAD__61;                        //               popad
   BYTE  OldCode[64];        // Original code, modified if need to make it work here   // NO BOUNDS CHECK WHEN COPYING!
  };
#pragma pack(pop)
 //---------------------------------------
public:
 typedef void (_cdecl *FUNCHOOKPROC32)(HVL HOOKREGS Registers,...);   // Check registers order !

 typedef BOOL (_cdecl *CODEHOOKPROC32)(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD Reg_EDI,HVL DWORD Reg_ESI,HVL DWORD Reg_EBP,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL DWORD Reg_EAX,...);    // Return 'TRUE' to execute original code

 PVOID JumpAddr;          // Where really was placed 'JMP' to 'CHookStub'
 PVOID ContAddr;          // To where return from 'CHookStub' (rigth after replaced code)
 DWORD BytesTaken;        // Taken from target code
 PVOID HookAddr;          // Addr of a hook proc, called from 'CHookStub'

 //--------------------------------------- 
 DWORD _stdcall RemoveHook(void)
  {
   if(!JumpAddr || !TargAddr || !HookAddr)return 1;
   if(WriteLocalProtectedMemory(JumpAddr, &OriginalCode, BytesTaken, TRUE))DBGMSGINF("Original code restored at %08X.",(DWORD)JumpAddr);
     else DBGMSGERR("Restoring code FAILED!");
   return 0;
  }
 //---------------------------------------
 DWORD _stdcall SetCodeHook(PVOID AddressToHook, CODEHOOKPROC32 HookProc, LPSTR Signature=NULL, UINT StealBytes=0) // TODO: StealExtra - do recalculation of jump/call offsets
  {
   DWORD Result; 
   DWORD OCBytes;
   BYTE  CodePatch[32];

   if(IsBadReadPtr(AddressToHook, HOOKREL32SIZE))return 9;
   CHookStub.PUSHAD_60        = 0x60;
   CHookStub.PUSHFD_9C        = 0x9C;
   CHookStub.PUSH_FF35        = 0x35FF;
   CHookStub.CALL_E8          = 0xE8;
   CHookStub.POP_EDX_5A       = 0x5A;
   CHookStub.MOV_ECX_8B4C2410 = 0x10244C8B;
   CHookStub.MOV_StackRet_89  = 0x89;
   CHookStub._EDX_51B0        = 0xB051;
   CHookStub.TEST_AL_AL_84C0  = 0xC084;
   CHookStub.JNZ_7506         = 0x0675;
   CHookStub.POPFD_9D         = 0x9D;
   CHookStub.POPAD_61         = 0x61;
   CHookStub.JMP_FF6424B0     = 0xB02464FF;
   CHookStub.POPFD__9D        = 0x9D;
   CHookStub.POPAD__61        = 0x61;
   
   if(StealBytes < HOOKREL32SIZE)StealBytes = HOOKREL32SIZE;
   OCBytes = PrepareCodeHook(AddressToHook, StealBytes, (PBYTE)&CHookStub.OldCode);
   FillMemory(&CodePatch, BytesTaken, 0x90);    // Watch for the buffer overflow !   // FastFillMemory
   HookAddr                   = HookProc;
   CodePatch[0]               = OPC_JUMP_R32;
   CHookStub.RetAddr          = (DWORD)&ContAddr;
   CHookStub.HookProcRel      = AddrToRelAddr(&CHookStub.CALL_E8, HOOKREL32SIZE, HookProc);  
   ((PDWORD)&CodePatch[1])[0] = AddrToRelAddr(JumpAddr, HOOKREL32SIZE, &CHookStub);
   if(!StackMod)((PDWORD)&CHookStub.OldCode[OCBytes])[0] = CHookStub.JMP_FF6424B0;          // Write returning jump
     else
      {
       CHookStub.OldCode[OCBytes] = OPC_JUMP_R32;
       ((PDWORD)&CHookStub.OldCode[OCBytes+1])[0] = AddrToRelAddr(&CHookStub.OldCode[OCBytes], HOOKREL32SIZE, ContAddr);
      }

   /*
   if(WriteLocalProtectedMemory(AddressToHook, &CodePatch, HOOKJMPSIZE, TRUE))DBGMSGINF("Hook placed at %08X.",(DWORD)AddressToHook);
     else DBGMSGERR("Replacing code FAILED!");
      */
   if(((PBYTE)JumpAddr)[0]==CodePatch[0])Signature=NULL;     // Don`t check signature if address are possibly already patched
   Result = MemoryPatchLocal(JumpAddr, &CodePatch, BytesTaken, Signature);     // !!!!!!!!!!!!! BytesTaked;   
   if(Result == 0)Busy = TRUE;
   return Result;
  }
 //---------------------------------------
 DWORD _stdcall SetFuncHook(PVOID AddressToHook, FUNCHOOKPROC32 HookProc, LPSTR Signature=NULL)
  {
   DWORD Result; 
   DWORD OCBytes;
   BYTE  CodePatch[32];

   if(IsBadReadPtr(AddressToHook, HOOKREL32SIZE))return 9;
   FHookStub.PUSHFD_9C = 0x9C;
   FHookStub.PUSHAD_60 = 0x60;
   FHookStub.CALL_E8   = 0xE8;
   FHookStub.POPAD_61  = 0x61;
   FHookStub.POPFD_9D  = 0x9D;
   
   OCBytes = PrepareCodeHook(AddressToHook, HOOKREL32SIZE, (PBYTE)&FHookStub.OldCode);
   FillMemory(&CodePatch, BytesTaken, 0x90);    // Watch for the buffer overflow !   // FastFillMemory
   HookAddr     = HookProc;
   CodePatch[0] = OPC_JUMP_R32;
   ((PDWORD)&CodePatch[1])[0] = AddrToRelAddr(JumpAddr, HOOKREL32SIZE, &FHookStub);
   FHookStub.HookProcRel      = AddrToRelAddr(&FHookStub.CALL_E8, HOOKREL32SIZE, HookProc);
   FHookStub.OldCode[OCBytes] = OPC_JUMP_R32;
   ((PDWORD)&FHookStub.OldCode[OCBytes+1])[0] = AddrToRelAddr(&FHookStub.OldCode[OCBytes], HOOKREL32SIZE, ContAddr);
      
   /*
   if(WriteLocalProtectedMemory(AddressToHook, &CodePatch, HOOKJMPSIZE, TRUE))DBGMSGINF("Hook placed at %08X.",(DWORD)AddressToHook);
     else DBGMSGERR("Replacing code FAILED!");
      */
   if(((PBYTE)JumpAddr)[0]==CodePatch[0])Signature=NULL;     // Don`t check signature if address are possibly already patched    
   Result = MemoryPatchLocal(JumpAddr, &CodePatch, BytesTaken, Signature);     // !!!!!!!!!!!!! BytesTaked;   
   if(Result == 0)Busy = TRUE;
   return Result;
  }
 
 //---------------------------------------
 // TODO: Rework this function !
 DWORD _stdcall SetCmdValue(PVOID CmdAddress, PVOID Value, LPSTR Signature=NULL)
  {
   DWORD Result = 3;
   HDE_STRUCT HdeInfo;
   BYTE CodePatch[32];

   if(IsBadReadPtr(CmdAddress, HOOKREL32SIZE))return 9;
   hde_disasm(CmdAddress, &HdeInfo);
   MoveMemory(&CodePatch, CmdAddress, HdeInfo.Len);      // memmove
   MoveMemory(&OriginalCode, CmdAddress, HdeInfo.Len);   // memmove
   if(HdeInfo.Relpresent)
    {  
     ((PDWORD)&CodePatch[1])[0] = AddrToRelAddr(CmdAddress, HdeInfo.Len, Value);
     Result = MemoryPatchLocal(CmdAddress, &CodePatch, HdeInfo.Len, Signature);
    }
   if(HdeInfo.Immpresent)
    {
     ((PDWORD)&CodePatch[1])[0] = reinterpret_cast<DWORD>(Value);
     Result = MemoryPatchLocal(CmdAddress, &CodePatch, HdeInfo.Len, Signature);
    }  
   if(Result == 0)Busy = TRUE;
   JumpAddr   = CmdAddress;
   BytesTaken = HdeInfo.Len;
   return Result;
  } 
 //---------------------------------------
 DWORD _stdcall SetCallHook(PVOID CmdAddress, PVOID Value, LPSTR Signature=NULL)
  {
   DWORD Result = 3;
   HDE_STRUCT HdeInfo;
   BYTE CodePatch[32];

   if(IsBadReadPtr(CmdAddress, HOOKREL32SIZE))return 9;
   hde_disasm(CmdAddress, &HdeInfo);
   FillMemory(&CodePatch,HdeInfo.Len,OPC_NOP);    // FastFillMemory
   MoveMemory(&OriginalCode, CmdAddress, HdeInfo.Len);    // memmove
  
   CodePatch[0] = OPC_JUMP_R32;
   ((PDWORD)&CodePatch[1])[0] = AddrToRelAddr(CmdAddress, HOOKREL32SIZE, Value);
   Result = MemoryPatchLocal(CmdAddress, &CodePatch, HdeInfo.Len, Signature);
    
   if(Result == 0)Busy = TRUE;
   JumpAddr   = CmdAddress;
   BytesTaken = HdeInfo.Len;
   return Result;
  } 
 //---------------------------------------
  
  
  
private: 
 BOOL      Busy;
 BOOL      StackMod;          // Replaced code modifies the stack - Changing EIP will no effect when executing replaced code
 PVOID     TargAddr;          // Where must be placed 'JMP' to 'CHookStub'
 CHOOKCALL CHookStub;         // Code of a hook stub
 FHOOKCALL FHookStub;         // Code of a hook stub
 BYTE      OriginalCode[64];  // Original, don`t modified code
 //---------------------------------------

 int _stdcall PrepareCodeHook(PVOID AddressToHook, DWORD HookCmdSize, PBYTE ModOldCode)
  {
   DWORD      OCBytes;
   HDE_STRUCT HdeInfo;

   OCBytes    = 0;
   ContAddr   = NULL;
   JumpAddr   = NULL;
   TargAddr   = AddressToHook;
   BytesTaken = 0;
  
   do
    {
     hde_disasm(&((BYTE*)AddressToHook)[BytesTaken],&HdeInfo);
     MoveMemory(&ModOldCode[OCBytes], &((BYTE*)AddressToHook)[BytesTaken], HdeInfo.Len);	  // Save Previous Code  // memmove
     // Call Rel32
     if((HdeInfo.Opcode == OPC_CALL_R32))
      {
       StackMod = true; //For any dangerous stack modification
       ((PDWORD)&ModOldCode[OCBytes+1])[0] = AddrToRelAddr(&ModOldCode[OCBytes], 5, RelAddrToAddr(&((BYTE*)AddressToHook)[BytesTaken], HdeInfo.Len, (HdeInfo.Rel8+HdeInfo.Rel16+HdeInfo.Rel32)));
      }
     // Conditional jumps 
     if((HdeInfo.Opcode == 0x0F)&&((HdeInfo.Opcode2 >= 0x80)&&(HdeInfo.Opcode2 <= 0x8F))){((PDWORD)&ModOldCode[OCBytes+2])[0] = AddrToRelAddr(&ModOldCode[OCBytes], 6, &((BYTE*)AddressToHook)[BytesTaken+HdeInfo.Len+HdeInfo.Rel32]);}
     if((HdeInfo.Opcode >= 0x70)&&(HdeInfo.Opcode <= 0x7F))
      {
       ModOldCode[OCBytes]   = 0x0F;   // Prefix
       ModOldCode[OCBytes+1] = (HdeInfo.Opcode+0x10);
       ((PDWORD)&ModOldCode[OCBytes+2])[0] = AddrToRelAddr(&ModOldCode[OCBytes], 6, RelAddrToAddr(&((BYTE*)AddressToHook)[BytesTaken], HdeInfo.Len, (HdeInfo.Rel8+HdeInfo.Rel16+HdeInfo.Rel32)));
       OCBytes += 4;  // For long jump size
      }
     /*if(HdeInfo.Relpresent)
      {
       // WARNING: Possible wrong hook - jump followed
       AddressToHook = RelAddrToAddr(&((BYTE*)AddressToHook)[BytesTaked], HdeInfo.Len, (HdeInfo.Rel8+HdeInfo.Rel16+HdeInfo.Rel32));
	   BytesTaked = 0;
      }
       else*/ BytesTaken += HdeInfo.Len;
       OCBytes += HdeInfo.Len;
    }
     while(BytesTaken < HookCmdSize);

   JumpAddr = AddressToHook;
   ContAddr = &((BYTE*)AddressToHook)[BytesTaken];
   MoveMemory(&OriginalCode, AddressToHook, BytesTaken);	      // Save Previous Code  // memmove
   return OCBytes;
  }
 //---------------------------------------
};     
//===========================================================================




//===========================================================================
#endif
