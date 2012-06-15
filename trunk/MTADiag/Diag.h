/***************************************************************************** 
* 
* PROJECT: MTADiag
* LICENSE: GNU GPL v3
* FILE: Diag.h
* PURPOSE: Header for Diag namespace
* DEVELOPERS: Matthew "Towncivilian" Wolfe <ligushka@gmail.com>
* 
* 
* 
* Multi Theft Auto is available from http://www.multitheftauto.com/
* 
*****************************************************************************/ 

#ifndef DIAG_H
#define DIAG_H

#include "Common.h"

#define CUR_MTA_VERSIONS 6 // beginning at 1; encompasses 1.1, 1.2, 1.3, 1.4, 1.5

#define MTAPathValue	"Last Install Location"
#define MTA11PathSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.1"
#define MTA12PathSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.2"
#define MTA13PathSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.3"
#define MTA14PathSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.4"
#define MTA15PathSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.5"

#define MTAGTAPathValue  "GTA:SA Path"
#define MTAGTAPathSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\Common"

#define MTAVerValue    "mta-version-ext"
#define MTA11VerSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.1\\Settings\\general"
#define MTA12VerSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.2\\Settings\\general"
#define MTA13VerSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.3\\Settings\\general"
#define MTA14VerSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.4\\Settings\\general"
#define MTA15VerSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.5\\Settings\\general"

#define MTA11NightlyURL "http://nightly.mtasa.com/?mtasa-1.1.1-rc-latest"
#define MTA12NightlyURL "http://nightly.mtasa.com/?mtasa-1.2-rc-latest"
#define MTA13NightlyURL "http://nightly.mtasa.com/?mtasa-1.3-rc-latest"
#define MTA14NightlyURL "http://nightly.mtasa.com/?mtasa-1.4-unstable-latest"
#define MTA15NightlyURL "http://nightly.mtasa.com/?mtasa-1.5-unstable-latest"

#define CompatModeRegKey1 "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers"
#define CompatModeRegKey2 "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers"

namespace Diag {

	void                    Begin                       ( void );
	void                    Cleanup                     ( void );

	// gather all currently installed MTA:SA versions and ask the user to pick between them if necessary
	void                    GeneratePaths               ( void );
	bool                    PollMTAVersions             ( void );
	void                    UserPickVersion             ( void );

	// gather MTA's path and version, and GTA:SA's path
	static std::string      GetMTAPath                  ( void );
	static std::string      GetGamePath                 ( void );
	static std::string      GetMTAVersion               ( void );

	// update MTA:SA to the latest nightly/unstable build
	// and update DirectX if necessary
	void                    UpdateDirectX               ( void );
	void                    UpdateMTA                   ( void );

	void                    DoSystemCommandWithOutput   ( std::string command );
	void                    GetDir                      ( std::string directory );
	void                    ExportRegKeyToFile          ( std::string subkey );
	void                    TrimCompatabilityExport     ( void );
	void                    QueryWMIC                   ( std::string, std::string = "", std::string = "", std::string = "" );

	// used for storing environment variables, current system time, files, and some paths
	static std::string      logFileName;
	static std::string      tempDir;
	static std::string      systemRoot;
	static SYSTEMTIME       sysTime;

	extern std::vector<std::string> files;

	static std::string      MTAPath;
	static std::string      GTAPath;

	// store current MTA version when GetMTAVersion() is called, and store the original version to dump in the log file
	static std::string      MTAVersion;
	static std::string      OriginalMTAVersion;

	static std::string      MTAVersionsInstalled[CUR_MTA_VERSIONS]; // array to store paths of all MTA versions currently installed
	static int              MTAVerChoice;                           // stores user's choice of which MTA version to diagnose

	static std::string      PasteBinResult;

	static bool             DXUpdated;                              // was DirectX updated by MTADiag?
}

#endif