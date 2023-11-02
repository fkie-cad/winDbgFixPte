#pragma once

#include "Files.h"

#define SYM_FLAG_CLEAN_CACHE (0x1)

#define SYM_DEFAULT_PDB_BASE (0x01000000)
#define SYM_DEFAULT_PDB_SIZE (0x01000000)

#define SYMBOLS_SERVER "https://msdl.microsoft.com/download/symbols"


VOID printSymInfo(
    _In_ PCHAR Label,
    _In_ PSYMBOL_INFO Info
)
{
    printf("%s\n", Label);
    printf("  Address: 0x%llx\n", Info->Address);
    printf("  Flags: 0x%x\n", Info->Flags);
    printf("  Index: 0x%x\n", Info->Index);
    printf("  MaxNameLen: 0x%x\n", Info->MaxNameLen);
    printf("  ModBase: 0x%llx\n", Info->ModBase);
    printf("  Name: %.*s\n", Info->NameLen, Info->Name);
    printf("  NameLen: 0x%x\n", Info->NameLen);
    printf("  Register: 0x%x\n", Info->Register);
    printf("  Reserved: %p\n", Info->Reserved);
    printf("  Scope: 0x%x\n", Info->Scope);
    printf("  Size: 0x%x\n", Info->Size);
    printf("  SizeOfStruct: 0x%x\n", Info->SizeOfStruct);
    printf("  Tag: 0x%x\n", Info->Tag);
    printf("  TypeIndex: 0x%x\n", Info->TypeIndex);
    printf("  Value: 0x%llx\n", Info->Value);
}

#define SYM_TYPE(__t__) \
    ( (__t__ == SymNone ) ? "None" \
    : (__t__ == SymCoff ) ? "SymCoff" \
    : (__t__ == SymCv ) ? "SymCv" \
    : (__t__ == SymPdb ) ? "SymPdb" \
    : (__t__ == SymExport ) ? "SymExport" \
    : (__t__ == SymDeferred ) ? "SymDeferred" \
    : (__t__ == SymSym ) ? "SymSym" \
    : (__t__ == SymDia ) ? "SymDia" \
    : (__t__ == SymVirtual ) ? "SymVirtual" \
    : (__t__ == NumSymTypes ) ? "NumSymTypes" \
    :  "None" )

VOID printImageHlpModule64(
    _In_ PIMAGEHLP_MODULE64 Module
)
{
    printf("IMAGEHLP_MODULE64\n");
    printf("  SizeOfStruct: 0x%x\n", Module->SizeOfStruct);
    printf("  BaseOfImage: 0x%llx\n", Module->BaseOfImage);
    printf("  ImageSize: 0x%x\n", Module->ImageSize);
    printf("  TimeDateStamp: 0x%x\n", Module->TimeDateStamp);
    printf("  CheckSum: 0x%x\n", Module->CheckSum);
    printf("  NumSyms: 0x%x\n", Module->NumSyms);
    printf("  SymType: 0x%x (%s)\n", Module->SymType, SYM_TYPE(Module->SymType));
    printf("  ModuleName: %s\n", Module->ModuleName);
    printf("  ImageName: %s\n", Module->ImageName);
    printf("  LoadedImageName: %s\n", Module->LoadedImageName);
    printf("  LoadedPdbName: %s\n", Module->LoadedPdbName);
    printf("  CVSig: 0x%x\n", Module->CVSig);
    printf("  CVData: %s\n", Module->CVData);
    printf("  PdbSig: 0x%x\n", Module->PdbSig);
    printf("  PdbSig70: %08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x\n", 
        Module->PdbSig70.Data1, 
        Module->PdbSig70.Data2, 
        Module->PdbSig70.Data3, 
        Module->PdbSig70.Data4[0], Module->PdbSig70.Data4[1], 
        Module->PdbSig70.Data4[2], Module->PdbSig70.Data4[3], Module->PdbSig70.Data4[4], Module->PdbSig70.Data4[5], Module->PdbSig70.Data4[6], Module->PdbSig70.Data4[7]);
    printf("  PdbAge: 0x%x\n", Module->PdbAge);
    printf("  PdbUnmatched: 0x%x\n", Module->PdbUnmatched);
    printf("  DbgUnmatched: 0x%x\n", Module->DbgUnmatched);
    printf("  LineNumbers: 0x%x\n", Module->LineNumbers);
    printf("  GlobalSymbols: 0x%x\n", Module->GlobalSymbols);
    printf("  TypeInfo: 0x%x\n", Module->TypeInfo);
    printf("  SourceIndexed: 0x%x\n", Module->SourceIndexed);
    printf("  Publics: 0x%x\n", Module->Publics);
    printf("  MachineType: 0x%x\n", Module->MachineType);
    printf("  Reserved: 0x%x\n", Module->Reserved);
}

