/***************************************************************************** 
* 
* PROJECT: MTADiag
* LICENSE: GNU GPL v3
* FILE: util.cpp
* PURPOSE: Utility functions
* DEVELOPERS: Matthew "Towncivilian" Wolfe <ligushka@gmail.com>
* 
* 
* 
* Multi Theft Auto is available from http://www.multitheftauto.com/
* 
*****************************************************************************/ 

#include "util.h"

std::string ReadRegKey ( std::string value, std::string subkey )
{
	HKEY hKey = 0; // handle to registry key
	char buf[255] = {0}; // buffer for reading
	DWORD dwType = 1; // REG_SZ
	DWORD dwBufSize = sizeof ( buf ); // buffer size

	if ( RegOpenKeyEx ( HKEY_LOCAL_MACHINE, subkey.c_str(), NULL, KEY_READ, &hKey ) == ERROR_SUCCESS ) // if registry key read was successfully
	{
		if ( RegQueryValueEx ( hKey, value.c_str(), NULL, &dwType, ( BYTE* ) buf, &dwBufSize ) == ERROR_SUCCESS ) // if registry value read was successfully
		{
			std::string value ( buf ); // store the value
			RegCloseKey ( hKey ); // close the registry key
			return value; // return the value
		}
		else
		{
			RegCloseKey ( hKey ); // close the registry key
			return "Can not query value."; // unable to query value
		}
	}
	else
	{
		return "Failed to read key."; // unable to read registry key
	}
}

bool DeleteCompatibilityEntries ( std::string subkey, HKEY hKeyType )
{
	HKEY hKey = 0; // handle to registry key
	char buf[255] = {0}; // buffer for reading registry value
	char buf2[255] = {0}; // buffer for reading value data
	DWORD dwType = 1; // REG_SZ
	DWORD dwBufSize = sizeof ( buf ); // registry value buffer size
	DWORD dwBuf2Size = sizeof ( buf2 ); // value data buffer size

	DWORD index = 0; // index for RegEnumKeyEx
	long result = 0; // result of RegEnumKeyEx

	bool changed = false;

	if ( RegOpenKeyEx ( hKeyType, subkey.c_str(), NULL, KEY_READ | KEY_WRITE, &hKey ) == ERROR_SUCCESS ) // if registry key read was successfully
	{
		while ( result != ERROR_NO_MORE_ITEMS ) // loop until we run out of registry values to read
		{
			result = RegEnumValue ( hKey, index, buf, &dwBufSize, 0, NULL, NULL, NULL ); // attempt to enumerate values and store result
			dwBufSize = sizeof ( buf ); // set the buffer size to the read value's size

			if ( result == ERROR_SUCCESS ) // if we read something
			{
				index++; // increment index
				if ( strstr ( buf, "gta_sa.exe" ) || strstr ( buf, "Multi Theft Auto.exe" ) ) // check for filename matches
				{
					if ( RegQueryValueEx ( hKey, buf, NULL, &dwType, ( BYTE* ) buf2, &dwBuf2Size ) == ERROR_SUCCESS ) // read the value's data
					{
						if ( strcmp ( buf2, "~ RUNASADMIN" ) == 0 || strcmp ( buf2, "RUNASADMIN" ) == 0 ) // is the value's data already just "RUNASADMIN"?
						{
							continue; // continue since we don't need to do anything
						}

						if ( strstr ( buf2, "RUNASADMIN" ) ) // does it contain "RUNASADMIN" along with something else?
						{
							if ( IsWin8OrNewer() ) // is the user running Windows 8 or newer?
							{
								char Win8Data[13] = "~ RUNASADMIN"; // set the data buffer to the proper string
								RegSetValueEx ( hKey, buf, 0, dwType, ( BYTE* ) Win8Data, sizeof (Win8Data) ); // set the value data to RUNASADMIN only
								changed = true;
								index--;
								continue;
							}
							else // 7 or older
							{
								char XPData[11] = "RUNASADMIN"; // set the data buffer to the proper string
								RegSetValueEx ( hKey, buf, 0, dwType, ( BYTE* ) XPData, sizeof (XPData) ); // set the value data to RUNASADMIN only
								changed = true;
								index--;
								continue;
							}
						}

						else // the user only has other compatibility mode settings enabled
						{
							RegDeleteValue ( hKey, buf ); // delete the registry value
							changed = true;
							index--;
							continue;
						}
					}
				}
			}
		}
		RegCloseKey ( hKey ); // close the registry key
		return changed;
	}
	else
	{
		return false; // unable to read registry key
	}
}

