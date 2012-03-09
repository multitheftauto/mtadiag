/***************************************************************************** 
* 
* PROJECT: MTADiag
* LICENSE: GNU GPL v3
* FILE: util.cpp
* PURPOSE: Utility function(s)
* DEVELOPERS: Matthew "Towncivilian" Wolfe <ligushka@gmail.com>
* 
* 
* 
* Multi Theft Auto is available from http://www.multitheftauto.com/
* 
*****************************************************************************/ 

#include "util.h"

std::string readRegKey ( std::string value, std::string subkey )
{
	HKEY hKey = 0;
	char buf[255] = {0};
	DWORD dwType = 1;
	DWORD dwBufSize = sizeof ( buf );

	if ( RegOpenKey ( HKEY_LOCAL_MACHINE, subkey.c_str(), &hKey) == ERROR_SUCCESS )
	{
		if ( RegQueryValueEx ( hKey, value.c_str(), NULL, &dwType, (BYTE*)buf, &dwBufSize) == ERROR_SUCCESS )
		{
			std::string path ( buf );
			RegCloseKey ( hKey );
			return path;
		}
		else
		{
			RegCloseKey ( hKey );
			return "Can not query key.";
		}
	}
	else
	{
		return "Failed to read key.";
	}
}