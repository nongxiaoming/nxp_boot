
@echo off

set astyle="AStyle.exe"


for /r . %%a in (*.cpp;*.c) do %astyle% --style=allman --indent=spaces=4 --pad-oper --pad-header --unpad-paren --suffix=none --align-pointer=name --lineend=linux --convert-tabs --verbose "%%a"

for /r . %%a in (*.hpp;*.h) do %astyle% --style=allman --indent=spaces=4 --pad-oper --pad-header --unpad-paren --suffix=none --align-pointer=name --lineend=linux --convert-tabs --verbose "%%a"


for /r . %%a in (*.orig) do del "%%a"

pause