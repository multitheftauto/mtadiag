/***************************************************************************** 
* 
* PROJECT: MTADiag
* LICENSE: GNU GPL v3
* FILE: Diag.h
* PURPOSE: Logging namespace
* DEVELOPERS: Matthew "Towncivilian" Wolfe <ligushka@gmail.com>
* 
* 
* 
* Multi Theft Auto is available from http://www.multitheftauto.com/
* 
*****************************************************************************/ 

#include "Log.h"

void Log::Open ( std::string filePath )
{
	logfile.open ( filePath.c_str(), std::ios::out );
}

void Log::Close ( void )
{
	logfile.close();
}

bool Log::WriteFileToLog ( std::string filePath, std::string itemName )
{
	std::ifstream file;
	file.open ( filePath.c_str(), std::ios::out );

	if ( !file )
	{
		WriteStringToLog ( "Can't open", filePath );
		WriteStringToLog ( "" );
		return false;
	}

	// trim any trailing spaces or ">" from system command piping from the item name
	std::string garbage ( " >" );
	size_t found;

	found = itemName.find_last_not_of ( garbage );
	if ( found != std::string::npos ) { itemName.erase ( found + 1 ); }

	logfile << itemName
			<< ":"
			<< std::endl << std::endl
			<< file.rdbuf()
			<< std::endl
			<< std::flush;

	file.close();

	return true;
}

void Log::WriteStringToLog ( std::string string, std::string string2, bool endline )
{
	logfile << string  
            << " "
            << string2;

	if ( endline )
		logfile << std::endl;
}