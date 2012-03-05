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

string readRegKey(string value, string subkey)
{
	HKEY hKey = 0;
	char buf[255] = {0};
	DWORD dwType = 0;
	DWORD dwBufSize = sizeof(buf);
	char *cValue;
	char *cSubkey;
	cValue = new char[strlen(value.c_str())+1];
	cSubkey = new char[strlen(subkey.c_str())+1];

	strcpy (cValue, value.c_str());
	strcpy (cSubkey, subkey.c_str());

	if ( RegOpenKey(HKEY_LOCAL_MACHINE, cSubkey, &hKey) == ERROR_SUCCESS)
	{
		dwType = REG_SZ;
		if ( RegQueryValueEx(hKey, cValue, 0, &dwType, (BYTE*)buf, &dwBufSize) == ERROR_SUCCESS)
		{
			string path(buf);
			delete [] cValue;
			delete [] cSubkey;
			return path;
		}
		else
		{
			RegCloseKey(hKey);
			return "Can not query key.";
		}
	}
	else
	{
		delete [] cValue;
		delete [] cSubkey;
		return "Failed to read key.";
	}
}