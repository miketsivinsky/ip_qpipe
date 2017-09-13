@echo off

set DISABLE_QMAKE_DEFAULT_BUILD=1
set USE_VS_BUILD=0

if "%1"=="" (
	set TOPDIR=.
) else (
	set TOPDIR=%1
)

del %TOPDIR%\Makefile.*
del %TOPDIR%\src\Makefile.*
del %TOPDIR%\test\test_rx_pipe\Makefile.*
del %TOPDIR%\test\test_tx_pipe\Makefile.*

del %TOPDIR%\*.sln
del %TOPDIR%\src\*.vcxproj.*
del %TOPDIR%\test\test_rx_pipe\*.vcxproj.*
del %TOPDIR%\test\test_tx_pipe\*.vcxproj.*


rd  %TOPDIR%\build /S /Q 
rd  %TOPDIR%\bin /S /Q 

if %DISABLE_QMAKE_DEFAULT_BUILD%==0 (
	rd  %TOPDIR%\src\debug /S /Q 
	rd  %TOPDIR%\src\release /S /Q
	
	rd  %TOPDIR%\test\test_rx_pipe\debug /S /Q 
	rd  %TOPDIR%\test\test_rx_pipe\release /S /Q

	rd  %TOPDIR%\test\test_tx_pipe\debug /S /Q 
	rd  %TOPDIR%\test\test_tx_pipe\release /S /Q
)

if %USE_VS_BUILD%==1 (
	rd  %TOPDIR%\src\x64 /S /Q
	rd  %TOPDIR%\test\test_rx_pipe\x64 /S /Q
	rd  %TOPDIR%\test\test_tx_pipe\x64 /S /Q
)
