#include "DirectoryState.h"
#include "WelcomeState.h"
#include "InstallState.h"
#include "../Installer.h"
#include <filesystem>


DirectoryState::DirectoryState(Installer * installer)
	: State(installer) {}

void DirectoryState::enact()
{
	m_installer->showFrame(Installer::FrameEnums::DIRECTORY_FRAME);
	m_installer->showButtons(true, true, true);
}

void DirectoryState::pressPrevious()
{
	m_installer->setState(new WelcomeState(m_installer));
}

void DirectoryState::pressNext()
{
	auto directory = m_installer->getDirectory();
	
	if (directory == "" || directory == " " || directory.length() < 3 || !std::filesystem::is_directory(directory))
		MessageBox(
			NULL,
			"Please enter a valid directory before proceeding.",
			"Invalid path!",
			MB_OK | MB_ICONERROR | MB_TASKMODAL
		);
	else
		m_installer->setState(new InstallState(m_installer));
}

void DirectoryState::pressClose()
{
	// No new screen
	PostQuitMessage(0);
}