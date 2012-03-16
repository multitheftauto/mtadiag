/***************************************************************************** 
* 
* PROJECT: MTADiag
* LICENSE: GNU GPL v3
* FILE: util.h
* PURPOSE: Header file for utility functions
* DEVELOPERS: Matthew "Towncivilian" Wolfe <ligushka@gmail.com>
* 
* 
* 
* Multi Theft Auto is available from http://www.multitheftauto.com/
* 
*****************************************************************************/ 

#ifndef UTIL_H
#define UTIL_H
#include "Common.h"

std::string			readRegKey			  ( std::string value, std::string subkey );
bool                CheckForFile          ( std::string FilePath );
void                ConvertUnicodeToASCII ( std::string file1, std::string file2 );

#endif