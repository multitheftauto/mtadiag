/***************************************************************************** 
* 
* PROJECT: MTADiag
* LICENSE: GNU GPL v3
* FILE: Curl.cpp
* PURPOSE: cURL functions to download a file
* DEVELOPERS: Matthew "Towncivilian" Wolfe <ligushka@gmail.com>
* 
* 
* 
* Multi Theft Auto is available from http://www.multitheftauto.com/
* 
*****************************************************************************/ 

#include "Curl.h"
#include "curl/curl.h"
#include "curl/types.h"
#include "curl/easy.h"
void progress_callback ( void* percent, double TotalToDL, double CurrentDL, double TotalToUL, double CurrentUL );

bool downloadFile ( std::string fileURL, std::string filePath )
{
	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	if ( curl )
	{
		FILE *fp;
		fopen_s ( &fp, filePath.c_str(), "wb" );
		curl_easy_setopt ( curl, CURLOPT_URL, fileURL.c_str() );
		curl_easy_setopt ( curl, CURLOPT_WRITEFUNCTION, fwrite );
		curl_easy_setopt ( curl, CURLOPT_WRITEDATA, fp );
		curl_easy_setopt ( curl, CURLOPT_NOPROGRESS, FALSE );
		curl_easy_setopt ( curl, CURLOPT_PROGRESSFUNCTION, progress_callback );
		res = curl_easy_perform ( curl );
		curl_easy_cleanup ( curl );
		fclose ( fp );
	}
	if ( !res )
		return true;
	else
		return false;
}

void progress_callback ( void* percent, double TotalToDL, double CurrentDL, double TotalToUL, double CurrentUL )
{
	printf ( "Downloaded: %3.0f%%\r", CurrentDL/TotalToDL * 100 );
	fflush ( stdout );
}