@set _MSVC_DIR=C:/Program Files (x86)/Microsoft Visual Studio 12.0
@set _MSVC_BIN_DIR=%_MSVC_DIR%/VC/bin
@set _MSVC_BIN_X64_DIR=%_MSVC_BIN_DIR%/amd64

@set _CMAKE="C:/Program Files (x86)/CMake/bin/cmake.exe"

@rem name of the compiler using cmake syntax
@set _COMPILER_NAME="Visual Studio 12 2013 Win64"

@set _CPU=x64

@set _ITK_BUILD_DIR=C:/libs/itk
@set _ITK_BIN_DIR=%_ITK_BUILD_DIR%/bin

@set PATH= 
@set PATH=%PATH%;C:/Windows/system32
@set PATH=%PATH%;%_MSVC_BIN_X64_DIR%
@set PATH=%PATH%;%_MSVC_BIN_DIR%
@set VS150COMNTOOLS= 
@set VS140COMNTOOLS= 
@set VS120COMNTOOLS=%_MSVC_DIR%/Common7/Tools/
@set VS110COMNTOOLS= 
@set VS100COMNTOOLS= 

@rem ************************************** for TACE
@set PATH=%PATH%;%_ITK_BIN_DIR%

@set _TACE_BIN_DIR=%_PROJECT_PATH%/cpp/generated/%_USER_SPECIFIC%/Release
@set _TACE=%_TACE_BIN_DIR%/TACE.exe