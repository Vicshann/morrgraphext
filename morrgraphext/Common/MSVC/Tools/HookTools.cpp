// AUTHOR: Vicshann

#include "HookTools.h"

//===========================================================================
// Sets jump at beginning of procedure to TargetAddr
//
DWORD _stdcall SetProcRedirection(PVOID Address, PVOID TargetAddr, LPSTR Signature)
{
 BYTE CodePatch[8];

 CodePatch[0] = OPC_JUMP_R32;
 ((PVOID*)&CodePatch[1])[0] = (PVOID)AddrToRelAddr(Address, HOOKREL32SIZE, TargetAddr);
 return MemoryPatchLocal(Address, &CodePatch, HOOKREL32SIZE, Signature);
}
//---------------------------------------------------------------------------
DWORD _stdcall SetFunctionPatch(HMODULE Module, DWORD Offset, PVOID FuncAddr, BYTE CmdCode, DWORD AddNOP, LPSTR Signature)
{
 PVOID PatchAddr;
 DWORD PatchLen;
 DWORD Address;
 DWORD Index = 0;
 BYTE  PatchData[512];

 FillMemory(&PatchData,sizeof(PatchData),0x90); // Fill with NOPs    //FastFillMemory
 PatchAddr = &((BYTE*)Module)[Offset];
 switch(CmdCode)
  {
   case 0xE8:    // Call
   case 0xE9:    // Jmp
     Address      = AddrToRelAddr(PatchAddr, 5, FuncAddr);     
     PatchLen     = HOOKREL32SIZE;
     PatchData[Index] = CmdCode;
     Index++;
     break;
   case 0xB8:    // mov EAX,NNNN
   case 0x68:    // push DWORD
   case 0xA1:    // mov EAX,[NNNN]  
   case 0xA3:    // mov [NNNN],EAX
     Address      = (DWORD)FuncAddr;     
     PatchLen     = 5;
     PatchData[Index] = CmdCode;
     Index++;
     break;	 
   case 0x6A:    // push BYTE
     Address      = (DWORD)FuncAddr;     
     PatchLen     = 2;
     PatchData[Index] = CmdCode;
     Index++;
     break;	
   case 0x90:    // nop
     Address      = 0x90909090;     
	 PatchLen     = 1;
     PatchData[Index] = CmdCode;
     Index++;
     break;	
   case 0xEB:    // jmp rel BYTE
     PatchLen     = 1;
     PatchData[Index] = CmdCode;
	 //if(FuncAddr){}  // convert addr to rel
     Index++;
     break;	
  }
         
 ((PDWORD)&PatchData[Index])[0] = Address;  
 return MemoryPatchLocal(PatchAddr, &PatchData, (PatchLen+AddNOP), Signature);
}
//===========================================================================