#include "Common.h"
#include <map>

// Command inclusions
#include "Commands/Command.h"
#include "Commands/InstallerCommand.h"
#include "Commands/DiffCommand.h"
#include "Commands/PatchCommand.h"


/** Entry point. */
int main(int argc, char *argv[])
{
	// Load all commands into a command map
	const auto start = std::chrono::system_clock::now();
	struct compare_string { bool operator()(const char * a, const char * b) const { return strcmp(a, b) < 0; } };
	const std::map<const char *, Command*, compare_string> commandMap{ 
		{"-installer", new InstallerCommand()},
		{"-diff", new DiffCommand()},
		{"-patch", new PatchCommand()}
	};

	// Check for valid arguments
	if (argc <= 1 || commandMap.find(argv[1]) == commandMap.end())
		exit_program("\n"
			"        Help:       /\n"
			" ~-----------------~\n"
			"/\n"
			" Operations Supported:\n"
			" -cd (To compress and package an entire directory)\n"
			" -dd (To diff an entire directory)\n"
			" -pd (To patch an entire directory)\n"
			"\n\n"
		);
	
	// Command exists in command map, execute it
	commandMap.at(argv[1])->execute(argc, argv);
	const auto end = std::chrono::system_clock::now();
	const std::chrono::duration<double> elapsed_seconds = end - start;
	std::cout << "Total duration: " << elapsed_seconds.count() << " seconds\n\n";
	system("pause");

	exit(EXIT_SUCCESS);
}