#include "ccallhook.h"

static CCallHook *arrow;

void __stdcall ShowClassicCursor()
{
    SetCursor(LoadCursor(NULL, IDC_ARROW));
}

void __stdcall mloop()
{
    HANDLE samp = GetModuleHandleA("samp.dll");
    if (samp == NULL || samp == INVALID_HANDLE_VALUE)
        return;

    static bool hooked = false;
    if (hooked)
        return;
    hooked = true;

    uint showCursor = (uint)samp + 0x9BD99;
    arrow = new CCallHook((void*)showCursor, sc_flags, 8, cp_skip);
    arrow->enable(ShowClassicCursor);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReasonForCall, LPVOID lpReserved)
{
    static CCallHook *mainloop;

    if (dwReasonForCall == DLL_PROCESS_ATTACH){
        mainloop = new CCallHook((void*)0x00748DA3,
                                 eSafeCall(sc_registers | sc_flags), 6);
        mainloop->enable(mloop);
    }
    else if (dwReasonForCall == DLL_PROCESS_DETACH){
        delete mainloop;
        delete arrow;
    }

    return TRUE;
}
