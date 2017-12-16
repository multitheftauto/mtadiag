/*****************************************************************************
*
* PROJECT: MTADiag
* LICENSE: GNU GPL v3
* FILE: ServicingScript.h
* PURPOSE: Servicing script launcher
*
* Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/
#include "ServicingScript.h"
#include "resource.h"

#include <Windows.h>
#include <stdexcept>

using namespace std::string_literals;

void ServicingScript::Run()
{
	// Load script content
	std::string content = GetScriptContent();

	// Write to temporary file so we can execute it
	WCHAR tempPath[MAX_PATH];
	if (!GetTempPathW(MAX_PATH, tempPath))
		throw std::runtime_error("Could not get temp path");

	if (!GetTempFileNameW(tempPath, L"mtadiag", 0, tempPath))
		throw std::runtime_error("Could not get temp file path");

	// Write and close file
	std::wstring tempPathStr = std::wstring{ tempPath } + L".bat"s;
	FILE* fileHandle = _wfopen(tempPathStr.c_str(), L"wb");
	if (!fileHandle)
		throw std::runtime_error("Could not open temporary file");

	fputs(content.c_str(), fileHandle);
	fclose(fileHandle);

	// Finally, execute script
	_wsystem((L"cmd /C "s + tempPathStr).c_str());
}

std::string ServicingScript::GetScriptContent()
{
	HMODULE handle = GetModuleHandle(NULL);
	if (!handle)
		throw std::runtime_error("Could not retrieve module handle");

	HRSRC resource = FindResource(handle, MAKEINTRESOURCE(IDR_SERVICING_SCRIPT), MAKEINTRESOURCE(TEXTFILE));
	if (!resource)
		throw std::runtime_error("Could not retrieve embedded resource");

	HGLOBAL rcData = LoadResource(handle, resource);
	if (!rcData)
		throw std::runtime_error("Could not load resource");

	DWORD size = SizeofResource(handle, resource);
	auto data = static_cast<const char*>(LockResource(rcData)); // it's not necessary to unlock according to msdn

	return std::string(data, size);
}
