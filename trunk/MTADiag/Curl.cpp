/***************************************************************************** 
* 
* PROJECT: MTADiag
* LICENSE: GNU GPL v3
* FILE: Curl.cpp
* PURPOSE: cURL functions
* DEVELOPERS: Matthew "Towncivilian" Wolfe <ligushka@gmail.com>
* 
* 
* 
* Multi Theft Auto is available from http://www.multitheftauto.com/
* 
*****************************************************************************/ 

#include "Curl.h"
#include "curl/curl.h"
#include "curl/easy.h"
#include "curl/types.h"

void progress_callback ( void* percent, double TotalToDL, double CurrentDL, double TotalToUL, double CurrentUL ); // percentage progress callback
size_t write_data ( void *ptr, size_t size, size_t nmemb, void *stream ); // writes HTTP response to a string

bool Curl::DownloadFile ( std::string fileURL, std::string filePath )
{
	CURL *curl; // initialize curl
	CURLcode res; // variable to store curl result
	curl = curl_easy_init(); // initialize curl_easy
	if ( curl ) // if curl was initialized
	{
		FILE *fp; // file pointer
		fopen_s ( &fp, filePath.c_str(), "wb" ); // open the temporary file download location
		curl_easy_setopt ( curl, CURLOPT_URL, fileURL.c_str() ); // set the URL
		curl_easy_setopt ( curl, CURLOPT_WRITEFUNCTION, fwrite ); // set the write function
		curl_easy_setopt ( curl, CURLOPT_WRITEDATA, fp ); // set the file path
		curl_easy_setopt ( curl, CURLOPT_NOPROGRESS, FALSE ); // we want progress
		curl_easy_setopt ( curl, CURLOPT_PROGRESSFUNCTION, progress_callback ); // set the progress callback function
		res = curl_easy_perform ( curl ); // perform; store result into res
		curl_easy_cleanup ( curl ); // clean up
		fclose ( fp ); // close the file
	}
	if ( !res ) // if we were successful
		return true;
	else // failure
		return false;
}

std::string Curl::CreatePasteBin ( std::string filePath, std::string pasteName )
{
	CURL *curl; // initialize curl
	CURLcode res; // variable to store curl result
	curl = curl_easy_init(); // initialize curl_easy

	std::string logText; // stores entire log file
	std::string post; // stores POST string
	std::stringstream ss; // create a stringstream
	std::ifstream file;

	// read entire MTADiag log into string
	file.open ( filePath.c_str(), std::ios::in );
		if ( file )
		{
			ss << file.rdbuf();
			logText = ss.str();
		}
	file.close();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	ss	<< "api_option=paste&" // paste
		<< "api_user_key=&" // paste as guest
		<< "api_paste_private=1&" // paste as unlisted
		<< "api_paste_name="
		<< curl_easy_escape ( curl, pasteName.c_str(), pasteName.length() ) // urlencode MTADiag log filename
		<< "&"
		<< "api_paste_expire_date=1M&" // paste will expire in one month
		<< "api_dev_key=use_your_own&" // Pastebin API dev key
		<< "api_paste_code="
		<< curl_easy_escape ( curl, logText.c_str(), logText.length() ); // urlencode log file contents
	post = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	if ( curl ) // if curl was initialized
	{
		curl_easy_setopt ( curl, CURLOPT_URL, "http://pastebin.com/api/api_post.php" ); // set the URL
		curl_easy_setopt ( curl, CURLOPT_POSTFIELDS, post.c_str() ); // set our log file as the POST field
		curl_easy_setopt ( curl, CURLOPT_NOPROGRESS, FALSE ); // we want progress
		curl_easy_setopt ( curl, CURLOPT_PROGRESSFUNCTION, progress_callback ); // set the progress callback function
		curl_easy_setopt ( curl, CURLOPT_WRITEFUNCTION, write_data ); // set the write function
		res = curl_easy_perform ( curl ); // perform; store result into res
		curl_easy_cleanup ( curl ); // clean up
	}
	if ( !res ) // if we were successful
	{

		return response;
	}
	else // failure
	{
		return "Failed to upload to Pastebin.";
	}
}

void progress_callback ( void*, double TotalToDL, double CurrentDL, double TotalToUL, double CurrentUL )
{
	if ( TotalToDL > 0 ) { printf ( "Downloaded: %3.0f%%\r", CurrentDL/TotalToDL * 100 ); }
	else { printf ( "Uploaded: %3.0f%%\r", CurrentUL/TotalToUL * 100 ); }
	fflush ( stdout );
}

size_t write_data ( void* ptr, size_t size, size_t nmemb, void* )
{
	std::string temp ( static_cast <const char*> ( ptr ), size* nmemb );
    Curl::response.append ( temp );
    return size* nmemb;
}