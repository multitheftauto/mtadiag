:: This .bat file is designed to improve overall OS health as most MTA lag and crashes are the result of badly maintained, bugged out Windows installations.
:: NOTE: YOU GOT TO RESTART YOUR PC TWICE AFTER USING THIS, TO NOT RUN INTO PROBLEMS WITH EG. UAC permissions!
:: It will try to achieve this by performing several automated mainentance actions which are generally universal across Win versions ::
:: Examples of such actions:
:: == Clean up Management instrumentation (which MTA and other apps/reliant system drivers use intensively and thus can be a source of problems)
:: == Reset Windows update as it takes care of the Windows image, which in turn is vital for OS critical file integrity scanning.
:: == In addition, having the Win update enabled and working, will automatically run CBS with each next update (summary system file scan and repairs if needed) without manually doing so
:: == Fix networking interface (reset network stack) as some corruption there can cause CD** Timed out/connection lost errors, and MTA to crash e.g on netc.dll
:: == Check and enable some Windows OS-critical services to ensure that isn't the source of problems, it often is.
:: == Schedule a Windows disk error scan on next boot-up
:: == Running this file may reset certain Windows components such as: Windows firewall settings, Windows update settings, etc. Run at your own responsibility (also, like if you got customized settings)



:: Start services Windows relies as per implementation for the very next actions::
sc config bits start= auto
sc config wuauserv start= auto
sc config cryptsvc start= auto
sc config winmgmt start= auto
net start mpssvc
net start bits
net start wuauserv
net start cryptsvc
net start winmgmt

:: often the source of problems (WMI, which MTA communicates with and so do system and device drivers/services). Could cause buggy (graphics) card driver behaviour if not fixed.
@echo on
echo y|net stop winmgmt /y
winmgmt /salvagerepository
winmgmt /resetrepository
winmgmt /regserver
wmiprvse /regserver
sc config winmgmt start= disabled
%systemdrive%
cd %windir%\system32\wbem
for /f %%s in (‘dir /b *.dll’) do regsvr32 /s %%s
wmiprvse /regserver
winmgmt /regserver
sc config winmgmt start= auto
net start winmgmt
for /f %%s in (‘dir /s /b *.mof *.mfl’) do mofcomp %%s

:: Reset Windows update components to ensure system health-watchdogs like CBS and DISM summary repairs/scans will run successfully on future Windows updates. (gradually this will repair OS file corruption)
set b=0

:bits
set /a b=%b%+1
if %b% equ 3 (
   goto end1
)
net stop bits
echo Checking the bits service status.
sc query bits | findstr /I /C:"STOPPED"
if not %errorlevel%==0 (
    goto bits
)
goto loop2

:end1
cls
echo.
echo Cannot reset since bits service failed to stop.
echo.
pause
goto Start


:loop2
set w=0

:wuauserv
set /a w=%w%+1
if %w% equ 3 (
   goto end2
)
net stop wuauserv
echo Checking the wuauserv service status.
sc query wuauserv | findstr /I /C:"STOPPED"
if not %errorlevel%==0 (
    goto wuauserv
)
goto loop3

:end2
cls
echo.
echo Cannot reset since wuauserv service failed to stop.
echo.
pause
goto Start



:loop3
set app=0

:appidsvc
set /a app=%app%+1
if %app% equ 3 (
   goto end3
)
net stop appidsvc
echo Checking the appidsvc service status.
sc query appidsvc | findstr /I /C:"STOPPED"
if not %errorlevel%==0 (
    goto appidsvc
)
goto loop4

:end3
cls
echo.
echo Cannot reset since appidsvc service failed to stop.
echo.
pause
goto Start


:loop4
set c=0

:cryptsvc
set /a c=%c%+1
if %c% equ 3 (
   goto end4
)
net stop cryptsvc
echo Checking the cryptsvc service status.
sc query cryptsvc | findstr /I /C:"STOPPED"
if not %errorlevel%==0 (
    goto cryptsvc
)
goto Reset

:end4
cls
echo.
echo Cannot reset since cryptsvc service failed to stop.
echo.
pause
goto Start


