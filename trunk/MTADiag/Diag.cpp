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
	if ( CheckForFile ( files[4].c_str() ) ) { std::cout << "DirectX is up-to-date." << std::endl << std::endl; }
	else { UpdateDirectX(); DXUpdated = 1; }

	// remove any compatibility mode settings on gta_sa.exe and/or Multi Theft Auto.exe

	// check HKCU first
	CompatRemoved1 = DeleteCompatibilityEntries ( CompatModeRegKey, HKEY_CURRENT_USER );
	// check HKLM for compat. mode settings set for all users
	// only check 32-bit OSes due to compat. mode settings *NOT* being written to Wow6432Node on 64-bit OSes, and I can't seem to query non-Wow6432Node keys
	if ( !bIsWOW64 ) { CompatRemoved2 = DeleteCompatibilityEntries ( CompatModeRegKey, HKEY_LOCAL_MACHINE ); }

	// check for any missing GTA files
	for ( unsigned int i = 0; i < GTAFiles.size(); i++ )
	{
		if ( !( CheckForFile ( ( GTAPath + GTAFiles[i].c_str() ) ) ) )
		{
			std::cout << "Missing GTA file: " << GTAFiles[i] << std::endl; // output any missing file
			bQuit = true; // we need to quit since the user's GTA install is probably screwed up
		}
	}
	if ( bQuit )
	{
		std::cout << "Your Grand Theft Auto installation is missing one or more files." << std::endl << "Please reinstall GTA and see if MTA works then." << std::endl;
		Cleanup ( true ); // clean up any temporary files that might have been created
		system ( "pause" );
		exit ( EXIT_FAILURE ); // exit
	};

	// check if MTA version matches the latest auto-update nightly
	if ( Curl::DownloadFile ( MTAVerURL, files[5].c_str() ) ) // download the version appropriation HTML
	{
		std::string MTAVersionTrim = MTAVersion; // copy the MTAVersion string
		MTAVersionTrim.resize ( 15 ); // trim the MTAVersion string stored in registry to 15 characters; the version appropriation HTML has a 15 char string lacking two trailing zeros
		if ( FindInFile ( files[5].c_str(), ( MTAVersionTrim ) ) ) // look for the current MTA version string in it
			std::cout << "MTA is up-to-date." << std::endl << std::endl; // we've found it, hooray, we don't need to update MTA
		else
			UpdateMTA(); // update MTA to latest nightly / unstable build, depending on MTA's major version
	}

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

	// is D3DX9_43.dll present? second check to ensure it was updated
	std::string DirectXState = ( CheckForFile ( files[4] ) ) ? "Yes" : "No";
	Log::WriteStringToLog ( "DirectX up-to-date:   ", DirectXState );
	if ( DXUpdated == 1 )
		Log::WriteStringToLog ( "DirectX was updated:   Yes");

	// was an erroneous compatibility mode setting removed?
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
	Log::WriteFileToLog ( MTAPath + "\\timings.log", "timings.log" );
	if ( IsVistaOrNewer() ) { Log::WriteFileToLog ( programData + "\\MTA San Andreas All\\" + MTAShortVersion + "\\report.log", "report.log" ); }

	DoSystemCommandWithOutput ( "ipconfig /all >" ); // get network configuration
	DoSystemCommandWithOutput ( "wevtutil qe Application /q:\"Event [System [(Level=2)] ] [EventData [(Data='Multi Theft Auto.exe')] ]\" /c:1 /f:text /rd:true >" ); // might help resolve Visual C++ runtime issues
	QueryWMIC ( "Path", "Win32_VideoController", "Get" ); // get some video controller information

	// get directory listing of some folders
	GetDir ( ( MTAPath + "\\MTA" ) );
	GetDir ( GTAPath );
	GetDir ( ( GTAPath + "\\models" ) );

	// get relevant MD5sums
	Log::WriteStringToLog ( GetFileMD5 ( GTAPath + "\\gta_sa.exe" ) );
	Log::WriteStringToLog ( "Value should be: 170b3a9108687b26da2d8901c6948a18 (HOODLUM 1.0)" );
	Log::WriteStringToLog ( GetFileMD5 ( GTAPath + "\\models\\gta3.img" ) );
	Log::WriteStringToLog ( "Value should be: 9282e0df8d7eee3c4a49b44758dd694d" );
	Log::WriteStringToLog ( "" );

	// font diagnostics
	Log::WriteStringToLog ( "Verdana (TrueType) registry value:", ReadRegKey ( "Verdana (TrueType)", "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts\\" ) );
	Log::WriteStringToLog ( GetFileMD5 ( systemRoot + "\\Fonts\\verdana.ttf" ) );
	Log::WriteStringToLog ( "Value should be: ba34b303291e36596759eb46ad9c51f2 (Win 8) / 6eee3713d2330d93183846f2d34f0976 (Win 7)" );
	Log::WriteStringToLog ( "" );
	GetDir ( systemRoot + "\\Fonts\\verd*" );

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
			std::cout << "Pastebin link (" << PasteBinResult << ") copied to your clipboard." << std::endl << "Please include the Pastebin link in your forum post." << std::endl;
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

