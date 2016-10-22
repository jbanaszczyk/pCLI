/******************************************************************************
 *
 * Copyright 2013 Jacek.Banaszczyk@gmail.com
 * Part of pCli project: https://github.com/jbanaszczyk/pCli
 *
 *****************************************************************************/

#include "stdafx.h"

#include "pApps.h"
#include "../common/Environment.h"
#include "../common/SysPidInfo.h"
#include "../common/IniFile.h"
#include "../common/YesNoOption.h"

#include <iostream>
#include <stdio.h>
#include <conio.h>
#include <array>

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>

static const boost::filesystem::path PORTABLE_APPS_APP_LE_32 = pApps::PORTABLE_APPS_APP / _T("TCCLE32");
static const boost::filesystem::path PORTABLE_APPS_APP_LE_64 = pApps::PORTABLE_APPS_APP / _T("TCCLE64");
static const boost::filesystem::path PORTABLE_APPS_APP_FULL_32 = pApps::PORTABLE_APPS_APP / _T("TCMD32");
static const boost::filesystem::path PORTABLE_APPS_APP_FULL_64 = pApps::PORTABLE_APPS_APP / _T("TCMD64");

const boost::filesystem::path TCC_EXE_LE_32 = _T("tcc.exe");
const boost::filesystem::path TCC_EXE_LE_64 = _T("tcc.exe");
const boost::filesystem::path TCC_EXE_FULL_32 = _T("tcc.exe");
const boost::filesystem::path TCC_EXE_FULL_64 = _T("tcc.exe");

static const std::tstring _SECTION_COMMAND = _T("Command");
static const std::tstring _SECTION_ENVIRONMENT = _T("Environment");
static const std::tstring _SECTION_LAUNCHER = _T("TccLePortable");
static const std::tstring _SECTION_PROFILE = _T("Profile");
static const std::tstring _SECTION_STARTUP = _T("StartUp");

static const std::tstring _SECTION_4NT = _T("4NT");
static const std::tstring _SECTION_4NT_BAK = _T("4NT.bak");

static const std::tstring _ENV_BACKUP_PREFIX = _T("$TCC$");
static const std::tstring _ENV_DIRDRIVES_FILE = _T("$TCC$DirDrivesFile");
static const std::tstring _ENV_DIRHIST_FILE = _T("$TCC$DirHistFile");
static const std::tstring _ENV_HIST_FILE = _T("$TCC$HistFile");
static const std::tstring _ENV_TEMP = _T("TEMP");
static const std::tstring _ENV_TMP = _T("TMP");

static const std::tstring _INI_4NT_NAME_LOCAL_HISTORY = _T("LocalHistory");
static const std::tstring _INI_4NT_NAME_LOCALDIR_HISTORY = _T("LocalDirHistory");
static const std::tstring _INI_4NT_NAME_TC_START_PATH = _T("TCStartPath");
static const std::tstring _INI_4NT_NAME_4START_PATH = _T("4StartPath");
static const std::tstring _INI_4NT_NAME_TREE_PATH = _T("TreePath");

static const std::tstring _INI_NAME_TEMP_DIRECTORY = _T("TempDirectory");
static const std::tstring _INI_NAME_PROFILE_DIRECTORY = _T("ProfileDirectory");
static const std::tstring _INI_NAME_SETTINGS_DIRECTORY = _T("SettingsDirectory");
static const std::tstring _INI_NAME_LOGS_DIRECTORY = _T("LogsDirectory");
static const std::tstring _INI_NAME_DIRDRIVES_FILE = _T("DirDrivesFile");
static const std::tstring _INI_NAME_DIRHISTORY_FILE = _T("DirHistFile");
static const std::tstring _INI_NAME_HISTORY_FILE = _T("HistFile");
static const std::tstring _INI_NAME_COMMANDLOG_FILE = _T("LogName");
static const std::tstring _INI_NAME_ERROSLOG_FILE = _T("LogErrorsName");
static const std::tstring _INI_NAME_HISTORYLOG_FILE = _T("HistLogName");
static const std::tstring _INI_NAME_INI_FILE = _T("TccIniFile");
static const std::tstring _INI_NAME_EDITOR = _T("Editor");
static const std::tstring _INI_NAME_LANGUAGE = _T("Language");
static const std::tstring _INI_NAME_FORCE32 = _T("Force32");
static const std::tstring _INI_NAME_LICENSE = _T("License");
static const std::tstring _INI_NAME_NOTCOMPATIBLE = _T("NotCompatible");
static const std::tstring _INI_NAME_PARAMETERS = _T("Parameters");
static const std::tstring _INI_NAME_WAIT = _T("Wait");

