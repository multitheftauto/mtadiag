/***************************************************************************** 
* 
* PROJECT: MTADiag
* LICENSE: GNU GPL v3
* FILE: CDiag.cpp
* PURPOSE: MTA diagnostic tool
* DEVELOPERS: Matthew "Towncivilian" Wolfe <ligushka@gmail.com>
* 
* 
* 
* Multi Theft Auto is available from http://www.multitheftauto.com/
* 
*****************************************************************************/ 

#include "CDiag.h"
#include "Curl.h"
#include "util.h"

// initialize everything
// used for storing environment variables & current system time
char                         CDiag::tempDir[255];
char                         CDiag::systemRoot[255];
SYSTEMTIME                   CDiag::sysTime;

// strings to store various paths
std::string                  CDiag::diagLogPath;
std::string                  CDiag::nightlyPath;
std::string                  CDiag::dxDiagLogPath;
std::string                  CDiag::taskListPath;
std::string                  CDiag::D3DX9_43Path;
std::string                  CDiag::MTAPath;
std::string                  CDiag::GTAPath;

// store current MTA version when GetMTAVersion() is called, and store the original version to dump in the log file
std::string                  CDiag::MTAVersion;
std::string                  CDiag::OriginalMTAVersion;

std::string                  CDiag::MTAVersionsInstalled[CUR_MTA_VERSIONS];  // array to store paths of all MTA versions currently installed
int                          CDiag::MTAVerChoice;                            // stores user's choice of which MTA version to diagnose

bool                         CDiag::DXUpdated;						         // was DirectX updated by MTADiag?

void CDiag::Init ( void )
{
	// obtain Temp and WINDOWS environment variables, and store system time
	GetEnvironmentVariable ( "temp", tempDir, buffSize );           // get the temp directory
	GetEnvironmentVariable ( "systemRoot", systemRoot, buffSize );  // get the WINDOWS directory
	GetLocalTime ( &sysTime );                                      // get the current system time

	// generate necessary file paths (MTADiag's own log, dxdiag, nightly exe download, task list)
	std::stringstream ss; // create a stringstream

	// append system time to MTADiag-Log filename
	ss << tempDir << "\\MTADiag-Log-" << sysTime.wYear << "-" << sysTime.wMonth << "-" << sysTime.wDay << "_" << sysTime.wHour << "-" << sysTime.wMinute << "-" << sysTime.wSecond << ".txt";
	diagLogPath = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	// append MTA nightly exe filename to temp directory path
	ss << tempDir << "\\MTANightly.exe";
	nightlyPath = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	// append dxdiag filename to temp directory path
	ss << tempDir << "\\dxdiag.log";
	dxDiagLogPath = ss.str();

	// clear the stringstream
	ss.str( "");
	ss.clear();

	// append tasklist filename to temp directory path
	ss << tempDir << "\\tasklist.txt";
	taskListPath = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	// append D3DX9_43.dll filename to system32 directory path
	ss << systemRoot << "\\system32\\D3DX9_43.dll";
	D3DX9_43Path = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	// poll all currently installed MTA versions; if there is more than one installed, ask the user to pick one
	if ( !PollMTAVersions() )
		UserPickVersion();
	// obtain GTA:SA's path and MTA's version
	GetGamePath();
	GetMTAVersion();
	OriginalMTAVersion = GetMTAVersion(); // store the original version to dump in the log file later on

	// update MTA to latest nightly/unstable build
	UpdateMTA();

	// check whether DirectX is up to date (actually whether D3DX9_43.dll is present in %systemroot%\system32)
	// and check if a D3D9.dll is present in the GTA:SA directory
	if ( IsDirectXUpToDate() ) { std::cout << "DirectX is up-to-date." << std::endl << std::endl; }
	else { UpdateDirectX(); DXUpdated = 1; }

	// generate a DXDiag log, a list of currently running processes, then concatenate those logs, MTA's logs, and some other miscellaneous info
	GenerateDXDiag();
	GenerateTaskList();
	ConcatenateLogs();
}

void CDiag::Destroy ( void )
{
	// clean up after ourselves
	remove ( dxDiagLogPath.c_str() );
	remove ( nightlyPath.c_str() );
	remove ( taskListPath.c_str() );
}

