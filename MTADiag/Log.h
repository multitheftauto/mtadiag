/***************************************************************************** 
* 
* PROJECT: MTADiag
* LICENSE: GNU GPL v3
* FILE: Diag.h
* PURPOSE: Header for Log namespace
* DEVELOPERS: Matthew "Towncivilian" Wolfe <ligushka@gmail.com>
* 
* 
* 
* Multi Theft Auto is available from http://www.multitheftauto.com/
* 
*****************************************************************************/ 

#ifndef LOG_H
#define LOG_H

#include "Common.h"

namespace Log {

	void                    Open                        ( std::string logPath );
	void                    Close                       ( void );

	bool                    WriteFileToLog              ( std::string filePath, std::string itemName );
	void                    WriteStringToLog            ( std::string string, std::string string2 = "", bool endline = true );

	static std::ofstream logfile;
}

#endif