void Diag::Cleanup ( bool deleteLog )
{
	// clean up after ourselves
	// start at 1 since 0 is the generated log's path; we still need that
	for ( unsigned int i = 1; i < files.size() - 1; i++) // don't delete D3DX9_43.dll
		remove ( files[i].c_str() ); // delete the file

	if ( deleteLog )
	{
		Log::Close(); // close the log file for writing
		remove ( files[0].c_str() ); // remove the MTADiag log
	}
}

void Diag::GeneratePaths ( void )
{
	// obtain Temp and WINDOWS environment variables, and store system time
	tempDir = getenv ( "Temp" );                        // get the Temp directory
	systemRoot = getenv ( "SystemRoot" );	            // get the WINDOWS directory
	programData = getenv ( "AllUsersProfile" );         // get the ProgramData directory 
	IsWow64Process ( GetCurrentProcess(), &bIsWOW64 );  // is MTADiag running under WOW64?
	GetLocalTime ( &sysTime );                          // get the current system time
	bQuit = false;                                      // initialize quit bool, used in GTA files checking
	MTAUpdated = false;                                 // initialize MTAUpdated bool

	// generate MTADiag log file path
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
	files.push_back ( tempDir + "\\ver.txt" ); // MTA's auto-update version appropriation

	// fill the GTAFiles vector
	GTAFiles.push_back ( "\\eax.dll" );
	GTAFiles.push_back ( "\\ogg.dll" );
	GTAFiles.push_back ( "\\stream.ini" );
	GTAFiles.push_back ( "\\vorbis.dll" );
	GTAFiles.push_back ( "\\vorbisFile.dll" );

	GTAFiles.push_back ( "\\anim\\anim.img" );
	GTAFiles.push_back ( "\\anim\\cuts.img" );
	GTAFiles.push_back ( "\\anim\\ped.ifp" );

	GTAFiles.push_back ( "\\audio\\CONFIG\\AudioEventHistory.txt" );
	GTAFiles.push_back ( "\\audio\\CONFIG\\BankLkup.dat" );
	GTAFiles.push_back ( "\\audio\\CONFIG\\BankSlot.dat" );
	GTAFiles.push_back ( "\\audio\\CONFIG\\EventVol.dat" );
	GTAFiles.push_back ( "\\audio\\CONFIG\\PakFiles.dat" );
	GTAFiles.push_back ( "\\audio\\CONFIG\\StrmPaks.dat" );
	GTAFiles.push_back ( "\\audio\\CONFIG\\TrakLkup.dat" );

	GTAFiles.push_back ( "\\audio\\SFX\\FEET" );
	GTAFiles.push_back ( "\\audio\\SFX\\GENRL" );
	GTAFiles.push_back ( "\\audio\\SFX\\PAIN_A" );
	GTAFiles.push_back ( "\\audio\\SFX\\SCRIPT" );
	GTAFiles.push_back ( "\\audio\\SFX\\SPC_EA" );
	GTAFiles.push_back ( "\\audio\\SFX\\SPC_FA" );
	GTAFiles.push_back ( "\\audio\\SFX\\SPC_GA" );
	GTAFiles.push_back ( "\\audio\\SFX\\SPC_NA" );
	GTAFiles.push_back ( "\\audio\\SFX\\SPC_PA" );

	GTAFiles.push_back ( "\\audio\\streams\\AA" );
	GTAFiles.push_back ( "\\audio\\streams\\ADVERTS" );
	GTAFiles.push_back ( "\\audio\\streams\\AMBIENCE" );
	GTAFiles.push_back ( "\\audio\\streams\\BEATS" );
	GTAFiles.push_back ( "\\audio\\streams\\CH" );
	GTAFiles.push_back ( "\\audio\\streams\\CO" );
	GTAFiles.push_back ( "\\audio\\streams\\CR" );
	GTAFiles.push_back ( "\\audio\\streams\\CUTSCENE" );
	GTAFiles.push_back ( "\\audio\\streams\\DS" );
	GTAFiles.push_back ( "\\audio\\streams\\HC" );
	GTAFiles.push_back ( "\\audio\\streams\\MH" );
	GTAFiles.push_back ( "\\audio\\streams\\MR" );
	GTAFiles.push_back ( "\\audio\\streams\\NJ" );
	GTAFiles.push_back ( "\\audio\\streams\\RE" );
	GTAFiles.push_back ( "\\audio\\streams\\RG" );
	GTAFiles.push_back ( "\\audio\\streams\\TK" );

	GTAFiles.push_back ( "\\data\\animgrp.dat" );
	GTAFiles.push_back ( "\\data\\animviewer.dat" );
	GTAFiles.push_back ( "\\data\\ar_stats.dat" );
	GTAFiles.push_back ( "\\data\\AudioEvents.txt" );
	GTAFiles.push_back ( "\\data\\carcols.dat" );
	GTAFiles.push_back ( "\\data\\cargrp.dat" );
	GTAFiles.push_back ( "\\data\\carmods.dat" );
	GTAFiles.push_back ( "\\data\\clothes.dat" );
	GTAFiles.push_back ( "\\data\\default.dat" );
	GTAFiles.push_back ( "\\data\\default.ide" );
	GTAFiles.push_back ( "\\data\\fonts.dat" );
	GTAFiles.push_back ( "\\data\\furnitur.dat" );
	GTAFiles.push_back ( "\\data\\gridref.dat" );
	GTAFiles.push_back ( "\\data\\gta.dat" );
	GTAFiles.push_back ( "\\data\\gta_quick.dat" );
	GTAFiles.push_back ( "\\data\\handling.cfg" );
	GTAFiles.push_back ( "\\data\\info.zon" );
	GTAFiles.push_back ( "\\data\\main.sc" );
	GTAFiles.push_back ( "\\data\\map.zon" );
	GTAFiles.push_back ( "\\data\\melee.dat" );
	GTAFiles.push_back ( "\\data\\numplate.dat" );
	GTAFiles.push_back ( "\\data\\object.dat" );
	GTAFiles.push_back ( "\\data\\ped.dat" );
	GTAFiles.push_back ( "\\data\\pedgrp.dat" );
	GTAFiles.push_back ( "\\data\\peds.ide" );
	GTAFiles.push_back ( "\\data\\pedstats.dat" );
	GTAFiles.push_back ( "\\data\\plants.dat" );
	GTAFiles.push_back ( "\\data\\polydensity.dat" );
	GTAFiles.push_back ( "\\data\\popcycle.dat" );
	GTAFiles.push_back ( "\\data\\procobj.dat" );
	GTAFiles.push_back ( "\\data\\shopping.dat" );
	GTAFiles.push_back ( "\\data\\statdisp.dat" );
	GTAFiles.push_back ( "\\data\\surface.dat" );
	GTAFiles.push_back ( "\\data\\surfaud.dat" );
	GTAFiles.push_back ( "\\data\\surfinfo.dat" );
	GTAFiles.push_back ( "\\data\\timecyc.dat" );
	GTAFiles.push_back ( "\\data\\timecycp.dat" );
	GTAFiles.push_back ( "\\data\\txdcut.ide" );
	GTAFiles.push_back ( "\\data\\vehicles.ide" );
	GTAFiles.push_back ( "\\data\\water.dat" );
	GTAFiles.push_back ( "\\data\\water1.dat" );
	GTAFiles.push_back ( "\\data\\weapon.dat" );

	GTAFiles.push_back ( "\\data\\Decision\\BLANK.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\Cop.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\FLAT.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\GangMbr.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\GROVE.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\Indoors.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\MISSION.grp" );
	GTAFiles.push_back ( "\\data\\Decision\\MISSION.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\m_empty.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\m_infrm.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\m_norm.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\m_std.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\m_tough.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\m_weak.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\PedEvent.txt" );

	GTAFiles.push_back ( "\\data\\Decision\\Allowed\\Cop.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\Allowed\\Fireman.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\Allowed\\GangMbr.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\Allowed\\Indoors.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\Allowed\\MISSION.grp" );
	GTAFiles.push_back ( "\\data\\Decision\\Allowed\\m_empty.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\Allowed\\m_norm.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\Allowed\\m_plyr.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\Allowed\\m_steal.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\Allowed\\m_tough.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\Allowed\\m_weak.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\Allowed\\RANDOM.grp" );
	GTAFiles.push_back ( "\\data\\Decision\\Allowed\\RANDOM.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\Allowed\\RANDOM2.grp" );
	GTAFiles.push_back ( "\\data\\Decision\\Allowed\\R_Norm.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\Allowed\\R_Tough.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\Allowed\\R_Weak.ped" );

	GTAFiles.push_back ( "\\data\\Decision\\andyd\\ADgrp.grp" );
	GTAFiles.push_back ( "\\data\\Decision\\andyd\\ADtemp.ped" );

	GTAFiles.push_back ( "\\data\\Decision\\chris\\maf5.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\chris\\ryder3.ped" );

	GTAFiles.push_back ( "\\data\\Decision\\ChrisM\\CMblnk.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\ChrisM\\m_std_cm.ped" );

	GTAFiles.push_back ( "\\data\\Decision\\Craig\\crack1.ped" );

	GTAFiles.push_back ( "\\data\\Decision\\david\\dam_sec.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\david\\hei2_sc.ped" );

	GTAFiles.push_back ( "\\data\\Decision\\Imran\\sci1_is.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\Imran\\std1_is.ped" );
	GTAFiles.push_back ( "\\data\\Decision\\Imran\\std2_is.ped" );

	/*
	GTAFiles.push_back ( "\\data\\Icons\\app.ico" );
	GTAFiles.push_back ( "\\data\\Icons\\bin.ico" );
	GTAFiles.push_back ( "\\data\\Icons\\saicon.ICN" );
	GTAFiles.push_back ( "\\data\\Icons\\saicon2.ICN" );
	GTAFiles.push_back ( "\\data\\Icons\\saicon3.ICN" );
	*/

	GTAFiles.push_back ( "\\data\\maps\\Audiozon.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\cull.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\occlu.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\occluint.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\occluLA.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\occlusf.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\occluveg.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\paths.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\paths2.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\paths3.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\paths4.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\paths5.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\tunnels.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\txd.ide" );

	GTAFiles.push_back ( "\\data\\maps\\country\\countn2.ide" );
	GTAFiles.push_back ( "\\data\\maps\\country\\countn2.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\country\\countrye.ide" );
	GTAFiles.push_back ( "\\data\\maps\\country\\countrye.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\country\\countryN.ide" );
	GTAFiles.push_back ( "\\data\\maps\\country\\countryN.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\country\\countryS.ide" );
	GTAFiles.push_back ( "\\data\\maps\\country\\countryS.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\country\\countryW.ide" );
	GTAFiles.push_back ( "\\data\\maps\\country\\countryw.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\country\\counxref.ide" );

	GTAFiles.push_back ( "\\data\\maps\\generic\\barriers.ide" );
	GTAFiles.push_back ( "\\data\\maps\\generic\\dynamic.ide" );
	GTAFiles.push_back ( "\\data\\maps\\generic\\dynamic2.ide" );
	GTAFiles.push_back ( "\\data\\maps\\generic\\multiobj.ide" );
	GTAFiles.push_back ( "\\data\\maps\\generic\\procobj.ide" );
	GTAFiles.push_back ( "\\data\\maps\\generic\\vegepart.ide" );

	GTAFiles.push_back ( "\\data\\maps\\interior\\gen_int1.ide" );
	GTAFiles.push_back ( "\\data\\maps\\interior\\gen_int1.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\interior\\gen_int2.ide" );
	GTAFiles.push_back ( "\\data\\maps\\interior\\gen_int2.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\interior\\gen_int3.ide" );
	GTAFiles.push_back ( "\\data\\maps\\interior\\gen_int3.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\interior\\gen_int4.ide" );
	GTAFiles.push_back ( "\\data\\maps\\interior\\gen_int4.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\interior\\gen_int5.ide" );
	GTAFiles.push_back ( "\\data\\maps\\interior\\gen_int5.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\interior\\gen_intb.ide" );
	GTAFiles.push_back ( "\\data\\maps\\interior\\gen_intb.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\interior\\int_cont.ide" );
	GTAFiles.push_back ( "\\data\\maps\\interior\\int_cont.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\interior\\int_LA.ide" );
	GTAFiles.push_back ( "\\data\\maps\\interior\\int_LA.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\interior\\int_SF.ide" );
	GTAFiles.push_back ( "\\data\\maps\\interior\\int_SF.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\interior\\int_veg.ide" );
	GTAFiles.push_back ( "\\data\\maps\\interior\\int_veg.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\interior\\propext.ide" );
	GTAFiles.push_back ( "\\data\\maps\\interior\\props.ide" );
	GTAFiles.push_back ( "\\data\\maps\\interior\\props2.ide" );
	GTAFiles.push_back ( "\\data\\maps\\interior\\savehous.ide" );
	GTAFiles.push_back ( "\\data\\maps\\interior\\savehous.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\interior\\stadint.ide" );
	GTAFiles.push_back ( "\\data\\maps\\interior\\stadint.ipl" );

	GTAFiles.push_back ( "\\data\\maps\\LA\\LAe.ide" );
	GTAFiles.push_back ( "\\data\\maps\\LA\\LAe.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\LA\\LAe2.ide" );
	GTAFiles.push_back ( "\\data\\maps\\LA\\LAe2.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\LA\\LAhills.ide" );
	GTAFiles.push_back ( "\\data\\maps\\LA\\LAhills.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\LA\\LAn.ide" );
	GTAFiles.push_back ( "\\data\\maps\\LA\\LAn.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\LA\\LAn2.ide" );
	GTAFiles.push_back ( "\\data\\maps\\LA\\LAn2.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\LA\\LAs.ide" );
	GTAFiles.push_back ( "\\data\\maps\\LA\\LAs.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\LA\\LAs2.ide" );
	GTAFiles.push_back ( "\\data\\maps\\LA\\LAs2.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\LA\\LAw.ide" );
	GTAFiles.push_back ( "\\data\\maps\\LA\\LAw.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\LA\\LAw2.ide" );
	GTAFiles.push_back ( "\\data\\maps\\LA\\LAw2.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\LA\\LaWn.ide" );
	GTAFiles.push_back ( "\\data\\maps\\LA\\LaWn.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\LA\\LAxref.ide" );

	GTAFiles.push_back ( "\\data\\maps\\leveldes\\leveldes.ide" );
	GTAFiles.push_back ( "\\data\\maps\\leveldes\\leveldes.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\leveldes\\levelmap.ide" );
	GTAFiles.push_back ( "\\data\\maps\\leveldes\\levelmap.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\leveldes\\levelxre.ide" );
	GTAFiles.push_back ( "\\data\\maps\\leveldes\\seabed.ide" );
	GTAFiles.push_back ( "\\data\\maps\\leveldes\\seabed.ipl" );

	GTAFiles.push_back ( "\\data\\maps\\SF\\SFe.ide" );
	GTAFiles.push_back ( "\\data\\maps\\SF\\SFe.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\SF\\SFn.ide" );
	GTAFiles.push_back ( "\\data\\maps\\SF\\SFn.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\SF\\SFs.ide" );
	GTAFiles.push_back ( "\\data\\maps\\SF\\SFs.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\SF\\SFSe.ide" );
	GTAFiles.push_back ( "\\data\\maps\\SF\\SFSe.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\SF\\SFw.ide" );
	GTAFiles.push_back ( "\\data\\maps\\SF\\SFw.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\SF\\SFxref.ide" );

	GTAFiles.push_back ( "\\data\\maps\\vegas\\vegasE.ide" );
	GTAFiles.push_back ( "\\data\\maps\\vegas\\vegasE.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\vegas\\vegasN.ide" );
	GTAFiles.push_back ( "\\data\\maps\\vegas\\vegasN.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\vegas\\VegasS.ide" );
	GTAFiles.push_back ( "\\data\\maps\\vegas\\vegasS.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\vegas\\VegasW.ide" );
	GTAFiles.push_back ( "\\data\\maps\\vegas\\vegasW.ipl" );
	GTAFiles.push_back ( "\\data\\maps\\vegas\\vegaxref.ide" );
	GTAFiles.push_back ( "\\data\\maps\\vegas\\vegaxref.ipl" );

	GTAFiles.push_back ( "\\data\\maps\\veh_mods\\veh_mods.ide" );

	GTAFiles.push_back ( "\\data\\Paths\\carrec.img" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES0.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES1.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES10.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES11.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES12.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES13.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES14.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES15.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES16.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES17.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES18.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES19.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES2.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES20.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES21.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES22.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES23.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES24.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES25.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES26.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES27.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES28.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES29.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES3.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES30.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES31.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES32.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES33.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES34.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES35.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES36.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES37.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES38.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES39.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES4.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES40.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES41.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES42.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES43.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES44.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES45.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES46.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES47.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES48.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES49.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES5.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES50.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES51.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES52.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES53.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES54.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES55.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES56.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES57.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES58.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES59.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES6.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES60.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES61.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES62.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES63.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES7.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES8.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\NODES9.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\ROADBLOX.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\spath0.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\tracks.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\tracks2.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\tracks3.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\tracks4.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\train.DAT" );
	GTAFiles.push_back ( "\\data\\Paths\\train2.DAT" );

	GTAFiles.push_back ( "\\data\\script\\main.scm" );
	GTAFiles.push_back ( "\\data\\script\\SCRIPT.IMG" );

	GTAFiles.push_back ( "\\models\\cutscene.img" );
	GTAFiles.push_back ( "\\models\\effects.fxp" );
	GTAFiles.push_back ( "\\models\\effectsPC.txd" );
	GTAFiles.push_back ( "\\models\\fonts.txd" );
	GTAFiles.push_back ( "\\models\\fronten1.txd" );
	GTAFiles.push_back ( "\\models\\fronten2.txd" );
	GTAFiles.push_back ( "\\models\\fronten3.txd" );
	GTAFiles.push_back ( "\\models\\fronten_pc.txd" );
	GTAFiles.push_back ( "\\models\\gta3.img" );
	GTAFiles.push_back ( "\\models\\gta_int.img" );
	GTAFiles.push_back ( "\\models\\hud.txd" );
	GTAFiles.push_back ( "\\models\\misc.txd" );
	GTAFiles.push_back ( "\\models\\particle.txd" );
	GTAFiles.push_back ( "\\models\\pcbtns.txd" );
	GTAFiles.push_back ( "\\models\\player.img" );

	GTAFiles.push_back ( "\\models\\coll\\peds.col" );
	GTAFiles.push_back ( "\\models\\coll\\vehicles.col" );
	GTAFiles.push_back ( "\\models\\coll\\weapons.col" );

	GTAFiles.push_back ( "\\models\\generic\\air_vlo.DFF" );
	GTAFiles.push_back ( "\\models\\generic\\arrow.DFF" );
	GTAFiles.push_back ( "\\models\\generic\\hoop.dff" );
	GTAFiles.push_back ( "\\models\\generic\\vehicle.txd" );
	GTAFiles.push_back ( "\\models\\generic\\wheels.DFF" );
	GTAFiles.push_back ( "\\models\\generic\\wheels.txd" );
	GTAFiles.push_back ( "\\models\\generic\\zonecylb.DFF" );

	GTAFiles.push_back ( "\\models\\grass\\grass0_1.dff" );
	GTAFiles.push_back ( "\\models\\grass\\grass0_2.dff" );
	GTAFiles.push_back ( "\\models\\grass\\grass0_3.dff" );
	GTAFiles.push_back ( "\\models\\grass\\grass0_4.dff" );
	GTAFiles.push_back ( "\\models\\grass\\grass1_1.dff" );
	GTAFiles.push_back ( "\\models\\grass\\grass1_2.dff" );
	GTAFiles.push_back ( "\\models\\grass\\grass1_3.dff" );
	GTAFiles.push_back ( "\\models\\grass\\grass1_4.dff" );
	GTAFiles.push_back ( "\\models\\grass\\grass2_1.dff" );
	GTAFiles.push_back ( "\\models\\grass\\grass2_2.dff" );
	GTAFiles.push_back ( "\\models\\grass\\grass2_3.dff" );
	GTAFiles.push_back ( "\\models\\grass\\grass2_4.dff" );
	GTAFiles.push_back ( "\\models\\grass\\grass3_1.dff" );
	GTAFiles.push_back ( "\\models\\grass\\grass3_2.dff" );
	GTAFiles.push_back ( "\\models\\grass\\grass3_3.dff" );
	GTAFiles.push_back ( "\\models\\grass\\grass3_4.dff" );
	GTAFiles.push_back ( "\\models\\grass\\plant1.dff" );
	GTAFiles.push_back ( "\\models\\grass\\plant1.txd" );

	GTAFiles.push_back ( "\\models\\txd\\intro1.txd" );
	GTAFiles.push_back ( "\\models\\txd\\intro2.txd" );
	GTAFiles.push_back ( "\\models\\txd\\INTRO3.TXD" );
	GTAFiles.push_back ( "\\models\\txd\\intro4.txd" );
	GTAFiles.push_back ( "\\models\\txd\\LD_BEAT.txd" );
	GTAFiles.push_back ( "\\models\\txd\\LD_BUM.txd" );
	GTAFiles.push_back ( "\\models\\txd\\LD_CARD.txd" );
	GTAFiles.push_back ( "\\models\\txd\\LD_CHAT.txd" );
	GTAFiles.push_back ( "\\models\\txd\\LD_DRV.txd" );
	GTAFiles.push_back ( "\\models\\txd\\LD_DUAL.txd" );
	GTAFiles.push_back ( "\\models\\txd\\ld_grav.txd" );
	GTAFiles.push_back ( "\\models\\txd\\LD_NONE.txd" );
	GTAFiles.push_back ( "\\models\\txd\\LD_OTB.txd" );
	GTAFiles.push_back ( "\\models\\txd\\LD_OTB2.txd" );
	GTAFiles.push_back ( "\\models\\txd\\LD_PLAN.txd" );
	GTAFiles.push_back ( "\\models\\txd\\LD_POKE.txd" );
	GTAFiles.push_back ( "\\models\\txd\\LD_POOL.txd" );
	GTAFiles.push_back ( "\\models\\txd\\LD_RACE.txd" );
	GTAFiles.push_back ( "\\models\\txd\\LD_RCE1.txd" );
	GTAFiles.push_back ( "\\models\\txd\\LD_RCE2.txd" );
	GTAFiles.push_back ( "\\models\\txd\\LD_RCE3.txd" );
	GTAFiles.push_back ( "\\models\\txd\\LD_RCE4.txd" );
	GTAFiles.push_back ( "\\models\\txd\\LD_RCE5.txd" );
	GTAFiles.push_back ( "\\models\\txd\\LD_ROUL.txd" );
	GTAFiles.push_back ( "\\models\\txd\\ld_shtr.txd" );
	GTAFiles.push_back ( "\\models\\txd\\LD_SLOT.txd" );
	GTAFiles.push_back ( "\\models\\txd\\LD_SPAC.txd" );
	GTAFiles.push_back ( "\\models\\txd\\LD_TATT.txd" );
	GTAFiles.push_back ( "\\models\\txd\\load0uk.txd" );
	GTAFiles.push_back ( "\\models\\txd\\loadsc0.txd" );
	GTAFiles.push_back ( "\\models\\txd\\loadsc1.txd" );
	GTAFiles.push_back ( "\\models\\txd\\loadsc10.txd" );
	GTAFiles.push_back ( "\\models\\txd\\loadsc11.txd" );
	GTAFiles.push_back ( "\\models\\txd\\loadsc12.txd" );
	GTAFiles.push_back ( "\\models\\txd\\loadsc13.txd" );
	GTAFiles.push_back ( "\\models\\txd\\loadsc14.txd" );
	GTAFiles.push_back ( "\\models\\txd\\loadsc2.txd" );
	GTAFiles.push_back ( "\\models\\txd\\loadsc3.txd" );
	GTAFiles.push_back ( "\\models\\txd\\loadsc4.txd" );
	GTAFiles.push_back ( "\\models\\txd\\loadsc5.txd" );
	GTAFiles.push_back ( "\\models\\txd\\loadsc6.txd" );
	GTAFiles.push_back ( "\\models\\txd\\loadsc7.txd" );
	GTAFiles.push_back ( "\\models\\txd\\loadsc8.txd" );
	GTAFiles.push_back ( "\\models\\txd\\loadsc9.txd" );
	GTAFiles.push_back ( "\\models\\txd\\LOADSCS.txd" );
	GTAFiles.push_back ( "\\models\\txd\\LOADSUK.txd" );
	GTAFiles.push_back ( "\\models\\txd\\outro.txd" );
	GTAFiles.push_back ( "\\models\\txd\\splash1.txd" );
	GTAFiles.push_back ( "\\models\\txd\\splash2.txd" );
	GTAFiles.push_back ( "\\models\\txd\\splash3.txd" );

	GTAFiles.push_back ( "\\text\\american.gxt" );

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