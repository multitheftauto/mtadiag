/***************************************************************************** 
* 
* PROJECT: MTADiag
* LICENSE: GNU GPL v3
* FILE: Diag.cpp
* PURPOSE: MTA diagnostic tool
* DEVELOPERS: Matthew "Towncivilian" Wolfe <ligushka@gmail.com>
* 
* 
* 
* Multi Theft Auto is available from http://www.multitheftauto.com/
* 
*****************************************************************************/ 

#include "Diag.h"
#include "Curl.h"
#include "util.h"

//#define SKIPUPDATE
//#define SKIPDXDIAG

std::string      Diag::diagLogPath; // because of extern

void Diag::Init ( void )
{
	// obtain necessary environment variables and generate filepaths used for temporary files
	GeneratePaths();

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
	std::cout << "Gathering information. Please wait..." << std::endl << std::endl;
#ifndef SKIPDXDIAG
	GenerateDXDiag();
#endif
	GenerateTaskList();
	GetCompatModeSettings();
	GetDirs();
	ConcatenateLogs();
}

void Diag::Destroy ( void )
{
	// clean up after ourselves
	remove ( dxDiagLogPath.c_str() );
	remove ( nightlyPath.c_str() );
	remove ( taskListPath.c_str() );
	remove ( DirectoryListingPath.c_str() );
	remove ( CompatModeRegPath1.c_str() );
	remove ( CompatModeRegPath2.c_str() );
	remove ( dirTempPath1.c_str() );
	remove ( dirTempPath2.c_str() );
	remove ( dirTempPath3.c_str() );
}

bool Diag::PollMTAVersions ( void )
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

void Diag::UserPickVersion ( void )
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

std::string Diag::GetMTAPath ( void )
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

std::string Diag::GetGamePath( void )
{
	GTAPath = readRegKey ( MTAGTAPathValue, MTAGTAPathSubKey );
	return GTAPath;
}

std::string Diag::GetMTAVersion ( void )
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

void Diag::UpdateMTA ( void )
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
#ifndef SKIPUPDATE
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
#endif
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

bool Diag::IsDirectXUpToDate ( void )
{
	std::ifstream ifile ( D3DX9_43Path.c_str() ); // check if D3DX9_43.dll is present in %systemroot%\system32 directory
	if ( ifile )
		return true;
	else
		return false;
}

void Diag::UpdateDirectX ( void )
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

bool Diag::CheckForD3D9 ( void )
{
	SetCurrentDirectory( GTAPath.c_str() );

	std::ifstream ifile ( "D3D9.dll" ); // check if D3D9.dll is present in GTASA directory
	if ( ifile )
		return true;
	else
		return false;
}

void Diag::GenerateDXDiag ( void )
{
	std::string DXLogPath;
	std::stringstream ss; // create a stringstream

	ss << "dxdiag /t" << dxDiagLogPath;
	DXLogPath = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	std::cout << "Generating DXDiag log..." << std::endl;

	system ( DXLogPath.c_str() ); // cook up dxdiag log
}

void Diag::GenerateTaskList ( void )
{
	std::string TaskListPath;
	std::stringstream ss; // create a stringstream

	ss << "tasklist >" << taskListPath.c_str();
	TaskListPath = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	system ( TaskListPath.c_str() ); // cook up list of currently running processes
}

void Diag::GetCompatModeSettings ( void )
{
	std::string RegPath1;
	std::string RegPath2;
	std::stringstream ss; // create a stringstream

	ss << "regedit /e /a " << CompatModeRegPath1.c_str() << " \"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers\"";
	RegPath1 = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	ss << "regedit /e /a " << CompatModeRegPath2.c_str() << " \"HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers\"";
	RegPath2 = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	system ( RegPath1.c_str() );
	system ( RegPath2.c_str() );
}

void Diag::GetDirs ( void )
{
	std::string dirPath1;
	std::string dirPath2;
	std::string dirPath3;
	std::string GTAModelsPath;
	std::string MTAInnerPath;
	std::stringstream ss; // create a stringstream

	// append "\models" to GTA path
	ss << GTAPath << "\\models";
	GTAModelsPath = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	// append "\MTA" to MTA path
	ss << MTAPath << "\\MTA";
	MTAInnerPath = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	ss << "dir \"" << MTAInnerPath << "\" >\"" << dirTempPath1 << "\"";
	dirPath1 = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	ss << "dir \"" << GTAPath << "\" >\"" << dirTempPath2 << "\"";
	dirPath2 = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	ss << "dir \"" << GTAModelsPath << "\" >\"" << dirTempPath3 << "\"";
	dirPath3 = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	system ( dirPath1.c_str() );
	system ( dirPath2.c_str() );
	system ( dirPath3.c_str() );
}