bool CheckForFile ( std::string FilePath )
{
	std::ifstream ifile ( FilePath.c_str() );
	if ( ifile )
		return true;
	else
		return false;
}

void ConvertUnicodeToASCII ( std::string file1, std::string file2 )
{
	std::stringstream ss; // create a stringstream
	std::string convert;

	ss << "TYPE " << file1 << " > " << file2; // TYPE <file1> > <file2>
	convert = ss.str ();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	system ( convert.c_str() ); // do it
}

bool CopyToClipboard ( std::string contents )
{
	HGLOBAL hGlobal = GlobalAlloc ( GMEM_MOVEABLE, contents.length() + 1 ); // get a handle to store our data in the clipboard

	if ( hGlobal ) // if handle was allocated successfully
	{
		char *szData = ( char * ) GlobalLock ( hGlobal ); // allocate a handle for the data

		strcpy_s ( szData, contents.length() + 1, contents.c_str() ); // copy data to clipboard

		GlobalUnlock ( hGlobal ); // free the handle

		OpenClipboard ( NULL ); // open the clipboard

		EmptyClipboard(); // empty any previous contents

		SetClipboardData ( CF_TEXT, hGlobal ); // set the clipboard data to the Pastebin URL

		CloseClipboard(); // close the clipboard

		return true; // success
	}
	else
		return false; // failure
}

// slightly modified version of http://msdn.microsoft.com/en-us/library/ms724451%28VS.85%29.aspx
bool IsVistaOrNewer ( void )
{
	OSVERSIONINFO osvi;
	bool bIsVistaOrNewer;

	ZeroMemory ( &osvi, sizeof ( OSVERSIONINFO ) );
	osvi.dwOSVersionInfoSize = sizeof ( OSVERSIONINFO );

	GetVersionEx ( &osvi );

	if ( ( bIsVistaOrNewer = ( osvi.dwMajorVersion >= 6 ) ) != 0 )
		return true;
	else
		return false;
}

// slightly modified version of http://msdn.microsoft.com/en-us/library/ms724451%28VS.85%29.aspx
bool IsWin7 ( void )
{
	OSVERSIONINFO osvi;
	bool bIsWin7OrNewer;

	ZeroMemory ( &osvi, sizeof ( OSVERSIONINFO ) );
	osvi.dwOSVersionInfoSize = sizeof ( OSVERSIONINFO );

	GetVersionEx ( &osvi );

	if ( ( bIsWin7OrNewer = ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1 ) ) != 0 )
		return true;
	else
		return false;
}

// slightly modified version of http://msdn.microsoft.com/en-us/library/ms724451%28VS.85%29.aspx
bool IsWin8OrNewer ( void )
{
	OSVERSIONINFO osvi;
	bool bIsWin8OrNewer;

	ZeroMemory ( &osvi, sizeof ( OSVERSIONINFO ) );
	osvi.dwOSVersionInfoSize = sizeof ( OSVERSIONINFO );

	GetVersionEx ( &osvi );

	if ( ( bIsWin8OrNewer = ( osvi.dwMajorVersion >= 6 && osvi.dwMinorVersion >= 2 ) ) != 0 )
		return true;
	else
		return false;
}

std::string GetFileMD5 ( std::string filename )
{
	FILE *fp; // file pointer

	fopen_s ( &fp, filename.c_str(), "rb" ); // try to open the file

	if ( fp == NULL ) // we can't open it
	{
		return ( "Unable to open " + filename + " for MD5 checksum." );
	}

	MD5 md5; // initialize MD5

	std::string md5sum; // string to store md5sum

	unsigned char buffer[4096]; // file buffer

	// read all bytes throughout the file
	while ( !feof ( fp ) )
	{
		unsigned int read = fread ( buffer, 1, 4096, fp );

		// update the MD5 with what we just read
		md5.update ( buffer, read );
	}

	md5.finalize(); // create a digest from the MD5 result

	fclose ( fp ); // close the file

	std::stringstream ss; // create a stringstream

	ss << "MD5sum for " << filename << ": " << md5;
	md5sum = ss.str();

	// clear the stringstream
	ss.str ("");
	ss.clear();

	return md5sum;
}