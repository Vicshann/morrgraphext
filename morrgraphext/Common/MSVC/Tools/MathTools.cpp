// AUTHOR: Vicshann

#include "MathTools.h"

//====================================================================================
__declspec(naked)long _stdcall FPURound32(float Value)
{  // Used Predefined Rounding Mode
 __asm FLD   DWORD PTR [ESP+4]
 __asm FISTP DWORD PTR [ESP+4]
 __asm MOV   EAX,[ESP+4]
 __asm RET 4
}
//---------------------------------------------------------------------------
__declspec(naked)float _stdcall FPUSqrt32(float Value)
{  // Float results returned in FPU stack
 __asm FLD   DWORD PTR [ESP+4]
 __asm FSQRT
 __asm RET 4
}
//---------------------------------------------------------------------------
__declspec(naked)float _stdcall FPURem32(float ValueA, float ValueB) // A%B
{  // Float results returned in FPU stack
 __asm FLD   DWORD PTR [ESP+8]    // ValueB
 __asm FLD   DWORD PTR [ESP+4]    // ValueA 
 __asm FPREM
 __asm FSTP  DWORD PTR [ESP+4]    // Round result
 __asm MOV   EAX,[ESP+4]
 __asm FSTP  DWORD PTR [ESP+4]    // Round result 
 __asm MOV   [ESP+4],EAX
 __asm FLD   DWORD PTR [ESP+4]
 __asm RET 8
}
//---------------------------------------------------------------------------
__declspec(naked)long _stdcall FPUDiv32(float ValueA, float ValueB) // A/B
{  // Float results returned in FPU stack
 __asm FLD   DWORD PTR [ESP+4]    // ValueA
 __asm FDIV  DWORD PTR [ESP+8]
 __asm FISTP DWORD PTR [ESP+4]    // Round result
 __asm MOV   EAX,[ESP+4]
 __asm RET 8
}
//---------------------------------------------------------------------------
__declspec(naked)float _stdcall FPUSinRad32(float Angle)
{  // Float results returned in FPU stack
 __asm FLD   DWORD PTR [ESP+4]
 __asm FSIN
 __asm RET 4
}
//---------------------------------------------------------------------------
__declspec(naked)float _stdcall FPUSinDeg32(float Angle)
{  // Float results returned in FPU stack
 __asm FLD   DWORD PTR [ESP+4]
 __asm MOV   DWORD PTR [ESP+4],180
 __asm FIDIV [ESP+4]
 __asm FLDPI
 __asm FMULP st(1),st
 __asm FSIN
 __asm RET 4
}
//---------------------------------------------------------------------------
__declspec(naked)float _stdcall FPUCosRad32(float Angle)
{  // Float results returned in FPU stack
 __asm FLD   DWORD PTR [ESP+4]
 __asm FCOS
 __asm RET 4
}
//---------------------------------------------------------------------------
__declspec(naked)float _stdcall FPUCosDeg32(float Angle)
{  // Float results returned in FPU stack
 __asm FLD   DWORD PTR [ESP+4]
 __asm MOV   DWORD PTR [ESP+4],180
 __asm FIDIV [ESP+4]
 __asm FLDPI
 __asm FMULP st(1),st
 __asm FCOS
 __asm RET 4
}
//---------------------------------------------------------------------------
__declspec(naked)__int64 _stdcall FPUSinCosRad32(float Angle)
{   // [0] - sine; [1] - cosine
 __asm FLD   DWORD PTR [ESP+4]
 __asm FSINCOS
 __asm FSTP  DWORD PTR [ESP+4]   // COS
 __asm MOV   EDX,[ESP+4]
 __asm FSTP  DWORD PTR [ESP+4]   // SIN
 __asm MOV   EAX,[ESP+4]
 __asm RET 4
}
//---------------------------------------------------------------------------
__declspec(naked)__int64 _stdcall FPUSinCosDeg32(float Angle)
{   // [0] - sine; [1] - cosine
 __asm FLD   DWORD PTR [ESP+4]
 __asm MOV   DWORD PTR [ESP+4],180
 __asm FIDIV [ESP+4]
 __asm FLDPI
 __asm FMULP st(1),st
 __asm FSINCOS
 __asm FSTP  DWORD PTR [ESP+4]   // COS
 __asm MOV   EDX,[ESP+4]
 __asm FSTP  DWORD PTR [ESP+4]   // SIN
 __asm MOV   EAX,[ESP+4]
 __asm RET 4
}
//---------------------------------------------------------------------------





//====================================================================================

