#ifndef WARNINGS_H
#define WARNINGS_H

// disable warnings in release and debug build
// 
//  4100: unreferenced formal parameter
//  4201: nonstandard extension used: nameless struct/union
//  4996: 'ExAllocatePoolWithTag': ExAllocatePoolWithTag is deprecated, use ExAllocatePool2
//  6320: Exception-filter expression is the constant EXCEPTION_EXECUTE_HANDLER
// 28175: member of _DRIVER_OBJECT should not be accessed
// 30029: A call was made to MmMapIoSpace(). This allocates executable memory.
// 30030: Warning: Allocating executable memory via specifying a MM_PAGE_PRIORITY type without a bitwise OR with MdlMappingNoExecute
// 28118: The current function is permitted to run at an IRQ level above the maximum permitted for '__PREfastPagedCode' (1).
// 28278: Function openLogFile appears with no prototype in scope. Only limited analysis can be performed.
#pragma warning( disable : 4100 4201 4996 6320 28175 30029 30030 28118 28278 )

// disable warnings just in debug build
// 
// 4101: 
// 4102: unreferenced label
// 4189: local variable is initialized but not referenced
// 4702: unreachable code
#ifdef DBG
#pragma warning( disable : 4101 4102 4189 4702 )
#endif

#endif