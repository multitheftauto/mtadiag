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
//#define DEBUGOUTPUT // output contents of files vector

std::vector<std::string>      Diag::files;

void Diag::Begin ( void )
{
	// obtain necessary environment variables and generate filepaths used for temporary files
	GeneratePaths();

	Log::Open ( files[0] ); // create the log file and open it

	Log::WriteStringToLog ( "MTADiag version", VERSION, false );
	Log::WriteStringToLog ( " by Towncivilian" );

	// poll all currently installed MTA versions; if there is more than one installed, ask the user to pick one
	if ( !PollMTAVersions() ) // PollMTAVersions will return true if there is only one version of MTA installed
		UserPickVersion();

	std::cout << "MTA install path: " << GetMTAPath() << std::endl;
	std::cout << "MTA version:      " << GetMTAVersion() << std::endl;
	std::cout << "GTA install path: " << GetGamePath() << std::endl;
	std::cout << std::endl;

	OriginalMTAVersion = GetMTAVersion(); // store the original version to dump in the log file later on

	// check whether DirectX is up to date (actually whether D3DX9_43.dll is present in %systemroot%\system32)
	if ( CheckForFile( files[4].c_str() ) ) { std::cout << "DirectX is up-to-date." << std::endl << std::endl; }
	else { UpdateDirectX(); DXUpdated = 1; }

	// remove any compatibility mode settings on gta_sa.exe and/or Multi Theft Auto.exe

	// check HKCU first
	CompatRemoved1 = DeleteCompatibilityEntries ( CompatModeRegKey, HKEY_CURRENT_USER );
	// check HKLM for compat. mode settings set for all users
	// only check 32-bit OSes due to compat. mode settings *NOT* being written to Wow6432Node on 64-bit OSes, and I can't seem to query non-Wow6432Node keys
	if ( !bIsWOW64 ) { CompatRemoved2 = DeleteCompatibilityEntries ( CompatModeRegKey, HKEY_LOCAL_MACHINE ); }

	// update MTA to latest nightly/unstable build, depending on the version
	UpdateMTA();

	// write a bunch of information to the log file since we just collected it
	Log::WriteStringToLog ( "MTA path:             ", MTAPath );
	Log::WriteStringToLog ( "Old MTA version:      ", OriginalMTAVersion );
	Log::WriteStringToLog ( "New MTA version:      ", MTAVersion );
	Log::WriteStringToLog ( "GTA path:             ", GTAPath );

	std::string D3D9Present = ( CheckForFile ( GTAPath + "\\D3D9.dll" ) ) ? "Yes" : "No";
	Log::WriteStringToLog ( "D3D9.dll present:     ", D3D9Present );

	std::string DirectXState = ( CheckForFile ( files[4] ) ) ? "Yes" : "No";
	Log::WriteStringToLog ( "DirectX up-to-date:   ", DirectXState );
	if ( DXUpdated == 1 )
		Log::WriteStringToLog ( "DirectX was updated:   Yes");

	if ( CompatRemoved1 == true || CompatRemoved2 == true )
		Log::WriteStringToLog ( "Compat. mode deleted:  Yes");
	Log::WriteStringToLog ( "" );

	// collect more information and output to log file
	std::cout << "Gathering information. Please wait..." << std::endl << std::endl;

	// gather the most useful system information first
#ifndef SKIPDXDIAG
	DoSystemCommandWithOutput ( "dxdiag /t " );
#endif
	DoSystemCommandWithOutput ( "tasklist >" );

	// write some of MTA's logs to our log
	Log::WriteFileToLog ( MTAPath + "\\MTA\\core.log", "core.log" );
	Log::WriteFileToLog ( MTAPath + "\\MTA\\logfile.txt", "logfile.txt" );
	Log::WriteFileToLog ( MTAPath + "\\MTA\\CEGUI.log", "CEGUI.log" );
	Log::WriteFileToLog ( MTAPath + "\\MTA\\timings.log", "timings.log" );
	if ( bIsVistaOrNewer ) { Log::WriteFileToLog ( programData + "\\MTA San Andreas All\\" + MTAShortVersion + "\\report.log", "report.log" ); }

	DoSystemCommandWithOutput ( "ipconfig /all >" ); // get network configuration
	DoSystemCommandWithOutput ( "wevtutil qe Application /q:\"Event [System [(Level=2)] ] [EventData [(Data='Multi Theft Auto.exe')] ]\" /c:1 /f:text /rd:true >" ); // might help resolve Visual C++ runtime issues
	QueryWMIC ( "Path", "Win32_VideoController", "Get" ); // get some video controller information

	// get directory listing of some folders
	GetDir ( ( MTAPath + "\\MTA" ) );
	GetDir ( GTAPath );
	GetDir ( ( GTAPath + "\\models" ) );

	// font diagnostics
	Log::WriteStringToLog ( "Verdana (TrueType) registry value:", ReadRegKey ( "Verdana (TrueType)", "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts\\" ) );
	Log::WriteStringToLog ( "" );
	GetDir ( systemRoot + "\\fonts\\verd*" );

	// close the log file for writing
	Log::Close();

	// upload to PasteBin
	std::cout << "Log file generated. Uploading to Pastebin..." << std::endl;

	PasteBinResult = Curl::CreatePasteBin ( files[0], logFileName ); // store the HTTP POST result into PasteBinResult

	// upload successful; copy URL to clipboard
	if ( strstr ( PasteBinResult.c_str(), "http://pastebin.com/" ) )
	{
		if ( CopyToClipboard ( PasteBinResult ) ) // was copying to clipboard successful?
		{
			std::cout << "Pastebin link copied to your clipboard." << std::endl << "Please include the Pastebin link in your forum post." << std::endl;
		}
		else // just in case that didn't work
		{
			std::cout << "Log file uploaded to Pastebin. Please include the Pastebin link in your forum post:" << std::endl;
			std::cout << PasteBinResult << std::endl;
		}
	}
	else // upload failure
	{
		std::cout << std::endl << std::endl << "Failed to upload log file to Pastebin." << std::endl;
		std::cout << "Error code: \"" << PasteBinResult << "\"" << std::endl;
		std::cout << "Please paste the contents of the opened Wordpad window at www.pastebin.com." << std::endl;
		std::cout << "Include the Pastebin link in your forum post." << std::endl << std::endl;	
		ShellExecute ( NULL, "open", "wordpad.exe", files[0].c_str(), NULL, SW_SHOW );
	}
}

