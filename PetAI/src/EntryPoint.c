#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpReserved)  // reserved
{
    (void) hinstDLL;
    (void) fdwReason;
    (void) lpReserved;

    // Perform actions based on the reason for calling.
    switch(fdwReason) 
    { 
        case DLL_PROCESS_ATTACH:
         // Initialize once for each new process.
         // Return FALSE to fail DLL load.
            break;

        case DLL_THREAD_ATTACH:
         // Do thread-specific initialization.
            break;

        case DLL_THREAD_DETACH:
         // Do thread-specific cleanup.
            break;

        case DLL_PROCESS_DETACH:
         // Perform any necessary cleanup.
            break;
        default: break;
    }

    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

#endif
