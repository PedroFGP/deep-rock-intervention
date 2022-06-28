#pragma once
#include "pch.h"
#include <Psapi.h>

#define CONCAT_IMPL(x, y) x##y
#define MACRO_CONCAT(x, y) CONCAT_IMPL(x, y)
#define PAD(SIZE) BYTE MACRO_CONCAT(_pad, __COUNTER__)[SIZE];
#define D2R(DEGRESS) DEGRESS*(3.141592f/180.0f)

template< typename Function > Function callVirtualFunction(PVOID p_Base, DWORD p_Index)
{
	PDWORD* v_VTablePointer = (PDWORD*)p_Base;
	PDWORD v_VTableFunctionBase = *v_VTablePointer;
	DWORD v_Address = v_VTableFunctionBase[p_Index];
	return (Function)(v_Address);
}

bool Compare(char* data, char* sig, uint64_t size);
char* FindSignature(char* start, char* end, char* sig, uint64_t size);
void* FindPointer(HMODULE mod, char* sig, uint64_t size, int addition);
bool PatchMem(void* address, void* bytes, uint64_t size);
std::pair<float*, float*> minmaxfloat(float* arr, size_t nelem);