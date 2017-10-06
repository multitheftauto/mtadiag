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

#pragma once

#include "Common.h"

#define CUR_MTA_VERSIONS 5 // beginning at 1; encompasses 1.1, 1.2, 1.3, 1.4, 1.5

// MTA install path
#define MTAPathValue	"Last Install Location"
#define MTA11PathSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.1"
#define MTA12PathSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.2"
#define MTA13PathSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.3"
#define MTA14PathSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.4"
#define MTA15PathSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.5"

// GTA:SA path
#define MTAGTAPathValue  "GTA:SA Path"
#define MTAGTAPathSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\Common"

// Long MTA version string
#define MTAVerValue    "mta-version-ext"
#define MTA11VerSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.1\\Settings\\general"
#define MTA12VerSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.2\\Settings\\general"
#define MTA13VerSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.3\\Settings\\general"
#define MTA14VerSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.4\\Settings\\general"
#define MTA15VerSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.5\\Settings\\general"

// Nightly download URLs
#define MTA11NightlyURL "https://nightly.mtasa.com/?mtasa-1.1.1-rc-latest"
#define MTA12NightlyURL "https://nightly.mtasa.com/?mtasa-1.2-rc-latest"
#define MTA13NightlyURL "https://nightly.mtasa.com/?mtasa-1.3.5-rc-latest"
#define MTA14NightlyURL "https://nightly.mtasa.com/?mtasa-1.4.1-rc-latest"
#define MTA15NightlyURL "https://nightly.mtasa.com/?mtasa-1.5-rc-latest"

#define MTAVerURL "https://nightly.mtasa.com/ver/"

enum
{
    FILE_LOG,
    FILE_TEMP,
    FILE_NIGHTLY_INSTALLER,
    FILE_WMIC_UNI,
    FILE_D3DX9_DLL,
};

enum
{
    OUTPUT_NONE,
    OUTPUT_ANSI,
    OUTPUT_UNICODE,
};

namespace Diag {

	void                    Begin                       ( void );
	void                    Cleanup                     ( bool deleteLog = false );

	// gather all currently installed MTA:SA versions and ask the user to pick between them if necessary
	void                    GeneratePaths               ( void );
	void                    GenerateGTAFiles            ( void );
	bool                    PollMTAVersions             ( void );
	void                    UserPickVersion             ( void );

	// gather MTA's path and version, and GTA:SA's path
	std::string             GetMTAPath                  ( void );
	std::string             GetGamePath                 ( void );
	std::string             GetMTAVersion               ( void );

	// update MTA:SA to the latest nightly/unstable build
	// and update DirectX if necessary
	void                    UpdateDirectX               ( void );
	void                    UpdateMTA                   ( void );

	// information gathering functions
	DWORD                   DoSystemCommandWithOutput   ( std::string command, int outputType = OUTPUT_ANSI, DWORD maxTimeMs = 20000 );
	void                    GetDir                      ( std::string directory );
	void                    QueryWMIC                   ( std::string, std::string = "", std::string = "", std::string = "" );

	extern std::vector<std::string> files;
}
