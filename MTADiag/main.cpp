/***************************************************************************** 
* 
* PROJECT: MTADiag
* LICENSE: GNU GPL v3
* FILE: main.cpp
* PURPOSE: MTA diagnostic tool
* DEVELOPERS: Matthew "Towncivilian" Wolfe <ligushka@gmail.com>
* 
* 
* 
* Multi Theft Auto is available from http://www.multitheftauto.com/
* 
*****************************************************************************/ 

#include "Common.h"
#include "Diag.h"

int main()
{
	SetConsoleTitle ( "MTADiag v" VERSION );
	std::cout << "MTADiag v" << VERSION << " by Towncivilian" << std::endl << std::endl;

	Diag::Init(); // begin diagnostics

	Diag::Destroy(); // remove temporary files (dxdiag.log, MTANightly.exe, tasklist.txt)

	system ( "pause" ); // wait for user exit

	remove ( Diag::diagLogPath.c_str() ); // remove the generated MTADiag log
};