#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Hook.hpp"

#ifdef _DEBUG
    #include <cstdio>
#endif

BOOL WINAPI DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    DisableThreadLibraryCalls(hModule);
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
#ifdef _DEBUG
        AllocConsole();
        AttachConsole(GetCurrentProcessId());
        SetConsoleTitleW(L"R6 Chams by Igromanru");
        freopen("CON", "w", stdout);
#endif
        if (const auto threadHandle = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(Hook::Init), nullptr, 0, nullptr))
        {
            CloseHandle(threadHandle);
            return TRUE;
        }
        return FALSE;
    case DLL_PROCESS_DETACH:
        Hook::Unload();
#ifdef _DEBUG
        Sleep(200);
        fclose(stdout);
        FreeConsole();
#endif
        return TRUE;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}
