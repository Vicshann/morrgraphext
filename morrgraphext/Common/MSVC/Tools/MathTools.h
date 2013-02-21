// AUTHOR: Vicshann

#pragma once

#ifndef MathToolsH
#define MathToolsH

#include "FileTools.h"
//===========================================================================
	   
float   _stdcall FPUSqrt32(float Value);
long    _stdcall FPURound32(float Value);
long    _stdcall FPUDiv32(float ValueA, float ValueB);
float   _stdcall FPURem32(float ValueA, float ValueB);
float   _stdcall FPUSinRad32(float Angle);
float   _stdcall FPUSinDeg32(float Angle);
float   _stdcall FPUCosRad32(float Angle);
float   _stdcall FPUCosDeg32(float Angle);
__int64 _stdcall FPUSinCosRad32(float Angle);
__int64 _stdcall FPUSinCosDeg32(float Angle);
 
//===========================================================================	   
#endif
