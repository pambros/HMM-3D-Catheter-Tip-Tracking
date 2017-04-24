@set _MSVC_BIN=C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin
@set _MSVC_BIN_X64=%_MSVC_BIN%\amd64
@set _CMAKE="C:\Program Files (x86)\CMake\bin\cmake.exe"

@rem name of the compiler using cmake syntax
@set _COMPILER_NAME="Visual Studio 12 2013 Win64"

@set _CPU=x64

@rem @set _GDCM_DIR=C:/libs/gdcm
@rem @set _GDCM_BIN=%_GDCM_DIR%/bin

@set _ITK_DIR=C:/libs/itk
@set _ITK_BIN=%_ITK_DIR%/bin

@rem @set _ZLIB_DIR=C:/libs/zlib
@rem @set _ZLIB_BIN=%_ZLIB_DIR%/bin

@set PATH= 
@set PATH=%PATH%;C:\Windows\system32
@set PATH=%PATH%;%_MSVC_BIN_X64%
@set PATH=%PATH%;%_MSVC_BIN%
@set VS140COMNTOOLS=
@set VS120COMNTOOLS=C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\
@set VS110COMNTOOLS=

@rem ************************************** for TACE
@rem @set PATH=%PATH%;%_GDCM_BIN%
@set PATH=%PATH%;%_ITK_BIN%
@rem @set PATH=%PATH%;%_ZLIB_BIN%

@set _TACELIB_PATH=%_PROJECT_PATH%/cpp/generated/%_USER_SPECIFIC%/Release
@set _TACE_EXE=%_TACELIB_PATH%/TACE.exe