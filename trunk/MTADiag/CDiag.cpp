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
char                    CDiag::tempDir[255];
char                    CDiag::systemRoot[255];
SYSTEMTIME              CDiag::sysTime;

// strings to store various paths
string                  CDiag::diagLogPath;
string                  CDiag::nightlyPath;
string                  CDiag::dxDiagLogPath;
string                  CDiag::taskListPath;
string                  CDiag::D3DX9_43Path;
string                  CDiag::MTAPath;
string                  CDiag::GTAPath;

// store current MTA version when GetMTAVersion() is called, and store the original version to dump in the log file
string                  CDiag::MTAVersion;
string                  CDiag::OriginalMTAVersion;

string                  CDiag::MTAVersionsInstalled[CUR_MTA_VERSIONS];  // array to store paths of all MTA versions currently installed
int                     CDiag::MTAVerChoice;                            // stores user's choice of which MTA version to diagnose

bool                    CDiag::DXUpdated;                               // was DirectX updated by MTADiag?

void CDiag::Init ( void )
{
	// obtain Temp and WINDOWS environment variables, and store system time
	GetEnvironmentVariable ( "temp", tempDir, buffSize );           // get the temp directory
	GetEnvironmentVariable ( "systemRoot", systemRoot, buffSize );  // get the WINDOWS directory
	GetLocalTime ( &sysTime );                                      // get the current system time

	// generate necessary file paths (MTADiag's own log, dxdiag, nightly exe download, task list)
	stringstream ss; // create a stringstream

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
	if ( IsDirectXUpToDate() ) { cout << "DirectX is up-to-date." << endl << endl; }
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
		cout << "Can't read MTA path." << endl << "You are either not running this program as an Administrator," << endl;
		cout << "or you may be running a version of MTA older than 1.1." << endl;
		cout << "Update at www.mtasa.com, then run MTADiag again if necessary." << endl;
		system( "pause" );
		exit ( EXIT_FAILURE );
	}
	else
		return false; // return false signifying that there are multiple versions of MTA:SA installed
}

void CDiag::UserPickVersion ( void )
{
	cout << "You have multiple versions of MTA installed." << endl << "Please pick which version to update and diagnose:" << endl;

	// iterate through currently installed MTA versions and output them
	// it'd be nice to number these sequentually even if an MTA:SA version is missing, i.e. [1] 1.4 [2] 1.3 [3] 1.1 but meh, too much work for no gain
	for (int i = 1; i < CUR_MTA_VERSIONS; i++)
	{
		if ( !MTAVersionsInstalled[i].empty() )
			cout << "[" << i << "] 1." << i << endl;
	}
	// have the user pick between the versions
	do {
		cout << "Enter version choice: ";
		cin >> MTAVerChoice;

		if ( MTAVersionsInstalled[MTAVerChoice].empty() || MTAVerChoice >= CUR_MTA_VERSIONS )
			cout << "Invalid choice entered." << endl;

	} while ( MTAVersionsInstalled[MTAVerChoice].empty() || MTAVerChoice >= CUR_MTA_VERSIONS );
}

string CDiag::GetMTAPath ( void )
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

string CDiag::GetGamePath( void )
{
	GTAPath = readRegKey ( MTAGTAPathValue, MTAGTAPathSubKey );
	return GTAPath;
}

