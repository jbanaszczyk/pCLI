// *************************************************************
// * Copyright 2011 jacek.banaszczyk@gmail.com                 *
// * Part of pCli project: https://github.com/jbanaszczyk/pCli *
// *************************************************************

#include "stdafx.h"

#include "pApps.h"
#include "../common/Environment.h"
#include "../common/SysInfo.h"
#include "../common/IniFile.h"
#include "../common/YesNoOption.h"
#include "../common/Logger.h"

#include <conio.h>
#include <array>

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>


static const boost::filesystem::path PORTABLE_APPS_APP_LE_32 = p_apps::PORTABLE_APPS_APP / _T("TCCLE32");
static const boost::filesystem::path PORTABLE_APPS_APP_LE_64 = p_apps::PORTABLE_APPS_APP / _T("TCCLE64");

const boost::filesystem::path TCC_EXE_LE_32 = _T("tcc.exe");
const boost::filesystem::path TCC_EXE_LE_64 = _T("tcc.exe");

static const std::tstring _SECTION_COMMAND = _T("Command");
static const std::tstring _SECTION_ENVIRONMENT = _T("Environment");
static const std::tstring _SECTION_LAUNCHER = _T("pCli");
static const std::tstring _SECTION_PROFILE = _T("Profile");
static const std::tstring _SECTION_STARTUP = _T("StartUp");

static const std::tstring _SECTION_4NT = _T("4NT");

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

