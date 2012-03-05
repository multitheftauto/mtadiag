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
#include "curl/curl.h"
#include "curl/types.h"
#include "curl/easy.h"

bool            downloadFile            (char *fileURL, string filePath);
void            progress_func           (void* ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded);
size_t          write_data              (void *ptr, size_t size, size_t nmemb, FILE *stream);

#endif