static const std::tstring _INI_VALUE_TCC_INI = _T("tcc.ini");
static const std::tstring _INI_VALUE_PROFILE_DIRECTORY = _T("profile");

static const std::tstring _INI_NAME_WIN_PROFILE = _T("PortableProfile");
static const std::tstring _INI_NAME_PROFILE_USER = _T("USERPROFILE");
static const std::tstring _INI_NAME_PROFILE_ROAMING = _T("APPDATA");
static const std::tstring _INI_NAME_PROFILE_LOCAL = _T("LOCALAPPDATA");
static const std::tstring _INI_NAME_PROFILE_ALLUSERS = _T("ALLUSERSPROFILE");
static const std::tstring _INI_NAME_PROFILE_PUBLIC = _T("PUBLIC");
static const std::tstring _INI_NAME_PROFILE_HOME_DRIVE = _T("HOMEDRIVE");
static const std::tstring _INI_NAME_PROFILE_HOME_PATH = _T("HOMEPATH");
static const std::tstring _INI_NAME_PROFILE_HOME_SHARE = _T("HOMESHARE");

static const pApps::iniFile::iniDefaults defaults[] = {
	{ _SECTION_LAUNCHER, _INI_NAME_SETTINGS_DIRECTORY, _T("settings") },
	{ _SECTION_LAUNCHER, _INI_NAME_INI_FILE, _INI_VALUE_TCC_INI },
	{ _SECTION_LAUNCHER, _INI_NAME_TEMP_DIRECTORY, _ENV_TEMP },
	{ _SECTION_LAUNCHER, _INI_NAME_PROFILE_DIRECTORY, _INI_VALUE_PROFILE_DIRECTORY },
	{ _SECTION_LAUNCHER, _INI_NAME_DIRDRIVES_FILE, _T("DirDrives.dat") },
	{ _SECTION_LAUNCHER, _INI_NAME_DIRHISTORY_FILE, _T("DirHistory.dat") },
	{ _SECTION_LAUNCHER, _INI_NAME_HISTORY_FILE, _T("History.dat") },
	{ _SECTION_LAUNCHER, _INI_NAME_LOGS_DIRECTORY, _T("logs") },
	{ _SECTION_LAUNCHER, _INI_NAME_COMMANDLOG_FILE, _T("command.log") },
	{ _SECTION_LAUNCHER, _INI_NAME_ERROSLOG_FILE, _T("errors.log") },
	{ _SECTION_LAUNCHER, _INI_NAME_HISTORYLOG_FILE, _T("history.log") },
	{ _SECTION_STARTUP, _INI_NAME_FORCE32, _T("false") },
	{ _SECTION_STARTUP, _INI_NAME_LICENSE, _T("false") },
	{ _SECTION_STARTUP, _INI_NAME_NOTCOMPATIBLE, _T("MailSSL CompleteAllFiles AutoProxy AutoFirewall Copyright EverythingSearch FilesCaseSensitive Redo Undo ConsolePopupWindows DebuggerToolTips FileCompletionLooping Lua CompleteAliases CompleteInternals PluginDirectory") },
	{ _SECTION_COMMAND, _INI_NAME_PARAMETERS, _T("/D /Q") },
	{ _SECTION_PROFILE, _INI_NAME_WIN_PROFILE, _T("true") },
	{ _SECTION_PROFILE, _INI_NAME_PROFILE_USER, _T("_thisUser_") },
	{ _SECTION_PROFILE, _INI_NAME_PROFILE_ROAMING, _T("_Roaming_") },
	{ _SECTION_PROFILE, _INI_NAME_PROFILE_LOCAL, _T("_Local_") },
	{ _SECTION_PROFILE, _INI_NAME_PROFILE_ALLUSERS, _T("_AllUsers_") },
	{ _SECTION_PROFILE, _INI_NAME_PROFILE_PUBLIC, _T("_Public_") },
};