static const p_apps::iniFile::iniDefaults defaults[] = {
		{_SECTION_LAUNCHER, _INI_NAME_SETTINGS_DIRECTORY, _T("settings")},
		{_SECTION_LAUNCHER, _INI_NAME_INI_FILE, _INI_VALUE_TCC_INI},
		{_SECTION_LAUNCHER, _INI_NAME_TEMP_DIRECTORY, _ENV_TEMP},
		{_SECTION_LAUNCHER, _INI_NAME_PROFILE_DIRECTORY, _INI_VALUE_PROFILE_DIRECTORY},
		{_SECTION_LAUNCHER, _INI_NAME_DIRDRIVES_FILE, _T("DirDrives.dat")},
		{_SECTION_LAUNCHER, _INI_NAME_DIRHISTORY_FILE, _T("DirHistory.dat")},
		{_SECTION_LAUNCHER, _INI_NAME_HISTORY_FILE, _T("History.dat")},
		{_SECTION_LAUNCHER, _INI_NAME_LOGS_DIRECTORY, _T("logs")},
		{_SECTION_LAUNCHER, _INI_NAME_COMMANDLOG_FILE, _T("command.log")},
		{_SECTION_LAUNCHER, _INI_NAME_ERROSLOG_FILE, _T("errors.log")},
		{_SECTION_LAUNCHER, _INI_NAME_HISTORYLOG_FILE, _T("history.log")},
		{_SECTION_STARTUP, _INI_NAME_FORCE32, _T("false")},
		{_SECTION_COMMAND, _INI_NAME_PARAMETERS, _T("/D /Q")},
		{_SECTION_PROFILE, _INI_NAME_WIN_PROFILE, _T("true")},
		{_SECTION_PROFILE, _INI_NAME_PROFILE_USER, _T("_thisUser_")},
		{_SECTION_PROFILE, _INI_NAME_PROFILE_ROAMING, _T("_Roaming_")},
		{_SECTION_PROFILE, _INI_NAME_PROFILE_LOCAL, _T("_Local_")},
		{_SECTION_PROFILE, _INI_NAME_PROFILE_ALLUSERS, _T("_AllUsers_")},
		{_SECTION_PROFILE, _INI_NAME_PROFILE_PUBLIC, _T("_Public_")},
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
void parseCmdLine(const int argc, const TCHAR* const argv[], boost::optional<std::tstring>& comspec,
                  boost::optional<std::tstring>& inifile, std::vector<std::tstring>& directives,
                  std::vector<std::tstring>& arguments,
                  std::vector<std::tstring>& commands) {
	++argv; // skip argv[0]

	comspec = boost::none;
	inifile = boost::none;
	bool mode_command = false;

	for (auto argumentIndex = 1; argumentIndex < argc; ++argumentIndex, ++argv) {
		std::tstring argument(*argv);

		if (mode_command) {
			// everything after /C or /K is a command
			commands.push_back(p_apps::quote(argument));
			continue;
		}

		if (boost::istarts_with(argument, _T("/c")) || boost::istarts_with(argument, _T("/k"))) {
			mode_command = true;
			continue;
		}

		if (!inifile && boost::starts_with(argument, _T("/@"))) {
			inifile = p_apps::unquote(std::tstring(argument.begin() + 2, argument.end()));
			continue;
		}

		if (!inifile && boost::starts_with(argument, _T("@"))) {
			inifile = p_apps::unquote(std::tstring(argument.begin() + 1, argument.end()));
			continue;
		}

		if (boost::starts_with(argument, _T("//"))) {
			const auto equalSignPosition = argument.find(_T("="));

			if (equalSignPosition != std::string::npos) {
				argument = argument.substr(0, equalSignPosition + 1) + p_apps::quote(argument.substr(equalSignPosition + 1, std::string::npos));
			}

			directives.push_back(argument);
			continue;
		}

		if (boost::starts_with(argument, _T("/"))) {
			arguments.push_back(argument);
			continue;
		}

		if (!comspec && boost::filesystem::is_directory(p_apps::unquote(argument))) {
			comspec = argument;
			continue;
		}

		mode_command = true;
		commands.push_back(_T("/K"));
		commands.push_back(p_apps::quote(argument));
	}
};

/******************************************************************************
*	Parse additional command line arguments
*	added in free form from ini file
*****************************************************************************/

void parseCmdLineArgumentsFromIniFile(const std::tstring& cmdLine, boost::optional<std::tstring>& comspec,
                                      boost::optional<std::tstring>& inifile, std::vector<std::tstring>& directives,
                                      std::vector<std::tstring>& options,
                                      std::vector<std::tstring>& commands) {

	int argc;
	const auto argv = CommandLineToArgvW((_T("dummy.exe ") + cmdLine).c_str(), &argc);
	// there is a difference in parsing argv0 and the rest

	if (argv == nullptr) {
		return;
	}

	parseCmdLine(argc, argv, comspec, inifile, directives, options, commands);
	LocalFree(argv);
};


class Launcher {
public:
	int argc;
	TCHAR** argv;
	TCHAR** envp;
	p_apps::Environment environment{};
	boost::filesystem::path argv0{};
	boost::filesystem::path pAppsDir{};
	boost::filesystem::path iniFileName{_T(VER_PRODUCTNAME_STR) _T(".ini")};

	Launcher(const Launcher& other) = delete;
	Launcher(Launcher&& other) noexcept = delete;
	Launcher& operator=(const Launcher& other) = delete;
	Launcher& operator=(Launcher&& other) noexcept = delete;
	Launcher(int argc, TCHAR** argv, TCHAR** envp);
	virtual ~Launcher() = default;

	virtual void launch() {
	}

	/******************************************************************************
	 * getArgv0
	 *   Retrieve argv[0] for the current process:
	 *   order:
	 *     real process executable name (from process snap shoot)
	 *     argv[0]
	 *****************************************************************************/
	void getArgv0();

	/******************************************************************************
	 *
	 * findPAppsDir
	 *   Find standard launcher' directory (where launcher ini exist)
	 *   You will get something like <ROOT>\pCli
	 *
	 *   Launcher executable can be executed from any of
	 *     <ROOT>
	 *     <ROOT>\PortableApps\pCli
	 *     <ROOT>\PortableApps\pCli\Other
	 *   In all cases findPAppsDir will point to <ROOT>\PortableApps\pCli
	 *
	 *****************************************************************************/
	virtual void findHomeDirectory() {
	}
};

class TccLauncher final : public Launcher {
	boost::optional<std::tstring> commandComspec;
	boost::optional<std::tstring> commandInifile;
	std::vector<std::tstring> commandDirectives;
	std::vector<std::tstring> commandOptions;
	std::vector<std::tstring> commandCommand;

public:
	TccLauncher(const TccLauncher& other) = delete;
	TccLauncher(TccLauncher&& other) noexcept = delete;
	TccLauncher& operator=(const TccLauncher& other) = delete;
	TccLauncher& operator=(TccLauncher&& other) noexcept = delete;
	TccLauncher(int argc, TCHAR** argv, TCHAR** envp);
	~TccLauncher() override = default;

	void launch() override;
	void findHomeDirectory() override;
	std::optional<boost::filesystem::path> locateTccExe(const boost::filesystem::path& argv0, const boost::filesystem::path& tccExePath) const;
};

Launcher::Launcher(int argc, TCHAR** argv, TCHAR** envp)
	: argc(argc)
  , argv(argv)
  , envp(envp) {

	// make stdin / stdout / stderr locale-aware
	p_apps::imbueIO();
}

void Launcher::getArgv0() {
	auto executableName = SysInfo::getExeName();
	argv0 = absolute(executableName
		                 ? executableName.get()
		                 : argv[0]);
}

TccLauncher::TccLauncher(int argc, TCHAR** argv, TCHAR** envp) // NOLINT(clang-diagnostic-shadow-field)
	: Launcher(argc, argv, envp) {
}

std::optional<boost::filesystem::path> TccLauncher::locateTccExe(const boost::filesystem::path& argv0, const boost::filesystem::path& tccExePath) const {
	// NOLINT(clang-diagnostic-shadow-field)

	boost::filesystem::path possibleExeLocations[] = {
			p_apps::PORTABLE_APPS / argv0.stem(),
			_T(".\\"),
			_T("..\\"),
			_T("..\\..\\..\\..\\..\\..\\..\\..\\")
		};

	auto parentDirectory = argv0.parent_path();

	for (const auto& possibleExeLocation : possibleExeLocations) {
		if (exists(parentDirectory / possibleExeLocation / tccExePath)) {
			return parentDirectory /= possibleExeLocation;
		}
	}
	return std::nullopt;
}

void TccLauncher::findHomeDirectory() {

	pAppsDir = argv0.parent_path();

	static const boost::filesystem::path exePaths[] = {
			p_apps::PORTABLE_APPS_APP / _T("TCCLE32") / TCC_EXE_LE_32,
			p_apps::PORTABLE_APPS_APP / _T("TCCLE64") / TCC_EXE_LE_64
		};

	for (const auto& exePath : exePaths) {
		auto tccExe = locateTccExe(argv0, exePath);
		if (tccExe) {
			pAppsDir = absolute(canonical(tccExe.value()));
			return;
		}
	}

	fail(_T("Unable to find application: %s from %s"), argv0.parent_path());
}

void TccLauncher::launch() {
	logger::trace(_T("Start"));

	environment.setUp(envp);
	logger::trace(_T("Environment: %d variables"), environment.size());

	getArgv0();
	logger::trace(_T("argv0: %s"), argv0);

	findHomeDirectory();
	logger::trace(_T("pApps directory: %s"), pAppsDir);

	/*******************************************************
	*	Retrieve command-line
	*	parsing command line have to be done prior to reading INI file
	*	because there can /@IniFileName arg forcing location of INI file
	*******************************************************/
	parseCmdLine(argc, argv, commandComspec, commandInifile, commandDirectives, commandOptions, commandCommand);
	/*******************************************************
	*	Get configuration
	*******************************************************/

	boost::system::error_code errCode;

	p_apps::iniFile launcherIni;
	launcherIni.setDefaults(defaults, _countof(defaults));

	for (const auto& knownEditor : knownEditors) {
		if (exists(pAppsDir / _T("..") / knownEditor, errCode)) {
			launcherIni.setDefaults(_SECTION_LAUNCHER, _INI_NAME_EDITOR, knownEditor);
			break;
		}
	}

	launcherIni.setDefaults(_SECTION_LAUNCHER, _INI_NAME_PROFILE_DIRECTORY, _INI_VALUE_PROFILE_DIRECTORY);
	launcherIni.readIniFile(pAppsDir / p_apps::LAUNCHER_INI, false);
	/*******************************************************
	*	Additional pCli.ini files
	*	per Domain, Domain\User, Domain\User\ComputerName
	*	it is supported and expected by 4Start.btm: [Command]Force32
	*******************************************************/
	auto profileDirectory = absolute(
		boost::filesystem::path(
			p_apps::Environment::expandEnv(
				launcherIni.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_PROFILE_DIRECTORY))),
		pAppsDir / p_apps::PORTABLE_APPS_DATA);
	auto profileDomainDirectory = profileDirectory / p_apps::getDomainName();
	auto profileRoamingDirectory = profileDirectory / p_apps::getUserName();
	auto profileLocalDirectory = profileRoamingDirectory / p_apps::getComputerName();

	if (!p_apps::makeDirWriteable(profileLocalDirectory)) {
		p_apps::abend(boost::_tformat(_T("Cann't write profile: %s")) % profileLocalDirectory._tstring(), 1);
	}

	launcherIni.readIniFile(profileDomainDirectory / p_apps::LAUNCHER_INI, false);
	launcherIni.readIniFile(profileRoamingDirectory / p_apps::LAUNCHER_INI, false);
	launcherIni.readIniFile(profileLocalDirectory / p_apps::LAUNCHER_INI, false);
	/*******************************************************/
	boost::optional<std::tstring> iniValueOpt;
	std::tstring iniValueStr;
	/*******************************************************
	*	additional arguments [Command]Parameters
	*******************************************************/
	iniValueOpt = launcherIni.getValue(_SECTION_COMMAND, _INI_NAME_PARAMETERS);

	if (iniValueOpt) {
		boost::optional<std::tstring> dummyComspec; // ignored
		boost::optional<std::tstring> dummyInifile; // ignored - there is no way to change ini file now
		parseCmdLineArgumentsFromIniFile(iniValueOpt.get(), dummyComspec, dummyInifile, commandDirectives, commandOptions, commandCommand);
	}

	/*******************************************************
	*	Profiles
	*******************************************************/

	p_apps::YesNoOption yesNoOption;
	if (yesNoOption(launcherIni.getValue(_SECTION_PROFILE, _INI_NAME_WIN_PROFILE))) {
		for (const auto& it : std::array<std::pair<const std::tstring, boost::filesystem::path>, 5>{
			     std::make_pair(_INI_NAME_PROFILE_USER, profileDirectory),
			     std::make_pair(_INI_NAME_PROFILE_ROAMING, profileRoamingDirectory),
			     std::make_pair(_INI_NAME_PROFILE_LOCAL, profileLocalDirectory),
			     std::make_pair(_INI_NAME_PROFILE_ALLUSERS, profileDirectory),
			     std::make_pair(_INI_NAME_PROFILE_PUBLIC, profileDirectory)
		     }) {
			auto thisVar = it.first;
			auto thisDir = it.second;

			auto thisValue = p_apps::Environment::expandEnv(environment.get(thisVar));
			thisDir = absolute(p_apps::Environment::expandEnv(launcherIni.getValueNonEmpty(_SECTION_PROFILE, thisVar)),
			                   thisDir);

			if (!p_apps::makeDirWriteable(thisDir)) {
				p_apps::abend(boost::_tformat(_T("Cann't write profile directory: %s")) % thisDir._tstring(), 1);
			}

			auto profileDirString = p_apps::normalize(thisDir);
			environment.set(thisVar, profileDirString, _ENV_BACKUP_PREFIX + thisVar);

			if (thisVar == _INI_NAME_PROFILE_USER) {
				environment.set(_INI_NAME_PROFILE_HOME_PATH,
				                (thisDir.root_directory() / thisDir.relative_path())._tstring(),
				                _ENV_BACKUP_PREFIX + _INI_NAME_PROFILE_HOME_PATH);
				environment.set(_INI_NAME_PROFILE_HOME_DRIVE, thisDir.root_name()._tstring(),
				                _ENV_BACKUP_PREFIX + _INI_NAME_PROFILE_HOME_DRIVE);
				environment.set(_INI_NAME_PROFILE_HOME_SHARE, p_apps::pathToUnc(thisDir.root_name()._tstring()),
				                _ENV_BACKUP_PREFIX + _INI_NAME_PROFILE_HOME_SHARE);
			}
		}
	}

	/*******************************************************
	*	create Data\Settings, copy defaults
	*******************************************************/
	auto settingsDirectory = absolute(
		boost::filesystem::path(
			p_apps::Environment::expandEnv(
				launcherIni.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_SETTINGS_DIRECTORY))),
		pAppsDir / p_apps::PORTABLE_APPS_DATA);

	if (!exists(settingsDirectory, errCode)) {
		if (!p_apps::makeDirWriteable(settingsDirectory)) {
			p_apps::abend(boost::_tformat(_T("Cann't write configuration: %s")) % settingsDirectory._tstring(), 1);
		}

		p_apps::copyCopy(absolute(p_apps::PORTABLE_APPS_DEFAULT, pAppsDir), settingsDirectory, errCode);
	}

	// iniFileName can be passed as an argument or retrieved from pCli.ini
	auto tccIniFilename = commandInifile
		                      ? commandInifile.get()
		                      : absolute(p_apps::Environment::expandEnv(
			                                 launcherIni.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_INI_FILE)),
		                                 settingsDirectory);

	if (!exists(tccIniFilename, errCode)) {
		p_apps::copyCopy(absolute(p_apps::PORTABLE_APPS_DEFAULT / _INI_VALUE_TCC_INI, pAppsDir), tccIniFilename,
		                 errCode);
	}

	/*******************************************************
	*	use tcc.ini
	*******************************************************/
	p_apps::iniFile tccIni;
	tccIni.readIniFile(tccIniFilename, true);
	auto settingsIni = p_apps::quote(p_apps::normalize(settingsDirectory));
	tccIni.setValue(_SECTION_4NT, _INI_4NT_NAME_TC_START_PATH, settingsIni);
	tccIni.setValue(_SECTION_4NT, _INI_4NT_NAME_4START_PATH, settingsIni);
	/*******************************************************
	*	Temp directory
	*	  Since PortableApps 10.0.2, if there is directory TempForPortableApps in the root of portable apps, it is used as TEMP
	*	  If setting [pCli]TempForPortableApps is true (default), and ..\..\TempForPortableApps exist, it is used and [pCli]TempDirectory is ignored
	*******************************************************/
	auto tempPath = pAppsDir / _T("..\\..\\TempForPortableApps");
	if (!is_directory(tempPath)) {
		iniValueOpt = launcherIni.getValue(_SECTION_LAUNCHER, _INI_NAME_TEMP_DIRECTORY);
		if (iniValueOpt) {
			tempPath = absolute(boost::filesystem::path(p_apps::Environment::expandEnv(iniValueOpt.get())),
			                    pAppsDir / p_apps::PORTABLE_APPS_DATA);
		} else {
			tempPath = environment.get(_ENV_TEMP);
		}
	}
	tempPath = p_apps::normalize(tempPath);
	if (p_apps::makeDirWriteable(tempPath)) {
		environment.set(_ENV_TEMP, tempPath.c_str());
		environment.set(_ENV_TMP, tempPath.c_str());
	}

	/*******************************************************
	*	History. DirHistory, etc
	*
	*	Workaround for bug: http://jpsoft.com/forums/threads/lost-history.5878/
	*	  If History / DirHistory is local, it is handled by TCC.exe
	*	  If History / DirHistory is global, it is handled by tcStart
	*******************************************************/

	iniValueStr = p_apps::Environment::expandEnv(
		launcherIni.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_HISTORY_FILE));
	if (!is_directory(profileLocalDirectory / iniValueStr)) {
		auto isLocal = yesNoOption(tccIni.getValue(_SECTION_4NT, _INI_4NT_NAME_LOCAL_HISTORY));

		for (const auto& it : commandOptions) {
			isLocal = isLocal || boost::iequals(it.c_str(), _T("/L:")) || boost::iequals(it.c_str(), _T("/LH"));
		}

		if (isLocal) {
			tccIni.setValue(_SECTION_4NT, _INI_NAME_HISTORY_FILE,
			                p_apps::quote(p_apps::normalize(profileLocalDirectory / iniValueStr)));
		} else {
			tccIni.setValue(_SECTION_4NT, _INI_NAME_HISTORY_FILE, _T(""));
			environment.set(_ENV_HIST_FILE, p_apps::quote(p_apps::normalize(profileLocalDirectory / iniValueStr)));
		}
	}

	// The same with DirHistory
	iniValueStr = p_apps::Environment::expandEnv(
		launcherIni.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_DIRHISTORY_FILE));

	if (!is_directory(profileLocalDirectory / iniValueStr)) {
		auto isLocal = yesNoOption(tccIni.getValue(_SECTION_4NT, _INI_4NT_NAME_LOCALDIR_HISTORY));

		for (const auto& it : commandOptions) {
			isLocal = isLocal || boost::iequals(it.c_str(), _T("/L:")) || boost::iequals(it.c_str(), _T("/LD"));
		}
		if (isLocal) {
			tccIni.setValue(_SECTION_4NT, _INI_NAME_DIRHISTORY_FILE,
			                p_apps::quote(p_apps::normalize(profileLocalDirectory / iniValueStr)));
		} else {
			tccIni.setValue(_SECTION_4NT, _INI_NAME_DIRHISTORY_FILE, _T(""));
			environment.set(_ENV_DIRHIST_FILE, p_apps::quote(p_apps::normalize(profileLocalDirectory / iniValueStr)));
		}
	}

	// DirDrivesFile
	iniValueStr = p_apps::Environment::expandEnv(
		launcherIni.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_DIRDRIVES_FILE));
	if (!is_directory(profileLocalDirectory / iniValueStr)) {
		environment.set(_ENV_DIRDRIVES_FILE, p_apps::quote(p_apps::normalize(profileLocalDirectory / iniValueStr)));
	}

	// TreePath
	tccIni.setValue(_SECTION_4NT, _INI_4NT_NAME_TREE_PATH, p_apps::quote(p_apps::normalize(profileLocalDirectory)));

	/*******************************************************
	*	Logs
	*******************************************************/
	auto logsDirectory = absolute(
		boost::filesystem::path(
			p_apps::Environment::expandEnv(launcherIni.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_LOGS_DIRECTORY))),
		pAppsDir / p_apps::PORTABLE_APPS_DATA);

	if (!p_apps::makeDirWriteable(logsDirectory)) {
		p_apps::abend(boost::_tformat(_T("Cann't write logs: %s")) % logsDirectory._tstring(), 1);
	}

	iniValueStr = p_apps::Environment::expandEnv(
		launcherIni.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_COMMANDLOG_FILE));
	tccIni.setValue(_SECTION_4NT, _INI_NAME_COMMANDLOG_FILE,
	                p_apps::quote(p_apps::normalize(logsDirectory / iniValueStr)));
	iniValueStr = p_apps::Environment::expandEnv(
		launcherIni.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_ERROSLOG_FILE));
	tccIni.setValue(_SECTION_4NT, _INI_NAME_ERROSLOG_FILE,
	                p_apps::quote(p_apps::normalize(logsDirectory / iniValueStr)));
	iniValueStr = p_apps::Environment::expandEnv(
		launcherIni.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_HISTORYLOG_FILE));
	tccIni.setValue(_SECTION_4NT, _INI_NAME_HISTORYLOG_FILE,
	                p_apps::quote(p_apps::normalize(logsDirectory / iniValueStr)));
	/*******************************************************
	*	Editor
	*******************************************************/
	iniValueOpt = launcherIni.getValue(_SECTION_LAUNCHER, _INI_NAME_EDITOR);

	if (iniValueOpt) {
		auto editorPath =
			absolute(
				boost::filesystem::path(p_apps::Environment::expandEnv(iniValueOpt.get())),
				pAppsDir / _T(".."));

		if (exists(editorPath, errCode)) {
			tccIni.setValue(_SECTION_4NT, _INI_NAME_EDITOR, p_apps::quote(p_apps::normalize(editorPath)));
		}
	}

	/*******************************************************
	*	32-bit? 64-bit?
	*******************************************************/

	auto runX64 = !yesNoOption(launcherIni.getValue(_SECTION_STARTUP, _INI_NAME_FORCE32)) && (SysInfo::isWow64());
	runX64 &= exists(pAppsDir / PORTABLE_APPS_APP_LE_64 / TCC_EXE_LE_64);
	boost::filesystem::path appDir;
	boost::filesystem::path exeName;

	appDir = absolute(runX64
		                  ? PORTABLE_APPS_APP_LE_64
		                  : PORTABLE_APPS_APP_LE_32,
	                  pAppsDir);
	exeName = runX64
		          ? TCC_EXE_LE_64
		          : TCC_EXE_LE_32;

	/*******************************************************
	*	Wait for tcc to finish
	*******************************************************/
	auto pWait = yesNoOption(launcherIni.getValue(_SECTION_STARTUP, _INI_NAME_WAIT))
		             ? p_apps::tpWait::pWait_Wait
		             : p_apps::tpWait::pWait_NoWait;

	/*******************************************************
	*	Language
	*******************************************************/
	auto languageName = launcherIni.getValue(_SECTION_LAUNCHER, _INI_NAME_LANGUAGE);

	if (languageName && !exists(appDir / (languageName.get() + _T(".dll")), errCode)) {
		languageName = boost::none;
	}

	if (!languageName && environment.exists(_T("PortableApps.comLanguageName"))) {
		languageName = environment.get(_T("PortableApps.comLanguageName"));
	}

	if (languageName && !exists(appDir / (languageName.get() + _T(".dll")), errCode)) {
		languageName = boost::none;
	}

	if (languageName) {
		tccIni.setValue(_SECTION_4NT, _T("LanguageDLL"), (languageName.get() + _T(".dll")));
	}

	/*******************************************************
	*	Copy content of [4NT]
	*	Don't set undocumented entries
	*******************************************************/
	p_apps::SetInsensitiveTChar names4NT;
	launcherIni.enumNames(_SECTION_4NT, names4NT);

	for (const auto& it : names4NT) {
		iniValueOpt = launcherIni.getValue(_SECTION_4NT, it);

		if (iniValueOpt) {
			tccIni.setValue(_SECTION_4NT, p_apps::Environment::expandEnv(it),
			                p_apps::Environment::expandEnv(iniValueOpt.get()));
		}
	}

	/*******************************************************
	*	additional environment variables [Environment]
	*******************************************************/
	p_apps::SetInsensitiveTChar namesEnv;
	launcherIni.enumNames(_SECTION_ENVIRONMENT, namesEnv);

	for (const auto& it : namesEnv) {
		iniValueOpt = launcherIni.getValue(_SECTION_ENVIRONMENT, it);

		if (iniValueOpt) {
			environment.set(p_apps::Environment::expandEnv(it), p_apps::Environment::expandEnv(iniValueOpt.get()));
		}
	}

	/*******************************************************
	*	write tcc.ini
	*******************************************************/
	tccIni.writeIniFile();

	/*******************************************************
	*	Ready to execute
	*******************************************************/
	std::vector<std::tstring> effectiveArgv;
	auto effectiveArgv0 = p_apps::normalize(appDir / exeName);
	effectiveArgv.push_back(p_apps::quote(effectiveArgv0));
	// commandComspec: ignored
	effectiveArgv.push_back(std::tstring(_T("@")) + p_apps::quote(p_apps::normalize(tccIniFilename)));
	p_apps::appendContainer(effectiveArgv, commandDirectives);
	p_apps::appendContainer(effectiveArgv, commandOptions);
	p_apps::appendContainer(effectiveArgv, commandCommand);
	auto cwd = pAppsDir;
	execute(pWait, effectiveArgv0, effectiveArgv, environment, cwd);
}

/******************************************************************************
*
*	main
*
******************************************************************************/

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[]) {
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	// _CrtSetBreakAlloc(522);
#endif

	logger::init();

	TccLauncher tccLauncher{argc, argv, envp};
	tccLauncher.launch();
	return 0;
}