bool Diag::ConcatenateLogs ( void )
{
	SetCurrentDirectory ( MTAPath.c_str() );

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

	std::ifstream reg1 ( CompatModeRegPath1.c_str(), std::ios::in | std::ios::binary );
	if ( !reg1 )
		std::cout << "Can't open RegCompat1.txt." << std::endl;

	std::ifstream reg2 ( CompatModeRegPath2.c_str(), std::ios::in | std::ios::binary );
	if ( !reg2 )
		std::cout << "Can't open RegCompat2.txt." << std::endl;

	std::ifstream mtadir ( dirTempPath1.c_str(), std::ios::in | std::ios::binary );
	if ( !mtadir )
		std::cout << "Can't open mtadir.txt." << std::endl;

	std::ifstream gtadir ( dirTempPath2.c_str(), std::ios::in | std::ios::binary );
	if ( !gtadir )
		std::cout << "Can't open gtadir.txt." << std::endl;

	std::ifstream gtamodelsdir ( dirTempPath3.c_str(), std::ios::in | std::ios::binary );
	if ( !gtamodelsdir )
		std::cout << "Can't open gtamodelsdir.txt." << std::endl;

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
	out << std::endl << std::endl;
	logfile.close();
	}
	if ( reg1 )
	{
	out << "Compatibility registry entries 1:" << std::endl << std::endl;
	out << reg1.rdbuf() << std::flush;
	reg1.close();
	}
	if ( reg2 )
	{
	out << "Compatibility registry entries 2:" << std::endl << std::endl;
	out << reg2.rdbuf() << std::flush;
	reg2.close();
	out << std::endl;
	}
	if ( mtadir )
	{
	out << "MTA directory listing:" << std::endl << std::endl;
	out << mtadir.rdbuf() << std::flush;
	mtadir.close();
	out << std::endl << std::endl;
	}
	if ( gtadir )
	{
	out << "GTA directory listing:" << std::endl << std::endl;
	out << gtadir.rdbuf() << std::flush;
	gtadir.close();
	out << std::endl<< std::endl;
	}
	if ( gtamodelsdir )
	{
	out << "GTA\\models directory listing:" << std::endl << std::endl;
	out << gtamodelsdir.rdbuf() << std::flush;
	gtamodelsdir.close();
	}

	out.close();

	std::cout << "Log files merged." << std::endl << std::endl;
	std::cout << "Please paste the contents of the opened Wordpad window at www.pastebin.com." << std::endl;
	std::cout << "Include the Pastebin link in your forum post." << std::endl;
	ShellExecute ( NULL, "open", "wordpad.exe", diagLogPath.c_str(), NULL, SW_SHOW );

	return true;
}

void Diag::GeneratePaths ( void )
{
	// obtain Temp and WINDOWS environment variables, and store system time
	tempDir = getenv ( "Temp" );            // get the Temp directory
	systemRoot = getenv ( "SystemRoot" );	// get the WINDOWS directory
	GetLocalTime ( &sysTime );              // get the current system time

	// generate necessary file paths

	std::stringstream ss;

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

	// append directory listing filename to temp directory path
	ss << tempDir << "\\DirectoryListing.txt";
	DirectoryListingPath = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	// append directory listing filename to temp directory path
	ss << tempDir << "\\mtadir.txt";
	dirTempPath1 = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	// append directory listing filename to temp directory path
	ss << tempDir << "\\gtadir.txt";
	dirTempPath2 = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	// append directory listing filename to temp directory path
	ss << tempDir << "\\gtamodelsdir.txt";
	dirTempPath3 = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	// append compatibility mode registry entry filename to temp directory path
	ss << tempDir << "\\RegCompat1.txt";
	CompatModeRegPath1 = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	// append compatibility mode registry entry filename to temp directory path
	ss << tempDir << "\\RegCompat2.txt";
	CompatModeRegPath2 = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	// append D3DX9_43.dll filename to system32 directory path
	ss << systemRoot << "\\system32\\D3DX9_43.dll";
	D3DX9_43Path = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();
}