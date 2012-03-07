/***************************************************************************** 
* 
* PROJECT: MTADiag
* LICENSE: GNU GPL v3
* FILE: Curl.h
* PURPOSE: Header for cURL functions
* DEVELOPERS: Matthew "Towncivilian" Wolfe <ligushka@gmail.com>
* 
* 
* 
* Multi Theft Auto is available from http://www.multitheftauto.com/
* 
*****************************************************************************/ 

#ifndef CCURL_H
#define CCURL_H
#include "Common.h"

bool            downloadFile            ( std::string fileURL, std::string filePath );

#endif