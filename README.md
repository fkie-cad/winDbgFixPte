# Fix WinDbg PTE Bug

Fixes the WinDbg `!pte` extension bug, not properly working sometimes in WinDbg >= VS 10.0.22621.755.

The extension is not working due to the `kdexts!DbgPagingLevels` is set to `0xFF` or other values than `0x04`.
```
0: kd> !pte fffff801`00000000
Levels not implemented for this platform
```
If it is set to `0x04`, the `!pte` command is working as expected.
This seems to be the only valid value.

This tool overwrites the value in memory and makes the `!pte` command working great again, 
if not working in an active WinDbg session.


## Version
1.0.3  
Last changed: 02.11.2023


## Requirements
- msbuild


## Build
```bash
$devcmd> build.bat [/?] [/d|/r] [/dp <value>] [/pts <platformToolset>] [/rtl] [/v]
```
`/pts` defaults to `v143` (for build tools 2022), other values, like `v142` (for build tools 2019) can be used, if building problems occur.


## Usage
Run in an elevated cmd.
```bash
$ fixPteBug.exe [/cc] [/h]
```

## Remarks

The `kdexts.dll` is not always loaded and may get unloaded. 
But as soon as the `!pte` command in WinDbg is issued it will be loaded.

An internet connection is required, for downloading a pdb file.  
The file is downloaded to the `%tmp%` folder and cached/not deleted.
It will automatically be deleted if run with the `/cc` flag, though.


## Copyright

Published under [GNU GENERAL PUBLIC LICENSE](LICENSE).