bool CDiag::PollMTAVersions ( void )
{
	MTAVersionsInstalled[1] = readRegKey ( MTAPathValue, MTA11PathSubKey ); // store MTA 1.1 path, if present
	MTAVersionsInstalled[2] = readRegKey ( MTAPathValue, MTA12PathSubKey ); // store MTA 1.2 path, if present
	MTAVersionsInstalled[3] = readRegKey ( MTAPathValue, MTA13PathSubKey ); // store MTA 1.3 path, if present
	MTAVersionsInstalled[4] = readRegKey ( MTAPathValue, MTA14PathSubKey ); // store MTA 1.4 path, if present

	// if a version isn't installed, "Failed to get key." is returned by readRegKey; clear that array element
	for ( int i = 1; i < CUR_MTA_VERSIONS; i++ )
	{
		if ( !strcmp ( MTAVersionsInstalled[i].c_str(), "Failed to read key." ) )
			MTAVersionsInstalled[i].assign( "" );
	}

	// check how many versions of MTA:SA are installed; if there's only one, we'll narrow it down and set MTAVerChoice to that version
	int versionCounter = 0;

	for (int i = 1; i < CUR_MTA_VERSIONS; i++)
	{
		if ( !MTAVersionsInstalled[i].empty() )
		{
			versionCounter++;
			MTAVerChoice = i;
		}
	}

	// there's only one version of MTA:SA installed, so return true and continue with diagnostics
	if ( versionCounter == 1 )
		return true;
	// the user doesn't seem to have any version of MTA:SA installed, is running a version of MTA:SA older than 1.1,
	// or is not running this program as Administrator when they should be
	else if ( versionCounter == 0 )
	{
		std::cout << "Can't read MTA path." << std::endl << "You are either not running this program as an Administrator," << std::endl;
		std::cout << "or you may be running a version of MTA older than 1.1." << std::endl;
		std::cout << "Update at www.mtasa.com, then run MTADiag again if necessary." << std::endl;
		system( "pause" );
		exit ( EXIT_FAILURE );
	}
	else
		return false; // return false signifying that there are multiple versions of MTA:SA installed
}

void CDiag::UserPickVersion ( void )
{
	std::cout << "You have multiple versions of MTA installed." << std::endl << "Please pick which version to update and diagnose:" << std::endl;

	// iterate through currently installed MTA versions and output them
	// it'd be nice to number these sequentually even if an MTA:SA version is missing, i.e. [1] 1.4 [2] 1.3 [3] 1.1 but meh, too much work for no gain
	for (int i = 1; i < CUR_MTA_VERSIONS; i++)
	{
		if ( !MTAVersionsInstalled[i].empty() )
			std::cout << "[" << i << "] 1." << i << std::endl;
	}
	// have the user pick between the versions
	do {
		std::cout << "Enter version choice: ";
		std::cin >> MTAVerChoice;

		if ( MTAVersionsInstalled[MTAVerChoice].empty() || MTAVerChoice >= CUR_MTA_VERSIONS )
			std::cout << "Invalid choice entered." << std::endl;

	} while ( MTAVersionsInstalled[MTAVerChoice].empty() || MTAVerChoice >= CUR_MTA_VERSIONS );
}

std::string CDiag::GetMTAPath ( void )
{
	switch ( MTAVerChoice )
	{
	case 1:
		MTAPath = readRegKey ( MTAPathValue, MTA11PathSubKey );
		return MTAPath;
		break;
	case 2:
		MTAPath = readRegKey ( MTAPathValue, MTA12PathSubKey );
		return MTAPath;
		break;
	case 3:
		MTAPath = readRegKey ( MTAPathValue, MTA13PathSubKey );
		return MTAPath;
		break;
	case 4:
		MTAPath = readRegKey ( MTAPathValue, MTA14PathSubKey );
		return MTAPath;
		break;
	}
	return "Unable to read MTA path.";
}

std::string CDiag::GetGamePath( void )
{
	GTAPath = readRegKey ( MTAGTAPathValue, MTAGTAPathSubKey );
	return GTAPath;
}

std::string CDiag::GetMTAVersion ( void )
{
	switch ( MTAVerChoice )
	{
	case 1:
		MTAVersion = readRegKey ( MTAVerValue, MTA11VerSubKey );
		return MTAVersion;
		break;
	case 2:
		MTAVersion = readRegKey ( MTAVerValue, MTA12VerSubKey );
		return MTAVersion;
		break;
	case 3:
		MTAVersion = readRegKey ( MTAVerValue, MTA13VerSubKey );
		return MTAVersion;
		break;
	case 4:
		MTAVersion = readRegKey ( MTAVerValue, MTA14VerSubKey );
		return MTAVersion;
		break;
	}
	return "Unable to read MTA version.";
}

