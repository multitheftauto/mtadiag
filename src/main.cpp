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
* Multi Theft Auto is available from https://www.mtasa.com/
* 
*****************************************************************************/ 

#include "Common.h"
#include "Diag.h"
extern std::vector<std::string>      Diag::files;

int main()
{
	SetConsoleTitleA ( "MTADiag v" VERSION ); // set the console title
	std::cout << "MTADiag v" << VERSION << " by Towncivilian" << std::endl << std::endl; // tell the user what this program is

	Diag::Begin(); // begin diagnostics

	Diag::Cleanup(); // remove temporary files

	system ( "pause" ); // wait for user exit

	remove ( Diag::files[FILE_LOG].c_str() ); // remove the generated MTADiag log
};