string CDiag::GetMTAVersion ( void )
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
	cout << "MTA install path: " << GetMTAPath() << endl;
	cout << "GTA install path: " << GTAPath << endl;
	cout << "MTA version: " << GetMTAVersion() << endl << endl;

	char *url;
	url = new char[255];
	char works;

	cout << "MTADiag will now download the latest patch of MTA:SA." << endl;
	cout << "Downloading..." << endl;

	switch ( MTAVerChoice )
	{
	case 1:
		strcpy ( url, MTA11NightlyURL );
		break;
	case 2:
		strcpy ( url, MTA12NightlyURL );
		break;
	case 3:
		strcpy ( url, MTA13NightlyURL );
		break;
	case 4:
		strcpy ( url, MTA14NightlyURL );
		break;
	}

	if ( downloadFile (url, nightlyPath) )
	{
		ifstream ifile ( nightlyPath.c_str() );
		if ( ifile )
		{
			cout << endl << "Launching the installer..." << endl;
			cout << "Run MTA once the installer has finished to see if it works now." << endl;
			system ( nightlyPath.c_str() );
		}
	}
	else
	{
		cout << "Unable to automatically download MTA patch. Launching download link..." << endl;
		system ("pause");
		ShellExecute ( NULL, "open", url, NULL, NULL, SW_HIDE );
		cout << endl << "Install the patch. ";
	}

	cout << "If MTA works now, enter 'y' to quit MTADiag." << endl << "If it doesn't, enter 'n' to continue diagnostics." << endl;
	cin >> works;

	if ( works == 'y' )
	{
		cout << "Enjoy playing MTA!" << endl;
		Destroy();
		system ("pause");
		exit (EXIT_SUCCESS);
	}
	else
		cout << "MTA version is now: " << GetMTAVersion() << endl << endl;
}

bool CDiag::IsDirectXUpToDate ( void )
{
	ifstream ifile ( D3DX9_43Path.c_str() ); // check if D3DX9_43.dll is present in %systemroot%\system32 directory
	if ( ifile )
		return true;
	else
		return false;
}

void CDiag::UpdateDirectX ( void )
{
	string DXWebSetupPath;
	stringstream ss; // create a stringstream

	// append dxwebsetup.exe filename to temp directory path
	ss << tempDir << "\\dxwebsetup.exe";
	DXWebSetupPath = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	char *dxWebSetupURL;
	dxWebSetupURL = new char[97];
	dxWebSetupURL = "http://download.microsoft.com/download/1/7/1/1718CCC4-6315-4D8E-9543-8E28A4E18C4C/dxwebsetup.exe";

	// tell the user what we're doing
	cout << "DirectX is not up-to-date." << endl;
	cout << "Downloading web updater..." << endl;

	if ( downloadFile( dxWebSetupURL, DXWebSetupPath.c_str() ) )
	{
		cout << endl << "Follow the instructions to update DirectX." << endl << endl;
		system( DXWebSetupPath.c_str() );
	}
	else
	{
		cout << "Unable to automatically download DirectX updater. Launching download link..." << endl;
		system ( "pause" );
		ShellExecute ( NULL, "open", dxWebSetupURL, NULL, NULL, SW_HIDE );
		cout << "Continue when DirectX has finished updating." << endl;
		system( "pause" );
	}
	delete [] dxWebSetupURL;
	remove( DXWebSetupPath.c_str() );
}

bool CDiag::CheckForD3D9 ( void )
{
	SetCurrentDirectory( GTAPath.c_str() );

	ifstream ifile ( "D3D9.dll" ); // check if D3D9.dll is present in GTASA directory
	if ( ifile )
		return true;
	else
		return false;
}

void CDiag::GenerateDXDiag ( void )
{
	string DXLogPath;
	stringstream ss; // create a stringstream

	ss << "dxdiag /t" << dxDiagLogPath; // 
	DXLogPath = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	cout << "Generating DXDiag log. Please wait." << endl;

	system( DXLogPath.c_str() ); // cook up dxdiag log

	ifstream ifile( dxDiagLogPath.c_str() );
	if ( ifile )
		cout << "DXDiag log generated successfully." << endl << endl;
	else
		cout << "DXDiag log unable to be generated." << endl << endl;
}

