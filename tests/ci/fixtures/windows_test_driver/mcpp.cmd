@echo off
"%MOCK_POWERSHELL%" -NoLogo -NoProfile -NonInteractive -ExecutionPolicy Bypass -File "%~dp0mcpp.ps1" %*
exit /b %errorlevel%
