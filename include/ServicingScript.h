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
#pragma once
#include <string>

class ServicingScript
{
public:
	ServicingScript() = default;

	void Run();

private:
	std::string GetScriptContent();
};
