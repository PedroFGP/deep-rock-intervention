// dllmain.cpp : Define o ponto de entrada para o aplicativo DLL.
#include "pch.h"

void* Dll = nullptr;
bool err = false;

void ClearingThread()
{
	while (1)
	{
		if (GetAsyncKeyState(VK_END) & 1) { FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(Dll), 0); }
		if (GetAsyncKeyState(VK_F4) & 1) { aimbotActive = !aimbotActive; }
		if (GetAsyncKeyState(VK_F5) & 1) { drawBonesActive = !drawBonesActive; }
		if (GetAsyncKeyState(VK_F6) & 1) { draw2DBoundingBoxActive = !draw2DBoundingBoxActive; }
		if (GetAsyncKeyState(VK_F7) & 1) { draw3DBoundingBoxActive = !draw3DBoundingBoxActive; }
		if (GetAsyncKeyState(VK_F8) & 1) { drawPlayerNamesActive = !drawPlayerNamesActive; }
		if (GetAsyncKeyState(VK_F9) & 1) { infiniteAmmoActive = !infiniteAmmoActive; }
		if (GetAsyncKeyState(VK_F10) & 1) { removeRecoilActive = !removeRecoilActive; }
		if (GetAsyncKeyState(VK_F11) & 1) { drawNamesActive = !drawNamesActive; }

		Sleep(20);
	}
}

BOOL WINAPI DllMain(HINSTANCE dll, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
	{
		DisableThreadLibraryCalls(dll);

		err = EngineInit() && CheatInit();

		if (err)
		{
			Dll = dll;
			auto t = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(ClearingThread), nullptr, 0, nullptr);
			if (t) { CloseHandle(t); }
		}

		return err;
	}
	case DLL_PROCESS_DETACH:
	{
		if (err) { CheatRemove(); };
		return true;
	}
	}
	return TRUE;
}

