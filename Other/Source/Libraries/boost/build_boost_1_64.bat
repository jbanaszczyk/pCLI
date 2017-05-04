@ECHO OFF
SETLOCAL

CALL .\bootstrap.bat

b2.exe address-model=32 architecture=x86 threading=multi --prefix=.\ --toolset=msvc-14.1 --build-type=complete --without-mpi --without-python -j %NUMBER_OF_PROCESSORS% --libdir=stage\msvc\141\Win32 install

b2.exe address-model=64 architecture=x86 threading=multi --prefix=.\ --toolset=msvc-14.1 --build-type=complete --without-mpi --without-python -j %NUMBER_OF_PROCESSORS% --libdir=stage\msvc\141\x64   install

REM b2.exe address-model=32 architecture=x86 threading=multi --prefix=.\ --toolset=msvc-14.0 --build-type=complete --without-mpi --without-python -j %NUMBER_OF_PROCESSORS% --libdir=stage\msvc\140\Win32 install

REM b2.exe address-model=64 architecture=x86 threading=multi --prefix=.\ --toolset=msvc-14.0 --build-type=complete --without-mpi --without-python -j %NUMBER_OF_PROCESSORS% --libdir=stage\msvc\140\x64   install

IF EXIST .\bin.v2 RMDIR .\bin.v2 /s/q

ENDLOCAL
@ECHO ON
