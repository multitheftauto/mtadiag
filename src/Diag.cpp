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
#include "Diag.hpp"
#include "Curl.h"
#include "util.h"

//#define SKIPUPDATE
//#define SKIPDXDIAG
//#define SKIPFILECHECK
//#define DEBUGOUTPUT // output contents of files vector
#define SKIP_MTA_UPDATE // Disabled until status page is fixed

std::vector<std::string>      Diag::files;

void Diag::Begin ( void )
{
	// obtain necessary environment variables and generate filepaths used for temporary files
	GeneratePaths();

	Log::Open ( files[FILE_LOG] ); // create the log file and open it

	Log::WriteStringToLog ( "MTADiag version", VERSION, false );
	Log::WriteStringToLog ( " by Towncivilian" );
	std::cout << "MTADiag version " VERSION << std::endl;

	// poll all currently installed MTA versions; if there is more than one installed, ask the user to pick one
	if ( !PollMTAVersions() ) // PollMTAVersions will return true if there is only one version of MTA installed
		UserPickVersion();

	std::cout << "MTA install path: " << GetMTAPath() << std::endl;
	std::cout << "MTA version:      " << GetMTAVersion() << std::endl;
	std::cout << "GTA install path: " << GetGamePath() << std::endl;
	std::cout << std::endl;

	// check whether DirectX is up to date (actually whether D3DX9_43.dll is present in %systemroot%\system32)
	if ( CheckForFile ( files[FILE_D3DX9_DLL].c_str() ) ) { std::cout << "DirectX is up-to-date." << std::endl << std::endl; }
	else { UpdateDirectX(); DXUpdated = 1; }

	// remove any compatibility mode settings on gta_sa.exe and/or Multi Theft Auto.exe

	// check HKCU first
	CompatRemoved1 = DeleteCompatibilityEntries ( CompatModeRegKey, HKEY_CURRENT_USER );
	// check HKLM for compatibility mode settings set for all users
	// only check 32-bit OSes due to compatibility mode settings *NOT* being written to Wow6432Node on 64-bit OSes
	// and I can't seem to query non-Wow6432Node keys - maybe I'm doing something wrong?
	if ( !bIsWOW64 ) { CompatRemoved2 = DeleteCompatibilityEntries ( CompatModeRegKey, HKEY_LOCAL_MACHINE ); }
#ifndef SKIPFILECHECK
	// check for any missing GTA files
	std::cout << "Checking for missing GTA files, please wait..." << std::endl;

	for ( unsigned int i = 0; i < ( sizeof ( fileList ) / sizeof ( fileList[0] ) ); i++ )
	{
			std::string szFilename = fileList[i].szFilename;

			if ( !( CheckForFile ( GTAPath + szFilename ) ) )
			{
				std::cout << "Missing GTA file: " << fileList[i].szFilename << std::endl; // output any messed up file
				//bQuit = true; // we need to quit since the user's GTA install is probably screwed up
			}
			std::cout << "\rChecking " << ( i + 1 ) << " out of " << ( sizeof ( fileList ) / sizeof ( fileList[0] ) ) << "...";
	}
	std::cout << std::endl;
	if ( bQuit )
	{
		std::cout << "Your Grand Theft Auto installation is missing one or more files." << std::endl << "Please reinstall GTA and see if MTA works then." << std::endl;
		Cleanup ( true ); // clean up any temporary files that might have been created, along with the MTADiag log
		system ( "pause" );
		exit ( EXIT_FAILURE ); // exit
	};
#endif
#ifndef SKIP_MTA_UPDATE
	// check if MTA version matches the latest auto-update nightly
	if ( Curl::DownloadFile ( MTAVerURL, files[FILE_TEMP].c_str() ) ) // download the version appropriation HTML
	{
		std::string MTAVersionTrim = MTAVersion; // copy the MTAVersion string
		MTAVersionTrim.resize ( 15 ); // trim the MTAVersion string stored in registry to 15 characters
									  // the version appropriation HTML has a 15 char string lacking two trailing zeros
		if ( FindInFile ( files[FILE_TEMP].c_str(), ( MTAVersionTrim ) ) ) // look for the current MTA version string in it
			std::cout << "MTA is up-to-date." << std::endl << std::endl; // we've found it, hooray, we don't need to update MTA
		else
		{
			OriginalMTAVersion = GetMTAVersion(); // store the original version to dump in the log file later on
			UpdateMTA(); // update MTA to latest nightly / unstable build, depending on MTA's major version
		}
	}
#endif

	// write some information to the log file
	Log::WriteStringToLog ( "MTA path:             ", MTAPath );

	if ( MTAUpdated ) // was MTA updated? if so, list the old and new versions
	{
		Log::WriteStringToLog ( "Old MTA version:      ", OriginalMTAVersion );
		Log::WriteStringToLog ( "New MTA version:      ", MTAVersion );
	}
	else // list the current version of MTA
	{
		Log::WriteStringToLog ( "MTA version:          ", MTAVersion );
	}

	Log::WriteStringToLog ( "GTA path:             ", GTAPath );

	// is D3D9 present in GTA:SA directory despite MTA:SA's own warning?
	std::string D3D9Present = ( CheckForFile ( GTAPath + "\\D3D9.dll" ) ) ? "Yes" : "No";
	Log::WriteStringToLog ( "D3D9.dll present:     ", D3D9Present );

	// is DirectX up-to-date? (D3DX9_43.dll present)
	std::string DirectXState = ( CheckForFile ( files[FILE_D3DX9_DLL] ) ) ? "Yes" : "No";
	Log::WriteStringToLog ( "DirectX up-to-date:   ", DirectXState );
	if ( DXUpdated == 1 )
		Log::WriteStringToLog ( "DirectX was updated:   Yes");

	// was an erroneous compatibility mode setting removed?
	if ( CompatRemoved1 == true || CompatRemoved2 == true )
		Log::WriteStringToLog ( "Compat. mode deleted:  Yes");
	Log::WriteStringToLog ( "" );
#ifndef SKIPFILECHECK
	// check for any modified or nonstandard GTA files
	std::cout << "Checking for modified or nonstandard GTA files, please wait..." << std::endl;

	for ( unsigned int i = 0; i < ( sizeof ( fileList ) / sizeof ( fileList[0] ) ); i++ )
	{
			std::string szMd5 = fileList[i].szMd5;
			std::string szFilename = fileList[i].szFilename;
#ifdef _DEBUG
            // Speed up debugging
            if ( GetAsyncKeyState( VK_F1 ) )
                break;
#endif
            std::string strCalcedMd5 = GetFileMD5( GTAPath + szFilename );
            long long fileSize = GetFileSize( GTAPath + szFilename );
			if ( strCalcedMd5 != szMd5 && strCalcedMd5 != fileList[i].szMd5Alt )
			{
				std::cout << "Nonstandard GTA file: " << szFilename << std::endl;

				Log::WriteStringToLog ( "Nonstandard GTA file: ", szFilename );
	            std::stringstream ss; ss << "MD5sum for " << szFilename << ": " << strCalcedMd5 << "   Size: " << fileSize;
	            Log::WriteStringToLog ( ss.str() );
				Log::WriteStringToLog ( "Value should be: ", szMd5 );
				Log::WriteStringToLog ( "" );
			}
			std::cout << "\rChecking " << ( i + 1 ) << " out of " << ( sizeof ( fileList ) / sizeof ( fileList[0] ) ) << "...";
	}
	std::cout << std::endl;
#endif
	// collect more information and output to log file
	std::cout << "Gathering information. Please wait..." << std::endl;
	ProgressBar ( 0 );

	// gather the most useful system information first
#ifndef SKIPDXDIAG
	if ( DoSystemCommandWithOutput ( std::string( "dxdiag /dontskip /whql:off /t " ) + files[FILE_TEMP], OUTPUT_NONE, 120000 ) != ERROR_SUCCESS )
    {
        // If timed out, try again but allow dxdiag to skip problem area
	    ProgressBar ( 5 );
        DoSystemCommandWithOutput ( std::string( "dxdiag /whql:off /t " ) + files[FILE_TEMP], OUTPUT_NONE, 120000 );
    }
	ProgressBar ( 10 );
#endif
	DoSystemCommandWithOutput ( "tasklist", OUTPUT_ANSI, 120000 );
	ProgressBar ( 20 );
	DoSystemCommandWithOutput ( "net start", OUTPUT_ANSI, 120000 );
	ProgressBar ( 20 );

	// write some of MTA's logs to our log
	Log::WriteDividerToLog ();
	Log::WriteFileToLog ( MTAPath + "\\MTA\\core.log", "core.log" );

	// 1.4
	Log::WriteDividerToLog ();
	Log::WriteFileToLog ( MTAPath + "\\MTA\\logfile.txt", "logfile.txt" );
	Log::WriteDividerToLog ();
	Log::WriteFileToLog ( MTAPath + "\\MTA\\logfile_old.txt", "logfile_old.txt" );
	Log::WriteDividerToLog ();
	Log::WriteFileToLog ( MTAPath + "\\MTA\\CEGUI.log", "CEGUI.log" );
	// 1.5
	Log::WriteDividerToLog ();
	Log::WriteFileToLog ( MTAPath + "\\MTA\\logs\\logfile.txt", "logfile.txt" );
	Log::WriteDividerToLog ();
	Log::WriteFileToLog ( MTAPath + "\\MTA\\logs\\logfile.txt.1", "logfile.txt.1" );
	Log::WriteDividerToLog ();
	Log::WriteFileToLog ( MTAPath + "\\MTA\\logs\\CEGUI.log", "CEGUI.log" );

	Log::WriteDividerToLog ();
	Log::WriteFileToLog ( MTAPath + "\\timings.log", "timings.log" );
	Log::WriteDividerToLog ();
	Log::WriteFileToLog ( MTAPath + "\\mods\\deathmatch\\resources\\benchmark\\output\\bench.log", "bench.log" ); // FPS benchmark log
	if ( IsVistaOrNewer() )
    {
    	Log::WriteDividerToLog ();
        Log::WriteFileToLog ( programData + "\\MTA San Andreas All\\" + MTAShortVersion + "\\report.log", "report.log" );
    }

	ProgressBar ( 40 );

	DoSystemCommandWithOutput ( "ipconfig /all" ); // get network configuration
	ProgressBar ( 50 );
	QueryWMIC ( "Path", "Win32_VideoController", "Get" ); // get some video controller information
	ProgressBar ( 55 );

	// get directory listing of some folders
	GetDir ( ( MTAPath + "\\MTA" ) );
	GetDir ( GTAPath );
	GetDir ( ( GTAPath + "\\models" ) );

	ProgressBar ( 60 );

	// get relevant MD5sum(	s)
    Log::WriteDividerToLog ();
	Log::WriteStringToLog ( "MD5sum for gta_sa.exe: " + GetFileMD5 ( GTAPath + "\\gta_sa.exe" ) );
	//Log::WriteStringToLog ( "Value should be: 170b3a9108687b26da2d8901c6948a18 (HOODLUM 1.0)" );
	Log::WriteStringToLog ( "" );

	ProgressBar ( 80 );

	// font diagnostics
	// Uses same logic as CGUI_Impl::CreateFntFromWinFon
	auto CheckWinFont = [](const std::string& strFontWinReg, const std::string& strFontWinFile)
	{
		auto CheckWinFontFile = [](const std::string& strFilename, const std::string& strTag)
		{
			std::string strMd5 = GetFileMD5(strFilename);
			long long fileSize = GetFileSize(strFilename);
			std::stringstream ss; ss << "  MD5sum for " << strFilename << ": " << strMd5 << "   Size: " << fileSize << "  " << strTag;
			Log::WriteStringToLog(ss.str());
		};
		std::string strFontWinRegName = ReadRegKey( strFontWinReg, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts\\" );
		std::string strWinFontsPath = systemRoot + "\\fonts";
		std::stringstream ss; ss << "Checking " << strFontWinReg << " - Registry value:'" << strFontWinRegName << "'";
		Log::WriteStringToLog(ss.str());
		if ((strFontWinRegName.find(":") != std::string::npos) || strFontWinRegName[0] == '\\' || strFontWinRegName[0] == '/')
			CheckWinFontFile(strFontWinRegName, "(Registry value)");
		CheckWinFontFile(strWinFontsPath + "\\" + strFontWinRegName, "(Font path + registry value)");
		if (strFontWinRegName != strFontWinFile)
			CheckWinFontFile(strWinFontsPath + "\\" + strFontWinFile, "(Font path + default name)");
		Log::WriteStringToLog("");
	};

	CheckWinFont("Verdana (TrueType)", "verdana.ttf");
	CheckWinFont("Tahoma (TrueType)", "tahoma.ttf");
	CheckWinFont("Tahoma Bold (TrueType)", "tahomabd.ttf");
	GetDir(systemRoot + "\\Fonts\\verd*");
	GetDir(systemRoot + "\\Fonts\\tahoma*");

	ProgressBar ( 90 );

    // Do this last in case of problems
	if ( IsVistaOrNewer() )
    {
         // might help resolve Visual C++ runtime issues
        DoSystemCommandWithOutput ( "wevtutil qe Application /q:\"Event [System [(Level=2)] ] [EventData [(Data='Multi Theft Auto.exe')] ]\" /c:1 /f:text /rd:true" );
    }

	ProgressBar ( 100 ); std::cout << std::endl << std::endl;

	// close the log file for writing
	Log::Close();

	// upload to PasteBin
	std::cout << "Log file generated. Uploading to Pastebin..." << std::endl;

	PasteBinResult = Curl::CreateMTAPasteBin ( files[FILE_LOG], logFileName ); // store the HTTP POST result into PasteBinResult

	// upload successful; copy URL to clipboard
	if ( HasDigits ( PasteBinResult ) && !( strstr ( PasteBinResult.c_str(), "DOCTYPE" ) ) && !( strstr ( PasteBinResult.c_str(), "error" ) ) )
	{
		PasteBinResult.insert ( 0, "https://pastebin.mtasa.com/" );
		if ( CopyToClipboard ( PasteBinResult ) ) // was copying to clipboard successful?
		{
			std::cout << "Pastebin link (" << PasteBinResult << ") copied to your clipboard." << std::endl << "Please include the Pastebin link in your forum post." << std::endl;
		}
		else // just in case that didn't work
		{
			std::cout << "Log file uploaded to Pastebin. Please include the Pastebin link in your forum post:" << std::endl;
			std::cout << PasteBinResult << std::endl;
		}
	}
	else // upload failure, open the log in WordPad so the user can copy & paste it themselves
	{
		std::cout << std::endl << std::endl << "Failed to upload log file to MTA Pastebin." << std::endl;
		std::cout << "Error code: \"" << PasteBinResult << "\"" << std::endl;
		std::cout << "Please paste the contents of the opened Wordpad window at https://pastebin.mtasa.com" << std::endl;
		std::cout << "Include the MTA Pastebin link in your forum post." << std::endl << std::endl;	
		ShellExecute ( NULL, "open", "wordpad.exe", QuoteFilename( files[FILE_LOG] ).c_str(), NULL, SW_SHOW );
	}
}

void Diag::Cleanup ( bool deleteLog )
{
	// clean up after ourselves
	// start at 1 since 0 is the generated log's path; we still need that
	for ( unsigned int i = 1; i < files.size() - 1; i++) // don't delete D3DX9_43.dll (this file must always be last in the vector)
		remove ( files[i].c_str() ); // delete the file

	if ( deleteLog ) // do we need to delete our log? (if MTADiag quit prior to completing diagnostics)
	{
		Log::Close(); // close the log file for writing
		remove ( files[FILE_LOG].c_str() ); // remove the MTADiag log
	}
}

void Diag::GeneratePaths ( void )
{
	// obtain Temp and WINDOWS environment variables, and store system time
	tempDir = GetEnv ( "Temp" );                        // get the Temp directory
	systemRoot = GetEnv ( "SystemRoot" );	            // get the WINDOWS directory
	programData = GetEnv ( "AllUsersProfile" );         // get the ProgramData directory 
	IsWow64Process ( GetCurrentProcess(), &bIsWOW64 );  // is MTADiag running under WOW64?
	GetLocalTime ( &sysTime );                          // get the current system time
	bQuit = false;                                      // initialize quit bool, used in GTA files checking
	MTAUpdated = false;                                 // initialize MTAUpdated bool

	// Known folders without depending on env vars
	wchar_t szResult[MAX_PATH] = L"";
	if (SUCCEEDED(SHGetFolderPathW( NULL, CSIDL_WINDOWS, NULL, 0, szResult )))
		systemRoot = ToUTF8(szResult);
	if (SUCCEEDED(SHGetFolderPathW( NULL, CSIDL_COMMON_APPDATA, NULL, 0, szResult )))
		programData = ToUTF8(szResult);

    // Try to use 'MTA San Andreas All\MTADiag' as a temp dir first, so non admin can peek at it
    {
	    std::string altTempDir = programData + "\\MTA San Andreas All\\MTADiag";
        SHCreateDirectoryEx( NULL, altTempDir.c_str(), NULL );
        // Check can write there
	    std::string altTempDirTest = altTempDir + "\\test.txt";
	    std::ofstream file;
	    file.open ( FromUTF8(altTempDirTest).c_str(), std::ios::out );
	    if ( file )
        {
            tempDir = altTempDir;
            file.close();
            remove( altTempDirTest.c_str() );
        }
    }

	// generate MTADiag log file path
	std::stringstream ss;

	ss << "MTADiag-Log-" << sysTime.wYear << "-" << sysTime.wMonth << "-" << sysTime.wDay << "_" << sysTime.wHour << "-" << sysTime.wMinute << "-" << sysTime.wSecond;
	logFileName = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	files.push_back ( tempDir + "\\" + logFileName + ".txt" ); // files [0] ... / log file path
	files.push_back ( tempDir + "\\tempoutput.txt" ); // general temporary output file for almost everything
	files.push_back ( tempDir + "\\MTANightly.exe" ); // filepath for nightly installer
	files.push_back ( tempDir + "\\WMICUni.txt" ); // WMIC command outputs as Unicode; we convert this file to ASCII for proper insertion & formatting in the log
	files.push_back ( systemRoot + "\\system32\\D3DX9_43.dll" ); // we check for the presence of this file to determine if DirectX is up to date (this file must always be last in the vector)

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

	// if a version isn't installed, "Failed to get key." is returned by ReadRegKey; clear that array element
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
	std::cout << "You have multiple versions of MTA installed." << std::endl << "Please pick which version to diagnose by entering the number within the brackets:" << std::endl;

	// iterate through currently installed MTA versions and output them
	for (int i = 1; i <= CUR_MTA_VERSIONS; i++)
	{
		if ( !MTAVersionsInstalled[i].empty() )
			std::cout << "[" << i << "] 1." << i << std::endl;
	}
	// have the user pick between the versions
	bool success;
	do {
		success = true;
		std::cout << "Enter version choice: ";
		std::cin >> MTAVerChoice;

		if ( std::cin.fail() ) {
			std::cerr << "Invalid choice entered." << std::endl;
			std::cin.clear();
			std::cin.ignore ( std::numeric_limits<std::streamsize>::max(), '\n' );
			success = false;
		} else if ( MTAVersionsInstalled[MTAVerChoice].empty() || MTAVerChoice > CUR_MTA_VERSIONS ) {
			std::cout << "Invalid choice entered." << std::endl;
			success = false;
		}

	} while ( !success );
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

std::string Diag::GetGamePath ( void )
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
	if ( Curl::DownloadFile ( url, files[FILE_NIGHTLY_INSTALLER].c_str() ) ) // if the download was successful, open the installer
	{
		std::cout << std::endl << "Launching the installer..." << std::endl;
		std::cout << "Run MTA once the installer has finished to see if it works now." << std::endl;
		_wsystem ( FromUTF8(QuoteFilename( files[FILE_NIGHTLY_INSTALLER] )).c_str()  );
	}
	else // if the download failed, open a browser window to start the download of the nightly
	{
		std::cout << "Unable to automatically download MTA patch. Launching download link..." << std::endl;
		system ( "pause" ); // wait for user acknowledgement
		ShellExecute ( NULL, "open", url.c_str(), NULL, NULL, SW_HIDE );
		std::cout << std::endl << "Install the patch. ";
	}
#endif

	std::cout << "If MTA works now, enter 'y' to quit MTADiag." << std::endl << "If it doesn't, enter 'n' to continue diagnostics." << std::endl;
	std::cin >> works; // get user input

	if ( works == 'y' )
	{
		std::cout << "Enjoy playing MTA!" << std::endl;
		Cleanup ( true ); // delete any temporary files and the MTADiag log
		system ( "pause" ); // wait for user acknowledgement
		exit ( EXIT_SUCCESS );
	}
	else
	{
		std::cout << "MTA version is now: " << GetMTAVersion() << std::endl << std::endl; // tell the user the updated version string
		MTAUpdated = true; // bool to control log file output of old and new MTA versions
	}
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
		_wsystem ( FromUTF8(QuoteFilename( DXWebSetupPath )).c_str() );
	}
	else // failure; open a browser window to start the download of DXWebSetup
	{
		std::cout << "Unable to automatically download DirectX updater. Launching download link..." << std::endl;
		system ( "pause" ); // wait for user acknowledgement
		ShellExecute ( NULL, "open", DXWebSetupURL.c_str(), NULL, NULL, SW_HIDE );
		std::cout << "Continue when DirectX has finished updating." << std::endl;
		system ( "pause" ); // wait for user acknowledgement
	}
	remove ( DXWebSetupPath.c_str() ); // delete the temporary file
}