:Reset
Ipconfig /flushdns
del /s /q /f "%ALLUSERSPROFILE%\Application Data\Microsoft\Network\Downloader\qmgr*.dat"
del /s /q /f "%ALLUSERSPROFILE%\Microsoft\Network\Downloader\qmgr*.dat"
del /s /q /f "%SYSTEMROOT%\Logs\WindowsUpdate\*"


if exist "%SYSTEMROOT%\winsxs\pending.xml.bak" del /s /q /f "%SYSTEMROOT%\winsxs\pending.xml.bak"
if exist "%SYSTEMROOT%\winsxs\pending.xml" (
    takeown /f "%SYSTEMROOT%\winsxs\pending.xml"
    attrib -r -s -h /s /d "%SYSTEMROOT%\winsxs\pending.xml"
    ren "%SYSTEMROOT%\winsxs\pending.xml" pending.xml.bak
)

if exist "%SYSTEMROOT%\SoftwareDistribution.bak" rmdir /s /q "%SYSTEMROOT%\SoftwareDistribution.bak"
if exist "%SYSTEMROOT%\SoftwareDistribution" (
    attrib -r -s -h /s /d "%SYSTEMROOT%\SoftwareDistribution"
    ren "%SYSTEMROOT%\SoftwareDistribution" SoftwareDistribution.bak
)

if exist "%SYSTEMROOT%\system32\Catroot2.bak" rmdir /s /q "%SYSTEMROOT%\system32\Catroot2.bak"
if exist "%SYSTEMROOT%\system32\Catroot2" (
    attrib -r -s -h /s /d "%SYSTEMROOT%\system32\Catroot2"
    ren "%SYSTEMROOT%\system32\Catroot2" Catroot2.bak
)


:: Reset the BITS service to the default security descriptor
sc.exe sdset bits D:(A;;CCLCSWRPWPDTLOCRRC;;;SY)(A;;CCDCLCSWRPWPDTLOCRSDRCWDWO;;;BA)(A;;CCLCSWLOCRRC;;;AU)(A;;CCLCSWRPWPDTLOCRRC;;;PU)

sc.exe sdset wuauserv D:(A;;CCLCSWRPWPDTLOCRRC;;;SY)(A;;CCDCLCSWRPWPDTLOCRSDRCWDWO;;;BA)(A;;CCLCSWLOCRRC;;;AU)(A;;CCLCSWRPWPDTLOCRRC;;;PU)

:: Reregister the BITS files and the files
cd /d %windir%\system32
regsvr32.exe /s atl.dll
regsvr32.exe /s urlmon.dll
regsvr32.exe /s mshtml.dll
regsvr32.exe /s shdocvw.dll
regsvr32.exe /s browseui.dll
regsvr32.exe /s jscript.dll
regsvr32.exe /s vbscript.dll
regsvr32.exe /s scrrun.dll
regsvr32.exe /s msxml.dll
regsvr32.exe /s msxml3.dll
regsvr32.exe /s msxml6.dll
regsvr32.exe /s actxprxy.dll
regsvr32.exe /s softpub.dll
regsvr32.exe /s wintrust.dll
regsvr32.exe /s dssenh.dll
regsvr32.exe /s rsaenh.dll
regsvr32.exe /s gpkcsp.dll
regsvr32.exe /s sccbase.dll
regsvr32.exe /s slbcsp.dll
regsvr32.exe /s cryptdlg.dll
regsvr32.exe /s oleaut32.dll
regsvr32.exe /s ole32.dll
regsvr32.exe /s shell32.dll
regsvr32.exe /s initpki.dll
regsvr32.exe /s wuapi.dll
regsvr32.exe /s wuaueng.dll
regsvr32.exe /s wuaueng1.dll
regsvr32.exe /s wucltui.dll
regsvr32.exe /s wups.dll
regsvr32.exe /s wups2.dll
regsvr32.exe /s wuweb.dll
regsvr32.exe /s qmgr.dll
regsvr32.exe /s qmgrprxy.dll
regsvr32.exe /s wucltux.dll
regsvr32.exe /s muweb.dll
regsvr32.exe /s wuwebv.dll
regsvr32.exe /s wudriver.dll
netsh winsock reset
netsh winsock reset proxy

