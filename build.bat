@echo off
setlocal enabledelayedexpansion

set my_name=%~n0
set my_dir="%~dp0"
set "my_dir=%my_dir:~1,-2%"

set /a exe=0
set /a cln=0
set /a verbose=0

set /a debug=0
set /a release=0
set /a bitness=64
set platform=x64
set configuration=Debug

:: option flags
:: add pdb
set /a OPT_FLAG_PDB=0x1
:: add rtl
set /a OPT_FLAG_RTL=0x2
SET /a flags=0

:: debug flags
set /a DP_FLAG=1
set /a EP_FLAG=2
set /a debug_print=%EP_FLAG%

set pts=v143

set prog_proj=fixPteBug.vcxproj



GOTO :ParseParams

:ParseParams

    IF [%1]==[] GOTO main
    if [%1]==[/?] goto help
    if [%1]==[/h] goto help
    if [%1]==[/help] goto help

    IF /i "%~1"=="/exe" (
        SET /a exe=1
        goto reParseParams
    )
    IF /i "%~1"=="/cln" (
        SET /a cln=1
        goto reParseParams
    )

    IF /i "%~1"=="/d" (
        SET /a debug=1
        goto reParseParams
    )
    IF /i "%~1"=="/r" (
        SET /a release=1
        goto reParseParams
    )
    
    :: print flags
    IF /i "%~1"=="/dp" (
        SET /a "debug_print=%~2"
        SHIFT
        goto reParseParams
    )
    IF /i "%~1"=="/dpf" (
        SET /a "debug_print=%debug_print%|DP_FLAG"
        goto reParseParams
    )
    IF /i "%~1"=="/epf" (
        set /a "debug_print=%debug_print%|EP_FLAG"
        goto reParseParams
    )
    
    :: option flags
    IF /i "%~1"=="/pdb" (
        SET /a "flags=%flags%|OPT_FLAG_PDB"
        goto reParseParams
    )
    IF /i "%~1"=="/rtl" (
        set /a "flags=%flags%|OPT_FLAG_RTL"
        goto reParseParams
    )

    IF /i "%~1"=="/pts" (
        SET pts=%~2
        SHIFT
        goto reParseParams
    )

    IF /i "%~1"=="/v" (
        SET /a verbose=1
        goto reParseParams
    ) ELSE (
        echo Unknown option : "%~1"
    )
    
    :reParseParams
    SHIFT
    if [%1]==[] goto main

GOTO :ParseParams


:main

    set /a "s=%debug%+%release%"
    if %s% == 0 (
        set /a debug=0
        set /a release=1
    )

    set /a "s=%exe%+%cln%"
    if %s% == 0 (
        set /a exe=1
    )

    if %verbose% == 1 (
        set /a "pdb=%flags%&OPT_FLAG_PDB"
        set /a "rtl=%flags%&OPT_FLAG_RTL"

        echo exe: %exe%
        echo cln: %cln%
        echo.
        echo debug: %debug%
        echo release: %release%
        echo bitness: %bitness%
        echo dprint: %debug_print%
        echo pdb: !pdb!
        echo rtl: !rtl!
        echo pts: %pts%
    )

    if %cln% == 1 (
        echo removing "%my_dir%\build"
        rmdir /s /q "%my_dir%\build" >nul 2>&1 
    )

    if %exe%==1 call :build %prog_proj%

    endlocal
    exit /B 0


:build
    SETLOCAL
        set proj=%~1
        if %debug%==1 call :buildEx %proj%,%platform%,Debug,%debug_print%,%flags%,%pts%
        if %release%==1 call :buildEx %proj%,%platform%,Release,%debug_print%,%flags%,%pts%
    ENDLOCAL
    
    EXIT /B %ERRORLEVEL%


:buildEx
    SETLOCAL
        set proj=%~1
        set platform=%~2
        set conf=%~3
        set /a dpf=%~4
        set flags=%~5
        set pte=%~6
        
        :: print flags
        set /a "dp=%dpf%&~EP_FLAG"
        set /a "ep=%dpf%&EP_FLAG"
        if %ep% NEQ 0 (
            set /a ep=1
        )

        :: option flags
        set /a "pdb=%flags%&OPT_FLAG_PDB"
        set /a "rtl=%flags%&OPT_FLAG_RTL"
        
        if %rtl% NEQ == 0 (
            set rtl=%conf%
        ) else (
            set rtl=None
        )

        :: pdbs
        if [%conf%] EQU [Debug] (
            set /a pdb=1
        )
        if %pdb% NEQ 0 (
            set /a pdb=1
        )
        
        if %verbose% EQU 1 (
            echo build
            echo  - Project=%proj%
            echo  - Platform=%platform%
            echo  - Configuration=%conf%
            echo  - DebugPrint=%dp%
            echo  - ErrorPrint=%ep%
            echo  - RuntimeLib=%rtl%
            echo  - PDB=%pdb%
            echo  - PTS=%pts%
            echo.
        )
        
        msbuild %proj% /p:Platform=%platform% /p:Configuration=%conf% /p:PlatformToolset=%pts% /p:DebugPrint=%dp% /p:ErrorPrint=%ep% /p:RuntimeLib=%rtl% /p:PDB=%pdb%
        echo.
        echo ----------------------------------------------------
        echo.
        echo.
    ENDLOCAL
    
    EXIT /B %ERRORLEVEL%


:usage
    echo Usage: %my_name% [/exe] [/cln] [/d] [/r] [/dp ^<flags^>] [/pdb] [/rtl]
    echo Default: %my_name% [/exe /r]
    exit /B 0
    
:help
    call :usage
    echo.
    echo Targets:
    echo /exe: Build fixPteBug.exe.
    echo /cln: Clean build folder.
    echo.
    echo Options:
    echo /d: Build in debug mode.
    echo /r: Build in release mode.
    echo /dp: Debug print flags. 1: Debug print, 2: Error print.
    echo /pdb: Compile with pdbs.
    echo /rtl: Compile with static RuntimeLibrary.
    echo /pts: msbuild PlatformToolSet. Default: v143
    echo.
    echo /v: More verbose mode.
    echo /h: Print this.
    echo.
    exit /B 0
