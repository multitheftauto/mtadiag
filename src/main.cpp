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
#include "ServicingScript.h"

extern std::vector<std::string>      Diag::files;

int main()
{
	SetConsoleTitleA ( "MTADiag v" VERSION ); // set the console title
	std::cout << "MTADiag v" << VERSION << " by Towncivilian" << std::endl << std::endl; // tell the user what this program is

	// Run servicing auto-fix script
	try
	{
		std::cout << "Do you want MTADiag to auto-fix your installation? By entering 'y' you allow us to make system changes." << std::endl;
		if (auto c = std::getchar(); c == 'y' || c == 'Y')
		{
			std::cout << "Alright! Starting now..." << std::endl;

			ServicingScript ss;
			ss.Run();
		}
	}
	catch (std::runtime_error& ex)
	{
		std::cout << "Could not run servicing script, because: " << ex.what() << "\n";
		std::cout << "Continuing..." << std::endl;
	}

	Diag::Begin(); // begin diagnostics

	Diag::Cleanup(); // remove temporary files

	system ( "pause" ); // wait for user exit

	remove ( Diag::files[FILE_LOG].c_str() ); // remove the generated MTADiag log
};