void CDiag::UpdateMTA ( void )
{
	std::cout << "MTA install path: " << GetMTAPath() << std::endl;
	std::cout << "GTA install path: " << GTAPath << std::endl;
	std::cout << "MTA version: " << GetMTAVersion() << std::endl << std::endl;

	std::string url;
	char works;

	std::cout << "MTADiag will now download the latest patch of MTA:SA." << std::endl;

	switch ( MTAVerChoice )
	{
	case 1:
		url = MTA11NightlyURL;
		break;
	case 2:
		url = MTA12NightlyURL;
		break;
	case 3:
		url = MTA13NightlyURL;
		break;
	case 4:
		url = MTA14NightlyURL;
		break;
	}

	if ( downloadFile (url, nightlyPath) )
	{
		std::ifstream ifile ( nightlyPath.c_str() );
		if ( ifile )
		{
			std::cout << std::endl << "Launching the installer..." << std::endl;
			std::cout << "Run MTA once the installer has finished to see if it works now." << std::endl;
			system ( nightlyPath.c_str() );
		}
	}
	else
	{
		std::cout << "Unable to automatically download MTA patch. Launching download link..." << std::endl;
		system ("pause");
		ShellExecute ( NULL, "open", url.c_str(), NULL, NULL, SW_HIDE );
		std::cout << std::endl << "Install the patch. ";
	}

	std::cout << "If MTA works now, enter 'y' to quit MTADiag." << std::endl << "If it doesn't, enter 'n' to continue diagnostics." << std::endl;
	std::cin >> works;

	if ( works == 'y' )
	{
		std::cout << "Enjoy playing MTA!" << std::endl;
		Destroy();
		system ("pause");
		exit (EXIT_SUCCESS);
	}
	else
		std::cout << "MTA version is now: " << GetMTAVersion() << std::endl << std::endl;
}

bool CDiag::IsDirectXUpToDate ( void )
{
	std::ifstream ifile ( D3DX9_43Path.c_str() ); // check if D3DX9_43.dll is present in %systemroot%\system32 directory
	if ( ifile )
		return true;
	else
		return false;
}

void CDiag::UpdateDirectX ( void )
{
	std::string DXWebSetupPath;
	std::stringstream ss; // create a stringstream

	// append dxwebsetup.exe filename to temp directory path
	ss << tempDir << "\\dxwebsetup.exe";
	DXWebSetupPath = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	std::string dxWebSetupURL = "http://download.microsoft.com/download/1/7/1/1718CCC4-6315-4D8E-9543-8E28A4E18C4C/dxwebsetup.exe";

	// tell the user what we're doing
	std::cout << "DirectX is not up-to-date." << std::endl;
	std::cout << "Downloading web updater..." << std::endl;

	if ( downloadFile( dxWebSetupURL.c_str(), DXWebSetupPath.c_str() ) )
	{
		std::cout << std::endl << "Follow the instructions to update DirectX." << std::endl << std::endl;
		system( DXWebSetupPath.c_str() );
	}
	else
	{
		std::cout << "Unable to automatically download DirectX updater. Launching download link..." << std::endl;
		system ( "pause" );
		ShellExecute ( NULL, "open", dxWebSetupURL.c_str(), NULL, NULL, SW_HIDE );
		std::cout << "Continue when DirectX has finished updating." << std::endl;
		system( "pause" );
	}
	remove( DXWebSetupPath.c_str() );
}

bool CDiag::CheckForD3D9 ( void )
{
	SetCurrentDirectory( GTAPath.c_str() );

	std::ifstream ifile ( "D3D9.dll" ); // check if D3D9.dll is present in GTASA directory
	if ( ifile )
		return true;
	else
		return false;
}

void CDiag::GenerateDXDiag ( void )
{
	std::string DXLogPath;
	std::stringstream ss; // create a stringstream

	ss << "dxdiag /t" << dxDiagLogPath;
	DXLogPath = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	std::cout << "Generating DXDiag log. Please wait." << std::endl;

	system( DXLogPath.c_str() ); // cook up dxdiag log

	std::ifstream ifile( dxDiagLogPath.c_str() );
	if ( ifile )
		std::cout << "DXDiag log generated successfully." << std::endl << std::endl;
	else
		std::cout << "DXDiag log unable to be generated." << std::endl << std::endl;
}