:: Set the startup type for OS-health critical services as automatic
sc config Winmgmt start= auto
sc config RpcLocator start= demand
sc config RpcSs start= auto
sc config RpcEptMapper start= auto
sc config wmiApSrv start= demand
sc config wuauserv start= auto
sc config WinHttpAutoProxySvc start= auto
sc config bits start= delayed-auto
sc config DcomLaunch start= auto
sc config MpsSvc start= auto
sc config BFE start= auto
sc config Appinfo start= demand
sc triggerinfo Appinfo start/machinepolicy
sc config ALG start= demand
sc config TrustedInstaller start= demand
sc config Wsearch start= delayed-auto
sc config Dhcp start= auto
sc config iphlpsvc start= auto
sc config nvspwmi start= auto
sc config EventSystem start= auto
sc config COMSysApp start= demand
sc config PeerDistSvc start= demand
sc config vmms start= auto
sc config WebClient start= demand
sc triggerinfo WebClient start/machinepolicy
sc config pla start= demand
sc config SysMain start= disabled
sc config dot3svc start= auto
sc config Wlansvc start= auto
sc config upnphost start= demand
sc config Netman start= demand
sc config ProfSvc start= auto
sc config PlugPlay start= auto
sc config NetTcpPortSharing start= disabled
sc config cryptsvc start= auto
sc config dnscache start= auto
sc config Power start= auto
sc config gpsvc start= auto
sc triggerinfo gpsvc start/machinepolicy
sc config sppsvc start= delayed-auto
sc triggerinfo sppsvc start/machinepolicy
sc triggerinfo dnscache start/machinepolicy
sc config Wcmsvc start= auto
sc triggerinfo Wcmsvc start/machinepolicy
sc config Audiosrv start= auto
sc config AudioEndpointBuilder start= auto
sc config WerSvc start= demand
sc triggerinfo WerSvc start/machinepolicy
sc config wudfsvc start= demand
sc triggerinfo wudfsvc start/machinepolicy
sc config msiserver start= demand
sc config FontCache3.0.0.0 start= demand
sc config W32Time start= demand
sc triggerinfo W32Time start/machinepolicy
sc config AppReadiness start= demand
sc config BrokerInfrastructure start= auto
sc config AppMgmt start= demand
sc config appidsvc start= demand
sc triggerinfo appidsvc start/machinepolicy

:: Prevent a virus from messing up MTA
sc config WinDefend start= auto

:Start
net start bits
net start wuauserv
net start appidsvc
net start cryptsvc

:: Reset networking interface for reasons mentioned at beginning of file
@echo on
netsh online
netsh winsock reset
netsh winhttp reset proxy
netsh interface ipv4 reset
netsh interface ipv6 reset
netsh int ip reset c:\resetLog.txt
netsh interface tcp reset
netsh interface portproxy reset
netsh interface httpstunnel reset
netsh rpc reset
netsh http flush logbuffer
netsh winsock reset all
netsh int 6to4 reset all
netsh int ipv4 reset all
netsh firewall delete
netsh int ipv6 reset all
rem netsh advfirewall reset
netsh int httpstunnel reset all
netsh int isatap reset all
netsh int portproxy reset all
netsh int tcp reset all
netsh int teredo reset all
netsh int tcp set global autotuninglevel=normal
netsh interface tcp set heuristics disabled
Reset Winsock control
netsh branchcache reset
netsh branchcache flush
fsutil resource setautoreset true C:\
fsutil resource setautoreset true
ipconfig /flushdns

:: schedule Disk error scan on next boot
echo y|chkdsk /F /V /R /X /B /spotfix

:: Some people never clean temp files and let their PC fill with junk so it gets slow. Do it for them
del C:\Windows\temp /S /Q /F
del C:\Windows\temp /S /Q /A:H
FOR /D %%p IN ("C:\Windows\temp\*") DO rmdir "%%p" /s /q

cd /D c:\
del /s/q *.tmp ~*.* *.partial

@echo off
  cd /d %temp%
  for /r %%a in (*.*) do (
  del /f /q "%%~a"
  ) >nul
  cd /d "%UserProfile%\Local Settings\Temporary Internet Files
   for /r %%b in (*.*) do (
   del /f /q "%%~b"
  ) >nul

  del C:\Windows\Prefetch\*.* /Q
  
 
winmgmt /resyncperf
msiexec /unregister
msiexec /regserver