void CDiag::GenerateTaskList ( void )
{
	string TaskListPath;
	stringstream ss; // create a stringstream

	ss << "tasklist >" << taskListPath.c_str(); // 
	TaskListPath = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	cout << "Generating list of running processes. Please wait." << endl;

	system( TaskListPath.c_str() ); // cook up list of currently running processes

	ifstream ifile ( taskListPath.c_str() );
	if ( ifile )
		cout << "Process list generated successfully." << endl << endl;
	else
		cout << "Process list unable to be generated." << endl << endl;
}

bool CDiag::ConcatenateLogs ( void )
{
	SetCurrentDirectory(  MTAPath.c_str() );

	ifstream dxdiag ( dxDiagLogPath.c_str(), ios::in | ios::binary );
	if ( !dxdiag )
		cout << "Can't open dxdiag.log." << endl;

	ifstream tasklist ( taskListPath.c_str(), ios::in | ios::binary );
	if ( !tasklist )
		cout << "Can't open tasklist.txt." << endl;

	ifstream cegui ( "MTA\\cegui.log", ios::in | ios::binary );
	if ( !cegui )
		cout << "No CEGUI.log present. Have you tried launching MTA:SA at least once?" << endl;

	ifstream core ( "MTA\\core.log", ios::in | ios::binary );
	if ( !core )
		cout << "No core.log present." << endl;

	ifstream logfile ( "MTA\\logfile.txt", ios::in | ios::binary );
	if ( !logfile )
		cout << "No logfile.txt present. Have you tried launching MTA:SA at least once?" << endl;

	ofstream out ( diagLogPath.c_str(), ios::out | ios::binary );

	if ( !out )
	{
		cout << "Can't open output file." << endl << endl;
		return false;
	}

	// make a soup of logs
	out << "MTADiag v" << VERSION << " by Towncivilian" << endl;
	out << "Log generated on " << sysTime.wYear << "-" << sysTime.wMonth << "-" << sysTime.wDay << " " << sysTime.wHour << ":" << sysTime.wMinute << ":" << sysTime.wSecond << endl;
	out << "MTA path: " << MTAPath << endl;
	out << "GTA Path: " << GTAPath << endl;
	out << "Old MTA version: " << OriginalMTAVersion << endl;
	out << "MTA version: " << MTAVersion << endl;
	string D3D9Present = ( CheckForD3D9() ) ? "Yes" : "No";
	out << "D3D9.dll present: " << D3D9Present << endl;				// check if user has a D3D9.dll that he didn't delete per MTA's recommendation
	string DirectXState = ( IsDirectXUpToDate() ) ? "Yes" : "No";
	out << "DirectX up-to-date: " << DirectXState << endl;			// ensure DirectX is up-to-date
	if ( DXUpdated == 1 )											// if DirectX was up-to-date already, no need to print this
		out << "DirectX was updated: Yes" << endl;
	out << endl;

	if ( dxdiag )
	{
	out << "dxdiag.log:" << endl << endl;
	out << dxdiag.rdbuf() << flush;
	out << endl << endl;
	dxdiag.close();
	}
	if ( tasklist )
	{
	out << "Running processes:" << endl << endl;
	out << tasklist.rdbuf() << flush;
	out << endl << endl;
	tasklist.close();
	}
	if ( cegui )
	{
	out << "CEGUI.log:" << endl << endl;
	out << cegui.rdbuf() << flush;
	out << endl << endl;
	cegui.close();
	}
	if ( core )
	{
	out << "core.log:" << endl << endl;
	out << core.rdbuf() << flush;
	core.close();
	}
	if ( logfile )
	{
	out << "logfile.txt:" << endl << endl;
	out << logfile.rdbuf() << flush;
	logfile.close();
	}

	out.close();

	cout << "Log files merged." << endl << endl;
	cout << "Please paste the contents of the opened Wordpad window at www.pastebin.com." << endl;
	cout << "Include the pastebin link in your forum post." << endl;
	ShellExecute ( NULL, "open", "wordpad.exe", diagLogPath.c_str(), NULL, SW_SHOW );

	return true;
}