:: Crude script for compiling. Works for now, but will need to be improved.

:: Delete main.exe
DEL main.exe

:: TODO: Make this script independent of the locaion of VC.
SET VC="C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC"

:: Only invoke vcvarsall.bat once a session. It appends to PATH
:: each time which eventually causes problems.
IF NOT DEFINED DevEnvDir (
    CALL %VC%\vcvarsall.bat x86
)

:: Invoke compiler.
%VC%\bin\cl /EHsc /Z7 main.cpp windows.cpp

:: Run the main file.
main