static const std::tstring knownEditors[] = {
	_T("UltraEdit Portable\\UltraEditPortable.exe"),
	_T("AkelPadPortable\\AkelPadPortable.exe"),
	_T("GeanyPortable\\GeanyPortable.exe"),
	_T("Notepad++Portable\\Notepad++Portable.exe"),
	_T("Notepad2Portable\\Notepad2Portable.exe"),
	_T("Notepad2-modPortable\\Notepad2-modPortable.exe")
};

/******************************************************************************
*	startup options are:
*	[optional comspec directory - ignored]
*	[[\]@iniFile]
*	list of //directives
*	list of /options
*	[/C | /K] command to execute
*****************************************************************************/
void parseCmdLine(const int argc, const TCHAR* const argv[], boost::optional<std::tstring>& comspec, boost::optional<std::tstring>& inifile, std::vector<std::tstring>& directives, std::vector<std::tstring>& options, std::vector<std::tstring>& command) {
	++argv;  // skip argv[0]
	boost::system::error_code errCode;
	comspec = boost::none;
	inifile = boost::none;
	enum {
		mode_unknown, mode_directive, mode_option, mode_command
	} kMode = mode_unknown;

	for (int idx = 1; argc > idx; ++idx, ++argv) {
		std::tstring option(*argv);

		if (mode_command != kMode && (boost::istarts_with(option, _T("/c")) || boost::istarts_with(option, _T("/k")))) {
			kMode = mode_command;
		}

		if (mode_command == kMode) { // everyting after /C or /K is command
			command.push_back(pApps::quote(option));
			continue;
		}

		if (!inifile && boost::starts_with(option, _T("/@"))) {
			inifile = pApps::unquote(std::tstring(option.begin() + 2, option.end()));
			continue;
		}

		if (!inifile && boost::starts_with(option, _T("@"))) {
			inifile = pApps::unquote(std::tstring(option.begin() + 1, option.end()));
			continue;
		}

		if (boost::starts_with(option, _T("//"))) {
			kMode = mode_directive;
		} else if (boost::starts_with(option, _T("/"))) {
			kMode = mode_option;
		}

		switch (kMode) {
		case mode_directive: {
			size_t pos = option.find(_T("="));

			if (std::string::npos != pos) {
				option = option.substr(0, pos + 1) + pApps::quote(option.substr(pos + 1, std::string::npos));
			}

			directives.push_back(option);
			break;
		}

		case mode_option:
			options.push_back(option);
			break;

		default:
			if (!comspec && boost::filesystem::is_directory(pApps::unquote(option))) {
				comspec = option;
				break;
			}

			kMode = mode_command;
			command.push_back(_T("/K"));
			command.push_back(pApps::quote(option));
			break;
		}
	}
};

/******************************************************************************
*	Parse additional command line arguments
*	added in free form from ini file
*****************************************************************************/

void parseCmdLine(const std::tstring& cmdLine, boost::optional<std::tstring>& comspec, boost::optional<std::tstring>& inifile, std::vector<std::tstring>& directives, std::vector<std::tstring>& options, std::vector<std::tstring>& command) {
	LPWSTR* argv;
	int argc;
	argv = CommandLineToArgvW((_T("dummy.exe ") + cmdLine).c_str(), &argc); // there is a difference in parsing argv0 and the rest

	if (nullptr == argv) {
		return;
	}

	parseCmdLine(argc, argv, comspec, inifile, directives, options, command);
	LocalFree(argv);
};

/******************************************************************************
*	Merge to STL containers
*	used to merge vectors
*****************************************************************************/
template <typename T>
void appendContainer(T& dst, const T& src) {
	if (src.size()) {
		dst.reserve(dst.size() + src.size());
		dst.insert(dst.end(), src.begin(), src.end());
	}
}

