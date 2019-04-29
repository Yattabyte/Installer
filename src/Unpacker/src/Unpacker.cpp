#include "Log.h"
#include "nSuite.h"
#include "Resource.h"
#include <chrono>
#include <iostream>


/** Entry point. */
int main()
{
	// Tap-in to the log, have it redirect to the console
	auto index = NST::Log::AddObserver([&](const std::string & message) {
		std::cout << message;
	});

	NST::Log::PushText(
		"                       ~\r\n"
		"       Unpackager     /\r\n"
		"  ~------------------~\r\n"
		" /\r\n"
		"~\r\n\r\n"
	);

	// Acquire archive resource
	const auto start = std::chrono::system_clock::now();
	const auto dstDirectory = NST::SanitizePath(NST::GetRunningDirectory());
	size_t fileCount(0ull), byteCount(0ull);
	NST::Resource archive(IDR_ARCHIVE, "ARCHIVE");
	if (!archive.exists()) 
		NST::Log::PushText("Cannot access archive resource (may be absent, corrupt, or have different identifiers), aborting...\r\n");
	else {
		// Parse the header
		std::string packageName("");
		char * packagedData(nullptr);
		size_t packagedDataSize(0ull);
		bool result = NST::ParseHeader(reinterpret_cast<char*>(archive.getPtr()), archive.getSize(), packageName, &packagedData, packagedDataSize);
		if (!result) 
			NST::Log::PushText("Critical failure: cannot parse archive package's header.\r\n");		
		else {
			// Report where we're unpacking to
			NST::Log::PushText(
				"Unpacking to the following directory:\r\n"
				"\t> " + dstDirectory + "\\" + packageName +
				"\r\n"
			);

			// Unpackage using the resource file
			if (!NST::DecompressDirectory(dstDirectory, reinterpret_cast<char*>(archive.getPtr()), archive.getSize(), &byteCount, &fileCount))
				NST::Log::PushText("Cannot decompress embedded package resource, aborting...\r\n");
			else {
				// Success, report results
				const auto end = std::chrono::system_clock::now();
				const std::chrono::duration<double> elapsed_seconds = end - start;
				NST::Log::PushText(
					"Files written:  " + std::to_string(fileCount) + "\r\n" +
					"Bytes written:  " + std::to_string(byteCount) + "\r\n" +
					"Total duration: " + std::to_string(elapsed_seconds.count()) + " seconds\r\n\r\n"
				);

				// Pause and exit
				NST::Log::RemoveObserver(index);
				system("pause");
				return EXIT_SUCCESS;
			}
		}
	}
	// Pause and exit
	NST::Log::RemoveObserver(index);
	system("pause");
	return EXIT_FAILURE;
}