INT findSymbol(
    _In_ HANDLE Process,
    _In_ PCHAR Name,
    _Out_ PSYMBOL_INFO* Info
)
{
    BOOL success;
    INT s = 0;
    //CHAR szSymbolName[MAX_SYM_NAME];
    SIZE_T bufferSize = sizeof(SYMBOL_INFO) + MAX_SYM_NAME;
    PUINT8 buffer = NULL;
    PSYMBOL_INFO info = NULL;

    *Info = NULL;

    //StringCchCopyA(szSymbolName, MAX_SYM_NAME, Name);
    buffer = malloc(bufferSize);
    if ( !buffer )
        goto clean;

    RtlZeroMemory(buffer, bufferSize);
    info = (PSYMBOL_INFO)buffer;
    info->SizeOfStruct = sizeof(SYMBOL_INFO);
    info->MaxNameLen = MAX_SYM_NAME;

    success = SymFromName(
                Process, 
                Name, 
                info
            );
    if ( !success )
    {
        s = GetLastError();
        EPrint("SymFromName for \"%s\" failed! (0x%x)\n", Name, s);
#ifdef ERROR_PRINT
        switch ( s )
        {
            case ERROR_NOT_FOUND:
                printf("    ERROR_NOT_FOUND\n"); break;
            default: break;
        }
#endif
        goto clean;
    }
    
    *Info = info;

clean:

    return s;
}

INT downloadPdb(
    _In_ PIMAGEHLP_MODULE64 Module,
    _In_ PCHAR OutPath
)
{
    INT s = 0;

    CHAR url[0x100];
    //PCHAR urlFormat = '{SYMBOLS_SERVER}/{pdb_file}/{guid}{debug_entry.Age:x}/{pdb_file}';
    PCHAR urlFormat = "%s/%s/%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x%x/%s";

    StringCchPrintfA(url, 0x100, urlFormat, 
        SYMBOLS_SERVER, 
        Module->CVData, 
        Module->PdbSig70.Data1, Module->PdbSig70.Data2, Module->PdbSig70.Data3, 
        Module->PdbSig70.Data4[0], Module->PdbSig70.Data4[1], 
        Module->PdbSig70.Data4[2], Module->PdbSig70.Data4[3], Module->PdbSig70.Data4[4], Module->PdbSig70.Data4[5], Module->PdbSig70.Data4[6], Module->PdbSig70.Data4[7], 
        Module->PdbAge, 
        Module->CVData);
    DPrint("url: %s\n", url);
    
    s = URLDownloadToFileA(
            NULL,
            url,
            OutPath,
            0,
            NULL
        );
    if ( s != 0 )
    {
        EPrint("Downloading \"%s\" failed! (0x%x)\n", url, s);
        goto clean;
    }

clean:
    return s;
}

/**
 * Would be more performant to parse pe header to get the pdb info, 
 * instead of calling SymLoadModuleEx with the dll first and the again with the pdb
 * 
 * Allocates SymbolInfo, has to be freed by caller
 */
