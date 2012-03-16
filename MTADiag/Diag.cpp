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
//#define DEBUGOUTPUT
//#define VERBOSE // controls output of whether files are present during concatenation - the user doesn't really give a shit, no need to give them this info

std::vector<std::string>      Diag::files;

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
	if ( CheckForFile( files[8].c_str() ) ) { std::cout << "DirectX is up-to-date." << std::endl << std::endl; }
	else { UpdateDirectX(); DXUpdated = 1; }

	// generate a DXDiag log, a list of currently running processes, then concatenate those logs, MTA's logs, and some other miscellaneous info
	std::cout << "Gathering information. Please wait..." << std::endl << std::endl;

#ifndef SKIPDXDIAG
	DoSystemCommandWithOutput ( "dxdiag /t ", files[1] );
#endif
	DoSystemCommandWithOutput ( "tasklist >", files[2] );
	QueryWMIC ("Win32_VideoController");
	GetDirs();
	ConcatenateLogs();
}

void Diag::Destroy ( void )
{
	// clean up after ourselves
	// start at 1 since 0 is the generated log's path; we still need that
	for (int i = 1; i < (signed)files.size(); i++)
		remove ( files[i].c_str() );
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
	files.push_back ( ss.str() ); // files[0]

	// clear the stringstream
	ss.str ("");
	ss.clear();

	files.push_back ( tempDir + "\\dxdiag.log" ); // files[1] ...
	files.push_back ( tempDir + "\\tasklist.txt" );
	files.push_back ( tempDir + "\\WMIC.txt" );
	files.push_back ( tempDir + "\\mtadir.txt" );
	files.push_back ( tempDir + "\\gtadir.txt" );
	files.push_back ( tempDir + "\\gtamodels.txt" );
	files.push_back ( tempDir + "\\regexport.txt" );
	files.push_back ( systemRoot + "\\system32\\D3DX9_43.dll" );
	files.push_back ( tempDir + "\\MTANightly.exe" );
	files.push_back ( tempDir + "\\WMICUni.txt" );

#ifdef DEBUGOUTPUT
	for (int i = 0; i < (signed)files.size(); i++)
		std::cout << i << " " << files[i] << std::endl;
#endif
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

std::string Diag::GetGamePath( void )
{
	GTAPath = readRegKey ( MTAGTAPathValue, MTAGTAPathSubKey );
	return GTAPath;
}

void Diag::UpdateMTA ( void )
{
	std::cout << "MTA install path: " << GetMTAPath() << std::endl;
	std::cout << "MTA version:      " << GetMTAVersion() << std::endl;
	std::cout << "GTA install path: " << GTAPath << std::endl;
	std::cout << std::endl;

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
	if ( downloadFile (url, files[9].c_str() ) )
	{
		std::ifstream ifile ( files[9].c_str()  );
		if ( ifile )
		{
			std::cout << std::endl << "Launching the installer..." << std::endl;
			std::cout << "Run MTA once the installer has finished to see if it works now." << std::endl;
			system ( files[9].c_str()  );
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

void Diag::UpdateDirectX ( void )
{
	std::string DXWebSetupPath = tempDir;

	DXWebSetupPath.append ( "\\dxwebsetup.exe" );

	std::string DXWebSetupURL = "http://download.microsoft.com/download/1/7/1/1718CCC4-6315-4D8E-9543-8E28A4E18C4C/dxwebsetup.exe";

	// tell the user what we're doing
	std::cout << "DirectX is not up-to-date." << std::endl;
	std::cout << "Downloading web updater..." << std::endl;

	if ( downloadFile( DXWebSetupURL.c_str(), DXWebSetupPath.c_str() ) )
	{
		std::cout << std::endl << "Follow the instructions to update DirectX." << std::endl << std::endl;
		system( DXWebSetupPath.c_str() );
	}
	else
	{
		std::cout << "Unable to automatically download DirectX updater. Launching download link..." << std::endl;
		system ( "pause" );
		ShellExecute ( NULL, "open", DXWebSetupURL.c_str(), NULL, NULL, SW_HIDE );
		std::cout << "Continue when DirectX has finished updating." << std::endl;
		system( "pause" );
	}
	remove( DXWebSetupPath.c_str() );
}

void Diag::DoSystemCommandWithOutput ( std::string command, std::string outputfile )
{
	system ( ( command + outputfile ).c_str() );
}

void Diag::QueryWMIC ( std::string arg1, std::string arg2 )
{
	std::string WMIC;
	std::stringstream ss; // create a stringstream

	ss << "wmic Path " << arg1.c_str() << " Get " << arg2 << " >" << files[10].c_str();
	WMIC = ss.str ();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	system ( WMIC.c_str() );

	ConvertUnicodeToASCII ( files[10], files[3] );

	remove ( files[10].c_str() );
}

void Diag::ExportRegKey ( std::string subkey )
{
	std::string ExportReg;
	std::stringstream ss; // create a stringstream

	ss << "regedit /e /a " << files[7] << " \"" << subkey << "\"";
	ExportReg = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	system ( ExportReg.c_str() );
}

void Diag::GetDirs ( void )
{
	std::string dirPath1;
	std::string dirPath2;
	std::string dirPath3;
	std::string GTAModelsPath = ( GTAPath + "\\models" );
	std::string MTAInnerPath = ( MTAPath + "\\MTA" );
	std::stringstream ss; // create a stringstream

	ss << "dir \"" << MTAInnerPath << "\" >\"" << files[4].c_str() << "\"";
	dirPath1 = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	ss << "dir \"" << GTAPath << "\" >\"" << files[5].c_str() << "\"";
	dirPath2 = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	ss << "dir \"" << GTAModelsPath << "\" >\"" << files[6].c_str() << "\"";
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

	std::ifstream dxdiag ( files[1].c_str(), std::ios::in | std::ios::binary );
	#ifdef VERBOSE
	if ( !dxdiag )
		std::cout << "Can't open dxdiag.log." << std::endl;
	#endif

	std::ifstream tasklist ( files[2].c_str(), std::ios::in | std::ios::binary );
	#ifdef VERBOSE
	if ( !tasklist )
		std::cout << "Can't open tasklist.txt." << std::endl;
	#endif

	std::ifstream cegui ( "MTA\\cegui.log", std::ios::in | std::ios::binary );
	#ifdef VERBOSE
	if ( !cegui )
		std::cout << "No CEGUI.log present. Have you tried launching MTA:SA at least once?" << std::endl;
	#endif

	std::ifstream core ( "MTA\\core.log", std::ios::in | std::ios::binary );
	#ifdef VERBOSE
	if ( !core )
		std::cout << "No core.log present." << std::endl;
	#endif

	std::ifstream logfile ( "MTA\\logfile.txt", std::ios::in | std::ios::binary );
	#ifdef VERBOSE
	if ( !logfile )
		std::cout << "No logfile.txt present. Have you tried launching MTA:SA at least once?" << std::endl;
	#endif

	std::ifstream wmic ( files[3].c_str(), std::ios::in | std::ios::binary );
	#ifdef VERBOSE
	if ( !wmic )
		std::cout << "Can't open WMIC.txt." << std::endl;
	#endif

	std::ifstream mtadir ( files[4].c_str(), std::ios::in | std::ios::binary );
	#ifdef VERBOSE
	if ( !mtadir )
		std::cout << "Can't open mtadir.txt." << std::endl;
	#endif

	std::ifstream gtadir ( files[5].c_str(), std::ios::in | std::ios::binary );
	#ifdef VERBOSE
	if ( !gtadir )
		std::cout << "Can't open gtadir.txt." << std::endl;
	#endif

	std::ifstream gtamodelsdir ( files[6].c_str(), std::ios::in | std::ios::binary );
	#ifdef VERBOSE
	if ( !gtamodelsdir )
		std::cout << "Can't open gtamodelsdir.txt." << std::endl;
	#endif

	std::ofstream out ( files[0].c_str(), std::ios::out | std::ios::binary );

	if ( !out )
	{
		std::cout << "Can't open output file." << std::endl << std::endl;
		return false;
	}

	// make a soup of logs
	out << "MTADiag v" << VERSION << " by Towncivilian" << std::endl;
	out << "Log generated on    " << sysTime.wYear << "-" << sysTime.wMonth << "-" << sysTime.wDay << " " << sysTime.wHour << ":" << sysTime.wMinute << ":" << sysTime.wSecond << std::endl;
	out << "MTA path:           " << MTAPath << std::endl;
	out << "Old MTA version:    " << OriginalMTAVersion << std::endl;
	out << "MTA version:        " << MTAVersion << std::endl;
	out << "GTA path:           " << GTAPath << std::endl;
	std::string D3D9Present = ( CheckForFile ( GTAPath + "\\D3D9.dll" ) ) ? "Yes" : "No";
	out << "D3D9.dll present:   " << D3D9Present << std::endl;        // check if user has a D3D9.dll that he didn't delete per MTA's recommendation
	std::string DirectXState = ( CheckForFile ( files[8] ) ) ? "Yes" : "No";
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

	// not the greatest time to do these, but meh
	
	ExportRegKey ( CompatModeRegKey1 );

	std::ifstream reg1 ( files[7].c_str(), std::ios::in | std::ios::binary );
	if ( !reg1 )
		std::cout << "Can't open registryexport.txt." << std::endl;
	else
	{
		out << "Compatibility registry entries 1:" << std::endl << std::endl;
		out << reg1.rdbuf() << std::flush;
		reg1.close();
		out << std::endl;
	}

	ExportRegKey ( CompatModeRegKey2 );

	std::ifstream reg2 ( files[7].c_str(), std::ios::in | std::ios::binary );
	if ( !reg2 )
		std::cout << "Can't open registryexport.txt." << std::endl;
	else
	{
		out << "Compatibility registry entries 2:" << std::endl << std::endl;
		out << reg2.rdbuf() << std::flush;
		reg2.close();
		out << std::endl;
	}

	if ( wmic )
	{
	out << "WMIC Path Win32_VideoController Get:" << std::endl << std::endl;
	out << wmic.rdbuf() << std::flush;
	wmic.close();
	out << std::endl << std::endl;
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

	std::cout << "Log file generated." << std::endl << std::endl;
	std::cout << "Please paste the contents of the opened Wordpad window at www.pastebin.com." << std::endl;
	std::cout << "Include the Pastebin link in your forum post." << std::endl;
	ShellExecute ( NULL, "open", "wordpad.exe", files[0].c_str(), NULL, SW_SHOW );

	return true;
}