void Diag::Cleanup ( void )
{
	// clean up after ourselves
	// start at 1 since 0 is the generated log's path; we still need that
	for ( unsigned int i = 1; i < files.size() - 1; i++) // don't delete D3DX9_43.dll
		remove ( files[i].c_str() );
}

void Diag::GeneratePaths ( void )
{
	// obtain Temp and WINDOWS environment variables, and store system time
	bIsVistaOrNewer = IsVistaOrNewer();      // is the user running Vista or newer?
	tempDir = getenv ( "Temp" );            // get the Temp directory
	systemRoot = getenv ( "SystemRoot" );	// get the WINDOWS directory
	if ( bIsVistaOrNewer ) { programData = getenv ( "ProgramData" ); } // get the ProgramData directory 
	IsWow64Process ( GetCurrentProcess(), &bIsWOW64 ); // is MTADiag running under WOW64?
	GetLocalTime ( &sysTime );              // get the current system time

	// generate necessary file paths
	std::stringstream ss;

	ss << "MTADiag-Log-" << sysTime.wYear << "-" << sysTime.wMonth << "-" << sysTime.wDay << "_" << sysTime.wHour << "-" << sysTime.wMinute << "-" << sysTime.wSecond;
	logFileName = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	files.push_back ( tempDir + "\\" + logFileName + ".txt" ); // files [0] ... / log file path
	files.push_back ( tempDir + "\\tempoutput.txt" ); // general temporary output file for almost everything
	files.push_back ( tempDir + "\\MTANightly.exe" ); // filepath for nightly
	files.push_back ( tempDir + "\\WMICUni.txt" ); // WMIC command outputs as Unicode; we convert this file to ASCII for proper insertion & formatting in the log
	files.push_back ( systemRoot + "\\system32\\D3DX9_43.dll" ); // we check for the presence of this file to determine if DirectX is up to date

	// output contents of files vector
#ifdef DEBUGOUTPUT
	for ( unsigned int i = 0; i < files.size(); i++ )
		std::cout << i << " " << files[i] << std::endl;
#endif
}

