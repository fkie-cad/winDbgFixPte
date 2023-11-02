#include <windows.h>
#include <Urlmon.h>
#include <tlhelp32.h>
#include <strsafe.h>

#include <dbghelp.h>
#include <dbgeng.h>

#include "warnings.h"
//#include "nt.h"
#include "print.h"
#include "Args.h"
#include "utils/process.h"
#include "utils/files.h"
#include "utils/sym.h"

#include "main.h"



#define DESIRED_PAGING_LEVEL_FLAG (0x04)



typedef struct _CMD_PARAMS {
    struct {
        UINT32 Verbose:1; // 1
        UINT32 CleanCache:1; // 2
        UINT32 Reserved:30;
    } Flags;
} CMD_PARAMS, *PCMD_PARAMS;



int parseParams(
    int argc, 
    char** argv, 
    PCMD_PARAMS Params,
    int start_i
);

void printUsage();

void printHelp();



int main(int argc, char** argv)
{
    int s = 0;

    HANDLE winDbgProc = NULL;
    ULONG winDbgPid = 0;
    PCHAR winDbgName = "windbg.exe";

    PCHAR kdextsName = "kdexts.dll";
    PCHAR kdextsBaseName = "kdexts";
    MODULE_INFO kdextsInfo = { 0 };
    PCHAR symbolName = "DbgPagingLevels";
    
    CMD_PARAMS params;
    
    printf("%s - %s\n\n", APP_NAME, APP_VS);
    printf("Compiled: %s -- %s\n\n", __DATE__, __TIME__);
    
    if ( isAskForHelp(argc, argv) )
    {
        printHelp();
        return 0;
    }

    if ( argc < 1 )
    {
        printUsage();
        return -1;
    }
    
    RtlZeroMemory(&params, sizeof(params));

    s = parseParams(argc, argv, &params, 1);
    if ( s != 0 )
    {
        printUsage();
        goto clean;
    }
    DPrint("Flags: 0x%x\n", *(PULONG)&params.Flags);
    
    if ( !IsProcessElevated() )
    {
        EPrint("Elevation required! Please run as Admin!\n");
        goto clean;
    }

    winDbgProc = getProcHandle(winDbgName, &winDbgPid);
    if ( !winDbgProc )
    {
        s = GetLastError();
        EPrint("Did not find a Process named \"%s\"! (0x%x)\n", winDbgName, s);
        goto clean;
    }

    printf("winDbg.exe\n");
    printf("  Proc: %p\n", winDbgProc);
    printf("  Pid: 0x%x\n", winDbgPid);

    s = getProcessModule(winDbgPid, kdextsName, &kdextsInfo);
    if ( s != 0 )
    {
        EPrint("Did not find a module named \"%s\"! (0x%x)\n", kdextsName, s);
        goto clean;
    }

    printf("kdexts.dll\n");
    printf("  Base: %p\n", kdextsInfo.Base);
    printf("  Size: 0x%x\n", kdextsInfo.Size);
    printf("\n");

    
    UINT64 pagingLevelsOffset = 0;
    s = getSymbolOffset(winDbgProc, &kdextsInfo, kdextsName, kdextsBaseName, symbolName, &pagingLevelsOffset, params.Flags.CleanCache);
    if ( s != 0 )
    {
        goto clean;
    }

    printf("\n");
    printf("Found %s\n", symbolName);
    printf("  rva: 0x%llx\n", pagingLevelsOffset);
    
    //
    // `DbgPagingLevels` should be readwrite 
    // cause it's in the .data section

    ULONG pagingLevels = 0;
    SIZE_T procOffset = (SIZE_T)kdextsInfo.Base + pagingLevelsOffset;
    s = readProcessMemory(winDbgProc, &pagingLevels, sizeof(pagingLevels), procOffset, 0);
    if ( s != 0 )
    {
        goto clean;
    }
    printf("  value: 0x%x\n", pagingLevels);

    if ( pagingLevels != DESIRED_PAGING_LEVEL_FLAG )
    {
        pagingLevels = DESIRED_PAGING_LEVEL_FLAG;
        s = writeProcessMemory(winDbgProc, &pagingLevels, sizeof(pagingLevels), procOffset, 0);
        if ( s != 0 )
        {
            goto clean;
        }
        printf("  set to 0x%x\n", pagingLevels);
        printf("Fixed!\n");
    }
    else
    {
        printf("Is fine.\n");
    }

clean:
    if ( winDbgProc )
        CloseHandle(winDbgProc);

    return s;
}

int parseParams(
    int argc, 
    char** argv, 
    PCMD_PARAMS Params,
    int start_i
)
{
    int s = 0;

    int i;
    char* arg;
    char *val1 = NULL;

    for ( i = start_i; i < argc; i++ )
    {
        arg = argv[i];
        val1 = GET_ARG_VALUE(argc, argv, i, 1);

        if ( IS_2C_ARG(arg, 'cc') )
        {
            Params->Flags.CleanCache = 1;
        }
        else if ( IS_1C_ARG(arg, 'v') )
        {
            Params->Flags.Verbose = 1;
        }
    }

    if ( s != 0 )
        return s;

    return s;
}

void printVersion()
{
    printf("Version: %s\n", APP_VS);
    printf("Last changed: %s\n", APP_LC);
    printf("Compiled: %s -- %s\n", __DATE__, __TIME__);
}

void printUsage()
{
    printf("Usage: %s"
            " [/cc]"
            " [/h]\n", 
            APP_NAME);
}

void printHelp()
{
    printVersion();
    printf("\n");
    printUsage();
    printf("\n");
    printf("Flags:\n");
    printf("/cc : Clean cached pdb file.\n");
    //printf("/v : Verbose output.\n");
    printf("/h : Print this.\n");
    printf("\n");
}
