#pragma once



typedef struct _MODULE_INFO {
    PVOID Base;
    ULONG Size;
} MODULE_INFO, *PMODULE_INFO;



BOOL IsProcessElevated()
{
	BOOL fIsElevated = FALSE;
	HANDLE hToken = NULL;
	TOKEN_ELEVATION elevation;
	DWORD dwSize;

	if ( !OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) )
	{
        EPrint("OpenProcessToken failed! (0x%x)\n", GetLastError());
		goto clean;  // if Failed, we treat as False
	}


	if ( !GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize) )
	{	
        EPrint("GetTokenInformation failed! (0x%x)\n", GetLastError());
		goto clean;// if Failed, we treat as False
	}

	fIsElevated = elevation.TokenIsElevated;

clean:
	if (hToken)
	{
		CloseHandle(hToken);
		hToken = NULL;
	}
	return fIsElevated; 
}

HANDLE getProcHandle(
    _In_ PCHAR Name,
    _Out_opt_ PULONG Pid
)
{
    ULONG le = 0;
    HANDLE process = NULL;
    PROCESSENTRY32 entry = { 0 };
    HANDLE snapshot = INVALID_HANDLE_VALUE;
    
    if ( Pid )
        *Pid = 0;

    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if ( snapshot == INVALID_HANDLE_VALUE )
    {
        return process;
    }

    entry.dwSize = sizeof(PROCESSENTRY32);
    if ( Process32First(snapshot, &entry) == TRUE )
    {
        DPrint("search: %s\n", Name);
        while ( Process32Next(snapshot, &entry) == TRUE )
        {
            //DPrint(" entry: %s %u\n", entry.szExeFile, entry.th32ProcessID);
            if ( _stricmp(entry.szExeFile, Name) == 0 )
            {
                //process = OpenProcess(PROCESS_VM_READ|PROCESS_VM_WRITE, FALSE, entry.th32ProcessID);
                process = OpenProcess(PROCESS_ALL_ACCESS , FALSE, entry.th32ProcessID);
                if ( !process )
                {
                    le = GetLastError();
                }
                
                if ( Pid )
                    *Pid =  entry.th32ProcessID;

                break;
            }
        }
    }

    CloseHandle(snapshot);
    
    if ( !process && le == 0 )
    {
        SetLastError(ERROR_OBJECT_NOT_FOUND);
    }

    return process;
}

INT readProcessMemory(
    _In_ HANDLE Process,
    _In_ PVOID Buffer, 
    _In_ UINT32 BufferSize, 
    _In_ SIZE_T Offset,
    _In_ ULONG NewProtect
)
{
    PVOID baseAddr = (PVOID)Offset;
    BOOL success;
    SIZE_T bytesWritten = 0;
    DWORD le;
    DWORD oldProtect;
    
    if ( NewProtect )
    {
        success = VirtualProtectEx(Process, baseAddr, BufferSize, PAGE_READONLY, &oldProtect);
        if ( !success )
        {
            le = GetLastError();
            EPrint("VirtualProtectEx failed for address %p! (0x%x)\n", baseAddr, le);
            return le;
        }
    }

    success = ReadProcessMemory(Process, baseAddr, Buffer, BufferSize, &bytesWritten);
    if ( !success )
    {
        le = GetLastError();
        EPrint("Failed reading 0x%zx bytes at %p! (0x%x)\n", bytesWritten, baseAddr, le);
    }
    
    
    if ( NewProtect )
    {
        success = VirtualProtectEx(Process, baseAddr, BufferSize, oldProtect, &oldProtect);
        if ( !success )
        {
            le = GetLastError();
            EPrint("VirtualProtectEx failed for address %p! (0x%x)\n", baseAddr, le);
        }
    }

    return 0;
}

INT getProcessModule(
    _In_ UINT32 Pid,
    _In_ PCHAR Name,
    _Inout_ PMODULE_INFO ModuleInfo
)
{
    UINT16 i = 0;
    UINT32 s = ERROR_OBJECT_NOT_FOUND;
    HANDLE snapshot = INVALID_HANDLE_VALUE;
    MODULEENTRY32 me32;

    FPrint();
    DPrint_HD(Pid, "  ");
    DPrint_A(Name, "  ");
    
    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, Pid);
    if ( snapshot == INVALID_HANDLE_VALUE )
    {
        s = GetLastError();
        EPrint("CreateToolhelp32Snapshot TH32CS_SNAPMODULE failed for Pid 0x%x! (0x%x)\n", Pid, s);
        return s;
    }

    memset(&me32, 0, sizeof(MODULEENTRY32));
    me32.dwSize = sizeof(MODULEENTRY32);

    if ( !Module32First(snapshot, &me32) )
    {
        s = GetLastError();
        EPrint("Module32First failed! (0x%x)", s);
        return s;
    }

    DPrint("[Nr. Name | Base | Size]\n");
    do
    {
        i++;

        DPrint("%2u. %s:"
               " 0x%p |"
               " 0x%lx\n", 
               i, me32.szModule,
               me32.modBaseAddr,
               me32.modBaseSize);

        if ( _stricmp(me32.szModule, Name) == 0 )
        {
            DPrint("found!\n");

            ModuleInfo->Base = me32.modBaseAddr;
            ModuleInfo->Size = me32.modBaseSize;

            s = 0;
            break;
        }
    }
    while ( Module32Next(snapshot, &me32) );
    DPrint("\n");

    CloseHandle(snapshot);

    return s;
}

INT writeProcessMemory(
    _In_ HANDLE Process,
    _In_ PVOID Payload, 
    _In_ UINT32 PayloadSize, 
    _In_ SIZE_T Offset,
    _In_ ULONG NewProtect
)
{
    PVOID baseAddr = (PVOID)Offset;
    BOOL success;
    SIZE_T bytesWritten = 0;
    DWORD le;
    DWORD oldProtect;

    if ( NewProtect )
    {
        success = VirtualProtectEx(Process, baseAddr, PayloadSize, PAGE_READWRITE, &oldProtect);
        if ( !success )
        {
            le = GetLastError();
            EPrint("VirtualProtectEx failed for address %p! (0x%x)\n", baseAddr, le);
            return le;
        }
    }

    success = WriteProcessMemory(Process, baseAddr, Payload, PayloadSize, &bytesWritten);
    if ( !success )
    {
        le = GetLastError();
        EPrint("Failed writing 0x%zx bytes at %p! (0x%x)\n", bytesWritten, baseAddr, le);
    }
    
    if ( NewProtect )
    {
        success = VirtualProtectEx(Process, baseAddr, PayloadSize, oldProtect, &oldProtect);
        if ( !success )
        {
            le = GetLastError();
            EPrint("VirtualProtectEx failed for address %p! (0x%x)\n", baseAddr, le);
        }
    }

    return 0;
}