bool Diag::PollMTAVersions ( void )
{
	MTAVersionsInstalled[1] = ReadRegKey ( MTAPathValue, MTA11PathSubKey ); // store MTA 1.1 path, if present
	MTAVersionsInstalled[2] = ReadRegKey ( MTAPathValue, MTA12PathSubKey ); // store MTA 1.2 path, if present
	MTAVersionsInstalled[3] = ReadRegKey ( MTAPathValue, MTA13PathSubKey ); // store MTA 1.3 path, if present
	MTAVersionsInstalled[4] = ReadRegKey ( MTAPathValue, MTA14PathSubKey ); // store MTA 1.4 path, if present
	MTAVersionsInstalled[5] = ReadRegKey ( MTAPathValue, MTA15PathSubKey ); // store MTA 1.5 path, if present

	// if a version isn't installed, "Failed to get key." is returned by readRegKey; clear that array element
	for ( int i = 1; i <= CUR_MTA_VERSIONS; i++ )
	{
		if ( !strcmp ( MTAVersionsInstalled[i].c_str(), "Failed to read key." ) )
			MTAVersionsInstalled[i].assign( "" );
	}

	// check how many versions of MTA:SA are installed; if there's only one, we'll narrow it down and set MTAVerChoice to that version
	int versionCounter = 0;

	for (int i = 1; i <= CUR_MTA_VERSIONS; i++)
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
	// the user doesn't seem to have any version of MTA:SA installed; they are probably running a version of MTA:SA older than 1.1
	else if ( versionCounter == 0 )
	{
		std::cout << "Can't read MTA path." << std::endl;
		std::cout << "You may be running a version of MTA older than 1.1." << std::endl;
		std::cout << "Update at www.mtasa.com, then run MTADiag again if necessary." << std::endl;
		system ( "pause" );
		exit ( EXIT_FAILURE );
	}
	else
		return false; // return false, signifying that there are multiple versions of MTA:SA installed
}

void Diag::UserPickVersion ( void )
{
	std::cout << "You have multiple versions of MTA installed." << std::endl << "Please pick which version to update and diagnose:" << std::endl;

	// iterate through currently installed MTA versions and output them
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
		MTAPath = ReadRegKey ( MTAPathValue, MTA11PathSubKey );
		return MTAPath;
		break;
	case 2:
		MTAPath = ReadRegKey ( MTAPathValue, MTA12PathSubKey );
		return MTAPath;
		break;
	case 3:
		MTAPath = ReadRegKey ( MTAPathValue, MTA13PathSubKey );
		return MTAPath;
		break;
	case 4:
		MTAPath = ReadRegKey ( MTAPathValue, MTA14PathSubKey );
		return MTAPath;
		break;
	case 5:
		MTAPath = ReadRegKey ( MTAPathValue, MTA15PathSubKey );
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
		MTAVersion = ReadRegKey ( MTAVerValue, MTA11VerSubKey );
		MTAShortVersion = "1.1";
		return MTAVersion;
		break;
	case 2:
		MTAVersion = ReadRegKey ( MTAVerValue, MTA12VerSubKey );
		MTAShortVersion = "1.2";
		return MTAVersion;
		break;
	case 3:
		MTAVersion = ReadRegKey ( MTAVerValue, MTA13VerSubKey );
		MTAShortVersion = "1.3";
		return MTAVersion;
		break;
	case 4:
		MTAVersion = ReadRegKey ( MTAVerValue, MTA14VerSubKey );
		MTAShortVersion = "1.4";
		return MTAVersion;
		break;
	case 5:
		MTAVersion = ReadRegKey ( MTAVerValue, MTA15VerSubKey );
		MTAShortVersion = "1.5";
		return MTAVersion;
		break;
	}
	return "Unable to read MTA version.";
}

std::string Diag::GetGamePath( void )
{
	GTAPath = ReadRegKey ( MTAGTAPathValue, MTAGTAPathSubKey );
	return GTAPath;
}

void Diag::UpdateMTA ( void )
{
	std::string url;
	char works; // stores user's input

	std::cout << "MTADiag will now download the latest patch of MTA:SA." << std::endl;

	// set the nightly URL according to the version of MTA we're diagnosing
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
	case 5:
		url = MTA15NightlyURL;
		break;
	}