void CDiag::GenerateTaskList ( void )
{
	std::string TaskListPath;
	std::stringstream ss; // create a stringstream

	ss << "tasklist >" << taskListPath.c_str();
	TaskListPath = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	std::cout << "Generating list of running processes. Please wait." << std::endl;

	system( TaskListPath.c_str() ); // cook up list of currently running processes

	std::ifstream ifile ( taskListPath.c_str() );
	if ( ifile )
		std::cout << "Process list generated successfully." << std::endl << std::endl;
	else
		std::cout << "Process list unable to be generated." << std::endl << std::endl;
}

bool CDiag::ConcatenateLogs ( void )
{
	SetCurrentDirectory(  MTAPath.c_str() );

	std::ifstream dxdiag ( dxDiagLogPath.c_str(), std::ios::in | std::ios::binary );
	if ( !dxdiag )
		std::cout << "Can't open dxdiag.log." << std::endl;

	std::ifstream tasklist ( taskListPath.c_str(), std::ios::in | std::ios::binary );
	if ( !tasklist )
		std::cout << "Can't open tasklist.txt." << std::endl;

	std::ifstream cegui ( "MTA\\cegui.log", std::ios::in | std::ios::binary );
	if ( !cegui )
		std::cout << "No CEGUI.log present. Have you tried launching MTA:SA at least once?" << std::endl;

	std::ifstream core ( "MTA\\core.log", std::ios::in | std::ios::binary );
	if ( !core )
		std::cout << "No core.log present." << std::endl;

	std::ifstream logfile ( "MTA\\logfile.txt", std::ios::in | std::ios::binary );
	if ( !logfile )
		std::cout << "No logfile.txt present. Have you tried launching MTA:SA at least once?" << std::endl;

	std::ofstream out ( diagLogPath.c_str(), std::ios::out | std::ios::binary );

	if ( !out )
	{
		std::cout << "Can't open output file." << std::endl << std::endl;
		return false;
	}

	// make a soup of logs
	out << "MTADiag v" << VERSION << " by Towncivilian" << std::endl;
	out << "Log generated on " << sysTime.wYear << "-" << sysTime.wMonth << "-" << sysTime.wDay << " " << sysTime.wHour << ":" << sysTime.wMinute << ":" << sysTime.wSecond << std::endl;
	out << "MTA path: " << MTAPath << std::endl;
	out << "GTA path: " << GTAPath << std::endl;
	out << "Old MTA version: " << OriginalMTAVersion << std::endl;
	out << "MTA version: " << MTAVersion << std::endl;
	std::string D3D9Present = ( CheckForD3D9() ) ? "Yes" : "No";
    out << "D3D9.dll present: " << D3D9Present << std::endl;        // check if user has a D3D9.dll that he didn't delete per MTA's recommendation
    std::string DirectXState = ( IsDirectXUpToDate() ) ? "Yes" : "No";
    out << "DirectX up-to-date: " << DirectXState << std::endl;     // ensure DirectX is up-to-date
    if ( DXUpdated == 1 )                                           // if DirectX was up-to-date already, no need to print this
		out << "DirectX was updated: Yes" << std::endl;
	out << std::endl;

	if ( dxdiag )
	{
	out << "dxdiag.log:" << std::endl << std::endl;
	out << dxdiag.rdbuf() << std::flush;
	out << std::endl << std::endl;
	dxdiag.close();
	}
	if ( tasklist )
	{
	out << "Running processes:" << std::endl << std::endl;
	out << tasklist.rdbuf() << std::flush;
	out << std::endl << std::endl;
	tasklist.close();
	}
	if ( cegui )
	{
	out << "CEGUI.log:" << std::endl << std::endl;
	out << cegui.rdbuf() << std::flush;
	out << std::endl << std::endl;
	cegui.close();
	}
	if ( core )
	{
	out << "core.log:" << std::endl << std::endl;
	out << core.rdbuf() << std::flush;
	core.close();
	}
	if ( logfile )
	{
	out << "logfile.txt:" << std::endl << std::endl;
	out << logfile.rdbuf() << std::flush;
	logfile.close();
	}

	out.close();

	std::cout << "Log files merged." << std::endl << std::endl;
	std::cout << "Please paste the contents of the opened Wordpad window at www.pastebin.com." << std::endl;
	std::cout << "Include the pastebin link in your forum post." << std::endl;
	ShellExecute ( NULL, "open", "wordpad.exe", diagLogPath.c_str(), NULL, SW_SHOW );

	return true;
}