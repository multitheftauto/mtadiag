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

#pragma once

#include "Common.h"
#include "md5.h"

// Compatability mode registry key
#define CompatModeRegKey "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers"

std::string         ReadRegKey			               ( std::string value, std::string subkey );
bool                DeleteCompatibilityEntries         ( std::string subkey, HKEY hKeyType );
bool                CheckForFile                       ( std::string FilePath );
void                ConvertUnicodeToASCII              ( std::string file1, std::string file2 );
bool                CopyToClipboard                    ( std::string contents );
bool                IsVistaOrNewer                     ( void );
bool                IsWin8OrNewer                      ( void );
std::string         GetFileMD5                         ( std::string filename );