#ifndef SKIPUPDATE
	if ( Curl::DownloadFile ( url, files[2].c_str() ) ) // if the download was successful, open the installer
	{
		std::cout << std::endl << "Launching the installer..." << std::endl;
		std::cout << "Run MTA once the installer has finished to see if it works now." << std::endl;
		system ( files[2].c_str()  );
	}
	else // if the download failed, open a browser window to start the download of the nightly
	{
		std::cout << "Unable to automatically download MTA patch. Launching download link..." << std::endl;
		system ("pause"); // wait for user acknowledgement
		ShellExecute ( NULL, "open", url.c_str(), NULL, NULL, SW_HIDE );
		std::cout << std::endl << "Install the patch. ";
	}
#endif

	std::cout << "If MTA works now, enter 'y' to quit MTADiag." << std::endl << "If it doesn't, enter 'n' to continue diagnostics." << std::endl;
	std::cin >> works; // get user input

	if ( works == 'y' )
	{
		std::cout << "Enjoy playing MTA!" << std::endl;
		Cleanup(); // delete any temporary files
		remove ( Diag::files[0].c_str() ); // remove the generated MTADiag log
		system ( "pause" ); // wait for user acknowledgement
		exit ( EXIT_SUCCESS );
	}
	else
		std::cout << "MTA version is now: " << GetMTAVersion() << std::endl << std::endl; // tell the user the updated version string
}

void Diag::UpdateDirectX ( void )
{
	std::string DXWebSetupPath = ( tempDir + "\\dxwebsetup.exe" ); // get the temporary file path

	std::string DXWebSetupURL = "http://download.microsoft.com/download/1/7/1/1718CCC4-6315-4D8E-9543-8E28A4E18C4C/dxwebsetup.exe"; // DXWebSetup URL

	// tell the user what we're doing
	std::cout << "DirectX is not up-to-date." << std::endl;
	std::cout << "Downloading web updater..." << std::endl;

	// attempt to download the file
	if ( Curl::DownloadFile ( DXWebSetupURL.c_str(), DXWebSetupPath.c_str() ) ) // success! open the file
	{
		std::cout << std::endl << "Follow the instructions to update DirectX." << std::endl << std::endl;
		system ( DXWebSetupPath.c_str() );
	}
	else // failure; open a browser window to start the download of DXWebSetup
	{
		std::cout << "Unable to automatically download DirectX updater. Launching download link..." << std::endl;
		system ( "pause" ); // wait for user acknowledgement
		ShellExecute ( NULL, "open", DXWebSetupURL.c_str(), NULL, NULL, SW_HIDE );
		std::cout << "Continue when DirectX has finished updating." << std::endl;
		system ( "pause" ); // wait for user acknowledgement
	}
	remove( DXWebSetupPath.c_str() ); // delete the temporary file
}

void Diag::DoSystemCommandWithOutput ( std::string command )
{
	system ( ( command + files[1] ).c_str() ); // do the command

	Log::WriteFileToLog ( files[1], command ); // write the result to the log file with the command as a description
}

void Diag::QueryWMIC ( std::string arg1, std::string arg2, std::string arg3, std::string arg4 )
{
	std::string WMIC;
	std::stringstream ss; // create a stringstream

	ss << "wmic " << arg1 << " " << arg2 << " " << arg3 << " " << arg4 << " >" << files[3].c_str(); // wmic <arg1> <arg2> <arg3> <arg4>
	WMIC = ss.str ();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	system ( WMIC.c_str() ); // do it

	ConvertUnicodeToASCII ( files[3], files[1] ); // convert the Unicode-encoded result to ASCII for proper display in the log file

	remove ( files[3].c_str() ); // delete the Unicode-encoded log file

	Log::WriteFileToLog ( files[1], ( "WMIC " + arg1 + " " + arg2 + " " + arg3 + " " + arg4 ) ); // write the result to the log file with a description
}

void Diag::GetDir ( std::string directory )
{
	std::string dirPath;
	std::stringstream ss; // create a stringstream

	ss << "dir \"" << directory << "\" >\"" << files[1].c_str() << "\""; // dir "<filepath>"
	dirPath = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	system ( dirPath.c_str() ); // do it

	Log::WriteFileToLog ( files[1].c_str(), ( directory + " directory listing" ) ); // write the result to the log file with a description
}