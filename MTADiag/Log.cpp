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
	logfile.open ( filePath.c_str(), std::ios::out ); // open the log file for writing
}

void Log::Close ( void )
{
	logfile.close(); // close the log file for writing
}

bool Log::WriteFileToLog ( std::string filePath, std::string itemName )
{
	std::ifstream file;
	file.open ( filePath.c_str(), std::ios::out ); // create the file

	if ( !file ) // if we can't create the file
	{
		WriteStringToLog ( "Can't access ", filePath );
		WriteStringToLog ( "" );
		return false; // failure!
	}

	// trim any trailing spaces or ">" from system command piping from the item name
	std::string garbage ( " >" );
	size_t found;

	found = itemName.find_last_not_of ( garbage );
	if ( found != std::string::npos ) { itemName.erase ( found + 1 ); }

    if ( file.peek() != EOF )
    {
	    logfile << itemName // item name
			    << ":" // colon
			    << std::endl << std::endl // linebreaks
			    << file.rdbuf() // file contents
			    << std::endl // linebreak
			    << std::flush; // clear the buffer
    }
    else
    {
	    logfile << itemName // item name
			    << ":" // colon
			    << std::endl << std::endl // linebreaks
			    << std::endl // linebreak
			    << std::flush; // clear the buffer
    }
	file.close(); // close the file

	return true; // success
}

void Log::WriteStringToLog ( std::string string, std::string string2, bool endline )
{
	logfile << string // write string1
            << " " // space
            << string2; // write string2

	if ( endline ) // add an endline if specified
		logfile << std::endl;
}

void Log::WriteDividerToLog ( void )
{
	Log::WriteStringToLog ( std::string( 120, '-' ) );
}