void Diag::QueryWMIC ( std::string arg1, std::string arg2, std::string arg3, std::string arg4 )
{
	std::string WMIC;
	std::stringstream ss; // create a stringstream

	ss << "wmic " << arg1 << " " << arg2 << " " << arg3 << " " << arg4;    // wmic <arg1> <arg2> <arg3> <arg4>
	WMIC = ss.str ();

    DoSystemCommandWithOutput ( WMIC, OUTPUT_UNICODE );
}

void Diag::GetDir ( std::string directory )
{
    Log::WriteDividerToLog ();
	std::string dirPath;
	std::stringstream ss; // create a stringstream

	ss << "dir \"" << directory << "\" >\"" << files[FILE_TEMP].c_str() << "\""; // dir "<filepath>"
	dirPath = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	_wsystem ( FromUTF8(dirPath).c_str() ); // do it

	Log::WriteFileToLog ( files[FILE_TEMP].c_str(), ( directory + " directory listing" ) ); // write the result to the log file with a description
}

DWORD Diag::DoSystemCommandWithOutput ( std::string command, int outputType, DWORD maxTimeMs )
{
	Log::WriteDividerToLog();
    time_t startTime = time( NULL );

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;       

    HANDLE h = NULL;
    if ( outputType != OUTPUT_NONE )
    {
        // File for output to be redirected to
        h = CreateFile( files[FILE_TEMP].c_str(),
            FILE_APPEND_DATA,
            FILE_SHARE_WRITE | FILE_SHARE_READ,
            &sa,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL );
        if ( h == NULL )
        {
	        Log::WriteStringToLog ( "**** DoSystemCommandWithOutput: Error CreateFile FILE_TEMP" );
        }
    }

    PROCESS_INFORMATION pi; 
    STARTUPINFO si;
    BOOL ret = FALSE; 
    DWORD flags = CREATE_NO_WINDOW;

    ZeroMemory( &pi, sizeof(PROCESS_INFORMATION) );
    ZeroMemory( &si, sizeof(STARTUPINFO) );
    si.cb = sizeof(STARTUPINFO); 
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdInput = NULL;
    si.hStdError = h;
    si.hStdOutput = h;

    ret = CreateProcess( NULL, (LPSTR)command.c_str(), NULL, NULL, TRUE, flags, NULL, NULL, &si, &pi );

    DWORD status = ERROR_INVALID_FUNCTION;
	std::stringstream ss;
    if ( ret == 0 )
    {
        DWORD dwError = GetLastError ();
	    ss << command << " - ERROR: Unable to run - CreateProcess error: " << dwError;
    }
    else
    {
        // Apply time limit for command to complete
        for( unsigned int i = 0 ; i < maxTimeMs ; i += 100 )
        {
            status = WaitForSingleObject ( pi.hProcess, 100 );
            if ( status != WAIT_TIMEOUT )
                break;
            ProgressBarInc();
        }
        if ( status == WAIT_TIMEOUT )
        {
            TerminateProcess ( pi.hProcess, 1 );
    	    ss << command << " - ERROR: Unable to complete - timed out after " << maxTimeMs << " ms";
            Sleep( 1000 );
        }
        else
        if ( status != ERROR_SUCCESS )
        {
            TerminateProcess ( pi.hProcess, 1 );
    	    ss << command << " - ERROR: Unable to complete - WaitForSingleObject status: " << status;
        }
        else
        {
            DWORD dwExitCode = 0xFFFFFFFF;
            GetExitCodeProcess( pi.hProcess, &dwExitCode );
	        ss << command << " (returned 0x" << std::hex << dwExitCode << ") [Took " << time( NULL ) - startTime << " seconds]";
        }
        CloseHandle( pi.hProcess );
        CloseHandle( pi.hThread );
    }

    CloseHandle( h );

    if ( outputType != OUTPUT_UNICODE )
    {
	    Log::WriteFileToLog ( files[FILE_TEMP], ss.str() ); // write the result to the log file with the passed command argument as a description
    }
    else
    {
	    ConvertUnicodeToASCII ( files[FILE_TEMP], files[FILE_WMIC_UNI] );
	    Log::WriteFileToLog ( files[FILE_WMIC_UNI], ss.str() );
	    remove ( files[FILE_WMIC_UNI].c_str() );
    }

    return status;
}