INT getSymbolInfo(
    _In_ HANDLE Proc,
    _In_ PMODULE_INFO ModInfo,
    _In_ PCHAR DllName,
    _In_ PCHAR DllBaseName,
    _In_ PCHAR SymbolName,
    _Out_ PSYMBOL_INFO* SymbolInfo,
    _In_ ULONG Flags
)
{
    INT s = 0;
    ULONG cb = 0;
    BOOL success;

    ULONG64 baseOfDll = (ULONG64)ModInfo->Base;
    ULONG64 baseOfPdb = (ULONG64)ModInfo->Base;

    IMAGEHLP_MODULE64 moduleInfo = { 0 };

    PCHAR pdbPath = NULL;
    ULONG pdbPathSize = 0x200;
    UINT64 pdbBase = SYM_DEFAULT_PDB_BASE;
    ULONG pdbSize = SYM_DEFAULT_PDB_SIZE;
    
    *SymbolInfo = NULL;


    //
    // initialize symobls for process

    success = SymInitialize(
                Proc, 
                "", 
                FALSE
            );
    if ( !success )
    {
        s = GetLastError();
        EPrint("SymInitialize failed! (0x%x)\n", s);
        goto clean;
    }
    DPrint("SymInitialize succeeded!\n");
    
    success = SymSetOptions(
        0
        | SYMOPT_CASE_INSENSITIVE // All symbol searches are insensitive to case.
        | SYMOPT_UNDNAME // All symbols are presented in undecorated form.
        //| SYMOPT_LOAD_LINES // Loads line number information.
        //| SYMOPT_EXACT_SYMBOLS // Do not load an unmatched .pdb file. Do not load export symbols if all else fails.
        | SYMOPT_ALLOW_ABSOLUTE_SYMBOLS // Enables the use of symbols that are stored with absolute addresses. Most symbols are stored as RVAs from the base of the module
        | SYMOPT_AUTO_PUBLICS // Do not search the public symbols when searching for symbols by address, or when enumerating symbols, unless they were not found in the global symbols or within the current scope
        //| SYMOPT_DEBUG // Pass debug output through OutputDebugString or the SymRegisterCallbackProc64 callback function
    );
    if ( !success )
    {
        s = GetLastError();
        EPrint("SymInitialize failed! (0x%x)\n", s);
        goto clean;
    }


    //
    // load symbols of the dll to get pdb info

    baseOfDll = SymLoadModuleEx(
                    Proc,
                    INVALID_HANDLE_VALUE,
                    DllName,
                    NULL,
                    (UINT64)ModInfo->Base,
                    ModInfo->Size,
                    NULL,
                    0
                );
    if ( baseOfDll == 0 )
    {
        s = GetLastError();
        EPrint("SymLoadModuleEx for dll failed! (0x%x)\n", s);
        goto clean;
    }
    DPrint_H(baseOfDll, "");
    
    RtlZeroMemory(&moduleInfo, sizeof(moduleInfo));
    moduleInfo.SizeOfStruct  = sizeof(moduleInfo);
    success = SymGetModuleInfo64(
                Proc,
                (UINT64)ModInfo->Base,
                &moduleInfo
            );
    if ( !success )
    {
        s = GetLastError();
        EPrint("SymGetModuleInfo64 failed! (0x%x)\n", s);
        goto clean;
    }
#ifdef DEBUG_PRINT
    printImageHlpModule64(&moduleInfo);
#endif


    //
    // download corresponding pdb file

    pdbPath = malloc(pdbPathSize);
    if ( !pdbPath )
        goto clean;
    cb = GetTempPathA(pdbPathSize, pdbPath);
    if ( !cb || cb >= pdbPathSize )
    {
        s = GetLastError();
        EPrint("GetTempPathA failed! (0x%x)\n", s);
        goto clean;
    }
    StringCchPrintfA(&pdbPath[cb], pdbPathSize-cb, 
        "%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x%x-%s.pdb", 
        moduleInfo.PdbSig70.Data1, moduleInfo.PdbSig70.Data2, moduleInfo.PdbSig70.Data3, 
        moduleInfo.PdbSig70.Data4[0], moduleInfo.PdbSig70.Data4[1], 
        moduleInfo.PdbSig70.Data4[2], moduleInfo.PdbSig70.Data4[3], moduleInfo.PdbSig70.Data4[4], moduleInfo.PdbSig70.Data4[5], moduleInfo.PdbSig70.Data4[6], moduleInfo.PdbSig70.Data4[7],
        moduleInfo.PdbAge,
        DllBaseName);
    DPrint_A(pdbPath, "");
    
    if ( !fileExists(pdbPath) )
    {
        printf("Downloading pdb for %s\n", DllName);
        printf("...");
        s = downloadPdb(&moduleInfo, pdbPath);
        if ( s != 0 )
        {
            printf("\r   ");
            EPrint("Downloading Pdb file failed! (0x%x)\n", s);
            goto clean;
        }
        printf("\r   ");
        DPrint("[x] Pdb file downloaded!\n");
    }
    else
    {
        DPrint("Found existing pdb file at \"%s\"\n", pdbPath);
    }
    


    //
    // load symbols of the pdb file

    baseOfPdb = SymLoadModuleEx(
                    Proc,
                    INVALID_HANDLE_VALUE,
                    pdbPath,
                    DllBaseName,
                    (UINT64)pdbBase,
                    pdbSize,
                    NULL,
                    0
                );
    if ( baseOfPdb == 0 )
    {
        s = GetLastError();
        EPrint("SymLoadModuleEx for pdb failed! (0x%x)\n", s);
        goto clean;
    }
    DPrint_H(baseOfPdb, "");
    
//    RtlZeroMemory(&moduleInfo, sizeof(moduleInfo));
//    moduleInfo.SizeOfStruct  = sizeof(moduleInfo);
//    success = SymGetModuleInfo64(
//                Proc,
//                (UINT64)ModInfo->Base,
//                &moduleInfo
//            );
//    if ( !success )
//    {
//        s = GetLastError();
//        EPrint("SymGetModuleInfo64 failed! (0x%x)\n", s);
//        goto clean;
//    }
//#ifdef DEBUG_PRINT
//    printImageHlpModule64(&moduleInfo);
//#endif


    //
    // find the required symbol in the process

    s = findSymbol(Proc, SymbolName, SymbolInfo);
    if ( s != 0 )
    {
        goto clean;
    }

clean:
    if ( baseOfDll )
        SymUnloadModule(Proc, baseOfDll);
    if ( baseOfPdb )
        SymUnloadModule(Proc, baseOfPdb);

    if ( pdbPath )
    {
        if ( fileExists(pdbPath) && Flags & SYM_FLAG_CLEAN_CACHE )
            DeleteFileA(pdbPath);
        free(pdbPath);
    }

    return s;
}

INT getSymbolOffset(
    _In_ HANDLE Proc,
    _In_ PMODULE_INFO ModInfo,
    _In_ PCHAR DllName,
    _In_ PCHAR DllBaseName,
    _In_ PCHAR SymbolName,
    _Out_ PUINT64 SymbolOffset,
    _In_ ULONG Flags
)
{
    INT s = 0;

    PSYMBOL_INFO symbolInfo = NULL;

    *SymbolOffset = 0;

    DPrint_H(Flags, "");

    s = getSymbolInfo(Proc, ModInfo, DllName, DllBaseName, SymbolName, &symbolInfo, Flags);
    if ( s != 0 )
    {
        goto clean;
    }
#ifdef DEBUG_PRINT
    printSymInfo(DllBaseName, symbolInfo);
#endif

    //
    // return the offset
    
    *SymbolOffset = symbolInfo->Address - SYM_DEFAULT_PDB_BASE;

clean:
    if ( symbolInfo )
        free(symbolInfo);

    return s;
}