/******************************************************************************
*
*	main
*
******************************************************************************/

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[]) {
#ifdef _DEBUG
#ifdef _MSC_VER
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	          _CrtSetBreakAlloc(926);
#endif
#endif

	boost::system::error_code errCode;

	pApps::imbueIO();

	/*******************************************************
	*	Parse environment
	*******************************************************/
	pApps::Environment mEnv(envp);

	/*******************************************************
	*	Identify startup directory
	*******************************************************/
	auto myPid = pApps::_sysPidInfo.getMyPID();
	auto argv0 = pApps::getArgv0(argv, mEnv);
	auto pAppsDir = pApps::findPAppsDir(PORTABLE_APPS_APP_LE_32 / TCC_EXE_LE_32, argv0, mEnv);

	if (!pAppsDir) {
		pApps::abend(boost::_tformat(_T("Unable to find application: %s")) % (PORTABLE_APPS_APP_LE_32 / TCC_EXE_LE_32)._tstring(), 1);
	}
	boost::filesystem::current_path(pAppsDir.get(), errCode);

	/*******************************************************
	*	Setup some environment from PortableApps pltform
	*******************************************************/
	env_PAppsC(pAppsDir, mEnv);
	/*******************************************************
	*	Retrieve command-line
	*	parsing command line have to be done prior to reading INI file
	*	because there can /@IniFileName arg forcing
	*	location of INI file
	*******************************************************/
	boost::optional<std::tstring> commandComspec;
	boost::optional<std::tstring> commandInifile;
	std::vector<std::tstring> commandDirectives;
	std::vector<std::tstring> commandOptions;
	std::vector<std::tstring> commandCommand;
	parseCmdLine(argc, argv, commandComspec, commandInifile, commandDirectives, commandOptions, commandCommand);
	/*******************************************************
	*	Get configuration
	*******************************************************/
	pApps::iniFile launcherIni;
	launcherIni.setDefaults(defaults, _countof(defaults));

	for (size_t idx = 0; _countof(knownEditors) > idx; ++idx) {
		if (boost::filesystem::exists(pAppsDir.get() / _T("..") / knownEditors[idx], errCode)) {
			launcherIni.setDefaults(_SECTION_LAUNCHER, _INI_NAME_EDITOR, knownEditors[idx]);
			break;
		}
	}

	launcherIni.setDefaults(_SECTION_LAUNCHER, _INI_NAME_PROFILE_DIRECTORY, _INI_VALUE_PROFILE_DIRECTORY);
	launcherIni.readIniFile(pAppsDir.get() / pApps::LAUNCHER_INI, false);
	/*******************************************************
	*	Additional TccLePortable.ini files
	*	per Domain, Domain\User, Domain\User\ComputerName
	*	it i ssupported and expected by 4Start.btm: [Command]Force32
	*******************************************************/
	auto profileDirectory = boost::filesystem::absolute(boost::filesystem::path(pApps::Environment::expandEnv(launcherIni.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_PROFILE_DIRECTORY))), pAppsDir.get() / pApps::PORTABLE_APPS_DATA);
	auto profileDomainDirectory = profileDirectory / pApps::getDomainName();
	auto profileRoamingDirectory = profileDirectory / pApps::getUserName();
	auto profileLocalDirectory = profileRoamingDirectory / pApps::getComputerName();

	if (!pApps::makeDirWriteable(profileLocalDirectory)) {
		pApps::abend(boost::_tformat(_T("Cann't write profile: %s")) % profileLocalDirectory._tstring(), 1);
	}

	launcherIni.readIniFile(profileDomainDirectory / pApps::LAUNCHER_INI, false);
	launcherIni.readIniFile(profileRoamingDirectory / pApps::LAUNCHER_INI, false);
	launcherIni.readIniFile(profileLocalDirectory / pApps::LAUNCHER_INI, false);
	/*******************************************************/
	boost::optional<std::tstring> iniValueOpt;
	std::tstring                  iniValueStr;
	/*******************************************************
	*	additional arguments [Command]Parameters
	*******************************************************/
	iniValueOpt = launcherIni.getValue(_SECTION_COMMAND, _INI_NAME_PARAMETERS);

	if (iniValueOpt) {
		boost::optional<std::tstring> dummyComspec;     // ignored
		boost::optional<std::tstring> dummyInifile;     // ignored - there is no way to change ini file now
		parseCmdLine(iniValueOpt.get(), dummyComspec, dummyInifile, commandDirectives, commandOptions, commandCommand);
	}

	/*******************************************************
	*	Profiles
	*******************************************************/

	pApps::YesNoOption yesNoOption;
	if (yesNoOption(launcherIni.getValue(_SECTION_PROFILE, _INI_NAME_WIN_PROFILE))) {
		for (const auto& it : std::array < std::pair<const std::tstring, boost::filesystem::path>, 5 > {
			std::make_pair(_INI_NAME_PROFILE_USER, profileDirectory),
				std::make_pair(_INI_NAME_PROFILE_ROAMING, profileRoamingDirectory),
				std::make_pair(_INI_NAME_PROFILE_LOCAL, profileLocalDirectory),
				std::make_pair(_INI_NAME_PROFILE_ALLUSERS, profileDirectory),
				std::make_pair(_INI_NAME_PROFILE_PUBLIC, profileDirectory)
		}) {
			auto thisVar = it.first;
			auto thisDir = it.second;

			auto thisValue = pApps::Environment::expandEnv(mEnv.get(thisVar));
			thisDir = boost::filesystem::absolute(pApps::Environment::expandEnv(launcherIni.getValueNonEmpty(_SECTION_PROFILE, thisVar)), thisDir);

			if (!pApps::makeDirWriteable(thisDir)) {
				pApps::abend(boost::_tformat(_T("Cann't write profile directory: %s")) % thisDir._tstring(), 1);
			}

			auto profileDirString = pApps::normalize(thisDir);
			mEnv.set(thisVar, profileDirString, _ENV_BACKUP_PREFIX + thisVar);

			if (thisVar == _INI_NAME_PROFILE_USER) {
				mEnv.set(_INI_NAME_PROFILE_HOME_PATH, (thisDir.root_directory() / thisDir.relative_path())._tstring(), _ENV_BACKUP_PREFIX + _INI_NAME_PROFILE_HOME_PATH);
				mEnv.set(_INI_NAME_PROFILE_HOME_DRIVE, thisDir.root_name()._tstring(), _ENV_BACKUP_PREFIX + _INI_NAME_PROFILE_HOME_DRIVE);
				mEnv.set(_INI_NAME_PROFILE_HOME_SHARE, pApps::pathToUnc(thisDir.root_name()._tstring()), _ENV_BACKUP_PREFIX + _INI_NAME_PROFILE_HOME_SHARE);
			}
		}
	}

	/*******************************************************
	*	create Data\Settings, copy defaults
	*******************************************************/
	auto settingsDirectory = boost::filesystem::absolute(boost::filesystem::path(pApps::Environment::expandEnv(launcherIni.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_SETTINGS_DIRECTORY))), pAppsDir.get() / pApps::PORTABLE_APPS_DATA);

	if (!boost::filesystem::exists(settingsDirectory, errCode)) {
		if (!pApps::makeDirWriteable(settingsDirectory)) {
			pApps::abend(boost::_tformat(_T("Cann't write configuration: %s")) % settingsDirectory._tstring(), 1);
		}

		pApps::copyCopy(boost::filesystem::absolute(pApps::PORTABLE_APPS_DEFAULT, pAppsDir.get()), settingsDirectory, errCode);
	}

	// iniFileName can be passed as an argument or retrieved from TccLePortable.ini
	boost::filesystem::path tccIniFilename = commandInifile ? commandInifile.get() : boost::filesystem::absolute(pApps::Environment::expandEnv(launcherIni.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_INI_FILE)), settingsDirectory);

	if (!boost::filesystem::exists(tccIniFilename, errCode)) {
		pApps::copyCopy(boost::filesystem::absolute(pApps::PORTABLE_APPS_DEFAULT / _INI_VALUE_TCC_INI, pAppsDir.get()), tccIniFilename, errCode);
	}

	/*******************************************************
	*	use tcc.ini
	*******************************************************/
	pApps::iniFile tccIni;
	tccIni.readIniFile(tccIniFilename, true);
	auto settingsIni = pApps::quote(pApps::normalize(settingsDirectory));
	tccIni.setValue(_SECTION_4NT, _INI_4NT_NAME_TC_START_PATH, settingsIni);
	tccIni.setValue(_SECTION_4NT, _INI_4NT_NAME_4START_PATH, settingsIni);
	/*******************************************************
	*	Temp directory
	*	  Since PortableApps 10.0.2, if there is directory TempForPortableApps in the root of portable apps, it is used as TEMP
	*	  If setting [TccLePortable]TempForPortableApps is true (default), and ..\..\TempForPortableApps exist, it is used and [TccLePortable]TempDirectory is ignored
	*******************************************************/
	auto tempPath = pAppsDir.get() / _T("..\\..\\TempForPortableApps");
	if (!boost::filesystem::is_directory(tempPath)) {
		iniValueOpt = launcherIni.getValue(_SECTION_LAUNCHER, _INI_NAME_TEMP_DIRECTORY);
		if (iniValueOpt) {
			tempPath = boost::filesystem::absolute(boost::filesystem::path(pApps::Environment::expandEnv(iniValueOpt.get())), pAppsDir.get() / pApps::PORTABLE_APPS_DATA);
		} else {
			tempPath = mEnv.get(_ENV_TEMP);
		}
	}
	tempPath = pApps::normalize(tempPath);
	if (pApps::makeDirWriteable(tempPath)) {
		mEnv.set(_ENV_TEMP, tempPath.c_str());
		mEnv.set(_ENV_TMP, tempPath.c_str());
	}

	/*******************************************************
	*	History. DirHistory, etc
	*
	*	Workaround for bug: http://jpsoft.com/forums/threads/lost-history.5878/
	*	  If History / DirHistory is local, it is handled by TCC.exe
	*	  If History / DirHistory is global, it is handled by tcStart
	*******************************************************/

	iniValueStr = pApps::Environment::expandEnv(launcherIni.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_HISTORY_FILE));
	if (!boost::filesystem::is_directory(profileLocalDirectory / iniValueStr)) {
		bool isLocal = yesNoOption(tccIni.getValue(_SECTION_4NT, _INI_4NT_NAME_LOCAL_HISTORY));

		for (const auto& it : commandOptions) {
			isLocal = isLocal || boost::iequals(it.c_str(), _T("/L:")) || boost::iequals(it.c_str(), _T("/LH"));
		}

		if (isLocal) {
			tccIni.setValue(_SECTION_4NT, _INI_NAME_HISTORY_FILE, pApps::quote(pApps::normalize(profileLocalDirectory / iniValueStr)));
		} else {
			tccIni.setValue(_SECTION_4NT, _INI_NAME_HISTORY_FILE, _T(""));
			mEnv.set(_ENV_HIST_FILE, pApps::quote(pApps::normalize(profileLocalDirectory / iniValueStr)));
		}
	}

	// The same with DirHistory
	iniValueStr = pApps::Environment::expandEnv(launcherIni.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_DIRHISTORY_FILE));

	if (!boost::filesystem::is_directory(profileLocalDirectory / iniValueStr)) {
		bool isLocal = yesNoOption(tccIni.getValue(_SECTION_4NT, _INI_4NT_NAME_LOCALDIR_HISTORY));

		for (const auto& it : commandOptions) {
			isLocal = isLocal || boost::iequals(it.c_str(), _T("/L:")) || boost::iequals(it.c_str(), _T("/LD"));
		}
		if (isLocal) {
			tccIni.setValue(_SECTION_4NT, _INI_NAME_DIRHISTORY_FILE, pApps::quote(pApps::normalize(profileLocalDirectory / iniValueStr)));
		} else {
			tccIni.setValue(_SECTION_4NT, _INI_NAME_DIRHISTORY_FILE, _T(""));
			mEnv.set(_ENV_DIRHIST_FILE, pApps::quote(pApps::normalize(profileLocalDirectory / iniValueStr)));
		}
	}

	// DirDrivesFile
	iniValueStr = pApps::Environment::expandEnv(launcherIni.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_DIRDRIVES_FILE));
	if (!boost::filesystem::is_directory(profileLocalDirectory / iniValueStr)) {
		mEnv.set(_ENV_DIRDRIVES_FILE, pApps::quote(pApps::normalize(profileLocalDirectory / iniValueStr)));
	}

	// TreePath
	tccIni.setValue(_SECTION_4NT, _INI_4NT_NAME_TREE_PATH, pApps::quote(pApps::normalize(profileLocalDirectory)));

	/*******************************************************
	*	Logs
	*******************************************************/
	auto logsDirectory = boost::filesystem::absolute(boost::filesystem::path(pApps::Environment::expandEnv(launcherIni.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_LOGS_DIRECTORY))), pAppsDir.get() / pApps::PORTABLE_APPS_DATA);

	if (!pApps::makeDirWriteable(logsDirectory)) {
		pApps::abend(boost::_tformat(_T("Cann't write logs: %s")) % logsDirectory._tstring(), 1);
	}

	iniValueStr = pApps::Environment::expandEnv(launcherIni.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_COMMANDLOG_FILE));
	tccIni.setValue(_SECTION_4NT, _INI_NAME_COMMANDLOG_FILE, pApps::quote(pApps::normalize(logsDirectory / iniValueStr)));
	iniValueStr = pApps::Environment::expandEnv(launcherIni.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_ERROSLOG_FILE));
	tccIni.setValue(_SECTION_4NT, _INI_NAME_ERROSLOG_FILE, pApps::quote(pApps::normalize(logsDirectory / iniValueStr)));
	iniValueStr = pApps::Environment::expandEnv(launcherIni.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_HISTORYLOG_FILE));
	tccIni.setValue(_SECTION_4NT, _INI_NAME_HISTORYLOG_FILE, pApps::quote(pApps::normalize(logsDirectory / iniValueStr)));
	/*******************************************************
	*	Editor
	*******************************************************/
	iniValueOpt = launcherIni.getValue(_SECTION_LAUNCHER, _INI_NAME_EDITOR);

	if (iniValueOpt) {
		auto editorPath =
			boost::filesystem::absolute(
			boost::filesystem::path(pApps::Environment::expandEnv(iniValueOpt.get())),
			pAppsDir.get() / _T(".."));

		if (boost::filesystem::exists(editorPath, errCode)) {
			tccIni.setValue(_SECTION_4NT, _INI_NAME_EDITOR, pApps::quote(pApps::normalize(editorPath)));
		}
	}

	/*******************************************************
	*	32-bit? 64-bit? TCC licensed or LE
	*******************************************************/
	bool runLicensed = yesNoOption(launcherIni.getValue(_SECTION_STARTUP, _INI_NAME_LICENSE));
	bool runX64 = !yesNoOption(launcherIni.getValue(_SECTION_STARTUP, _INI_NAME_FORCE32)) && (pApps::_sysPidInfo.isWow64());
	runLicensed &= runX64 ? boost::filesystem::exists(pAppsDir.get() / PORTABLE_APPS_APP_FULL_64 / TCC_EXE_FULL_64) : boost::filesystem::exists(pAppsDir.get() / PORTABLE_APPS_APP_FULL_32 / TCC_EXE_FULL_32);
	runX64 &= runLicensed ? boost::filesystem::exists(pAppsDir.get() / PORTABLE_APPS_APP_FULL_64 / TCC_EXE_FULL_64) : boost::filesystem::exists(pAppsDir.get() / PORTABLE_APPS_APP_LE_64 / TCC_EXE_LE_64);
	boost::filesystem::path appDir;
	boost::filesystem::path exeName;

	if (runLicensed) {
		appDir = boost::filesystem::absolute(runX64 ? PORTABLE_APPS_APP_FULL_64 : PORTABLE_APPS_APP_FULL_32, pAppsDir.get());
		exeName = runX64 ? TCC_EXE_FULL_64 : TCC_EXE_FULL_32;
	} else {
		appDir = boost::filesystem::absolute(runX64 ? PORTABLE_APPS_APP_LE_64 : PORTABLE_APPS_APP_LE_32, pAppsDir.get());
		exeName = runX64 ? TCC_EXE_LE_64 : TCC_EXE_LE_32;
	}

	/*******************************************************
	*	Wait for tcc to finish
	*******************************************************/
	iniValueOpt = launcherIni.getValue(_SECTION_STARTUP, _INI_NAME_WAIT);

	auto pWait = pApps::tpWait::pWait_Auto;
	if (yesNoOption(iniValueOpt, true) == yesNoOption(iniValueOpt, false)) {	// value is well defined and equals to "true" or "false" (doesn't fall into default)
		pWait = yesNoOption(iniValueOpt, true) ? pApps::tpWait::pWait_Wait : pApps::tpWait::pWait_NoWait;
	}

	/*******************************************************
	*	Language
	*******************************************************/
	auto languageName = launcherIni.getValue(_SECTION_LAUNCHER, _INI_NAME_LANGUAGE);

	if (languageName && !boost::filesystem::exists(appDir / (languageName.get() + _T(".dll")), errCode)) {
		languageName = boost::none;
	}

	if (!languageName && mEnv.exists(_T("PortableApps.comLanguageName"))) {
		languageName = mEnv.get(_T("PortableApps.comLanguageName"));
	}

	if (languageName && !boost::filesystem::exists(appDir / (languageName.get() + _T(".dll")), errCode)) {
		languageName = boost::none;
	}

	if (languageName) {
		tccIni.setValue(_SECTION_4NT, _T("LanguageDLL"), (languageName.get() + _T(".dll")));
	}

	/*******************************************************
	*	Some [4NT] entries are created under newer TCC
	*	  They are invalid under TCC/LE (older)
	*     Under TCC/LE - make backup [4NT] => [4NT.bak];
	*	  Under TCC - restore
	*     List of entries is defined in TccPortable.ini
	*******************************************************/

	if (runLicensed) {
		auto keys = launcherIni.getValue(_SECTION_STARTUP, _INI_NAME_NOTCOMPATIBLE);
		if (keys) {
			typedef boost::tokenizer<boost::char_separator<TCHAR>, std::_tstring::const_iterator, std::_tstring > tokenizer_t;
			boost::char_separator<TCHAR> sep(_T(",; "));
			tokenizer_t tokens(keys.get(), sep);

			for (const auto& key : tokens) {
				iniValueOpt = tccIni.getValue(_SECTION_4NT_BAK, key);
				if (iniValueOpt) {
					tccIni.setValue(_SECTION_4NT, key, iniValueOpt.get());
					tccIni.setValue(_SECTION_4NT_BAK, key, _T(""));
				}
			}
		}
	}

	/*******************************************************
	*	Copy content of [4NT]
	*	Don't set undocumented entries
	*******************************************************/
	pApps::iniFile::namesSet names4NT;
	launcherIni.enumNames(_SECTION_4NT, names4NT);

	for (const auto& it : names4NT) {
		iniValueOpt = launcherIni.getValue(_SECTION_4NT, it);

		if (iniValueOpt) {
			tccIni.setValue(_SECTION_4NT, pApps::Environment::expandEnv(it), pApps::Environment::expandEnv(iniValueOpt.get()));
		}
	}

	//*****************************************************

	if (!runLicensed) {
		auto keys = launcherIni.getValue(_SECTION_STARTUP, _INI_NAME_NOTCOMPATIBLE);
		if (keys) {
			boost::char_separator<TCHAR> sep(_T(",; "));
			typedef boost::tokenizer<boost::char_separator<TCHAR>, std::_tstring::const_iterator, std::_tstring > tokenizer_t;
			tokenizer_t tokens(keys.get(), sep);

			for (const auto& key : tokens) {
				iniValueOpt = tccIni.getValue(_SECTION_4NT, key);
				if (iniValueOpt) {
					tccIni.setValue(_SECTION_4NT_BAK, key, iniValueOpt.get());
					tccIni.setValue(_SECTION_4NT, key, _T(""));
				}
			}
		}
	}

	/*******************************************************
	*	additional environment variables [Environment]
	*******************************************************/
	pApps::iniFile::namesSet namesEnv;
	launcherIni.enumNames(_SECTION_ENVIRONMENT, namesEnv);

	for (const auto& it : namesEnv) {
		iniValueOpt = launcherIni.getValue(_SECTION_ENVIRONMENT, it);

		if (iniValueOpt) {
			mEnv.set(pApps::Environment::expandEnv(it), pApps::Environment::expandEnv(iniValueOpt.get()));
		}
	}

	/*******************************************************
	*	write tcc.ini
	*******************************************************/
	tccIni.writeIniFile();

	/*******************************************************
	*	Ready to launch
	*******************************************************/
	std::vector<std::tstring> effectiveArgv;
	std::tstring effectiveArgv0 = pApps::normalize(appDir / exeName);
	effectiveArgv.push_back(pApps::quote(effectiveArgv0));
	// commandComspec: ignored
	effectiveArgv.push_back(std::tstring(_T("@")) + pApps::quote(pApps::normalize(tccIniFilename)));
	appendContainer(effectiveArgv, commandDirectives);
	appendContainer(effectiveArgv, commandOptions);
	appendContainer(effectiveArgv, commandCommand);
	boost::filesystem::path cwd = pAppsDir.get();
	pApps::launch(pWait, effectiveArgv0, effectiveArgv, mEnv, cwd);
	return 0;
}
