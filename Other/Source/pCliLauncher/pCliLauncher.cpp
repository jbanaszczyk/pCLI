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

p_apps::YesNoOption yesNoOption;

static const std::filesystem::path PORTABLE_APPS_APP_LE_32 = p_apps::PORTABLE_APPS_APP / _T("TCCLE32");
static const std::filesystem::path PORTABLE_APPS_APP_LE_64 = p_apps::PORTABLE_APPS_APP / _T("TCCLE64");

const std::filesystem::path TCC_EXE_LE_32 = _T("tcc.exe");
const std::filesystem::path TCC_EXE_LE_64 = _T("tcc.exe");

static const std::tstring _SECTION_COMMAND = _T("Command");
static const std::tstring _SECTION_ENVIRONMENT = _T("Environment");
static const std::tstring _SECTION_LAUNCHER = _T("pCli");
static const std::tstring _SECTION_PROFILE = _T("Profile");

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
static const std::tstring _INI_NAME_WAIT = _T("Wait");

static const std::tstring _INI_NAME_COMMANDS = _T("Commands");
static const std::tstring _INI_NAME_OPTIONS = _T("Options");
static const std::tstring _INI_NAME_DIRECTIVES = _T("Directives");

static const std::tstring _INI_VALUE_TCC_INI = _T("tcc.ini");

static const std::tstring _INI_VALUE_PROFILE_DIRECTORY = _T("profile");
static const std::tstring _INI_NAME_PROFILE_PORTABLE = _T("PortableProfile");
static const std::tstring _INI_NAME_PROFILE_USER = _T("USERPROFILE");
static const std::tstring _INI_NAME_PROFILE_ROAMING = _T("APPDATA");
static const std::tstring _INI_NAME_PROFILE_LOCAL = _T("LOCALAPPDATA");
static const std::tstring _INI_NAME_ENV_HOME_DRIVE = _T("HOMEDRIVE");
static const std::tstring _INI_NAME_ENV_HOME_PATH = _T("HOMEPATH");
static const std::tstring _INI_NAME_ENV_HOME_SHARE = _T("HOMESHARE");
static const std::tstring _INI_VALUE_SKIP_PROFILE = _T(".");

static const p_apps::IniFile::iniDefaults defaults[] = {
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
		{_SECTION_LAUNCHER, _INI_NAME_FORCE32, _T("false")},

		{_SECTION_COMMAND, _INI_NAME_COMMANDS, _T("/D")},

		{_SECTION_PROFILE, _INI_NAME_PROFILE_PORTABLE, _T("true")},
		{_SECTION_PROFILE, _INI_NAME_PROFILE_USER, _T("_thisUser_")},
		{_SECTION_PROFILE, _INI_NAME_PROFILE_ROAMING, _T("_Roaming_")},
		{_SECTION_PROFILE, _INI_NAME_PROFILE_LOCAL, _T("_Local_")},
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
*	/? (usage)
*	@@myIniFile (alternative to pCli.ini)
*	[[\]@iniFile]
*	list of //directives
*	list of /options
*	[/C | /K] command to execute
*****************************************************************************/
/******************************************************************************
*	Parse additional command line arguments
*	added in free form from ini file
*****************************************************************************/

class Launcher {
public:
	int argc;
	TCHAR** argv;
	TCHAR** envp;
	p_apps::Environment environment{};
	std::filesystem::path argv0{};
	std::filesystem::path pAppsDir{};
	std::filesystem::path iniFileName{_T(VER_PRODUCTNAME_STR) _T(".ini")};

	Launcher(const Launcher& other) = delete;
	Launcher(Launcher&& other) noexcept = delete;
	Launcher& operator=(const Launcher& other) = delete;
	Launcher& operator=(Launcher&& other) noexcept = delete;
	Launcher(int argc, TCHAR** argv, TCHAR** envp);
	virtual ~Launcher() = default;

	virtual void launch() = 0;

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

class Commands : public std::vector<std::wstring> {
public:
	// ReSharper disable once CppInconsistentNaming
	void push_back(const std::wstring& command);

	static bool isSlashC(const std::wstring& val) {
		return boost::iequals(val, _T("/c")) || boost::iequals(val, _T("/k"));
	}
};

void Commands::push_back(const std::wstring& command) {
	if (this->empty()) {
		if (!isSlashC(command)) {
			push_back(std::wstring{_T("/k")});
		}
	} else {
		if (isSlashC(command)) {
			push_back(std::wstring{_T("&")});
			return;
		}
	}
	std::vector<std::wstring>::push_back(p_apps::quote(command));
}

class Directives : public std::vector<std::wstring> {
public:
	// ReSharper disable once CppInconsistentNaming
	void push_back(const std::wstring& directive);
};

void Directives::push_back(const std::wstring& directive) {
	const auto equalSignPosition = directive.find(_T("="));

	if (equalSignPosition != std::string::npos) {
		auto key = directive.substr(0, equalSignPosition);
		if (!boost::starts_with(key, _T("//"))) {
			key = _T("//") + key;
		}

		auto value = p_apps::quote(directive.substr(equalSignPosition + 1, std::string::npos));

		std::vector<std::wstring>::push_back(key + _T("=") + value);
	}
}

class TccLauncher final : public Launcher {
	boost::optional<std::tstring> commandComspec = boost::none;
	boost::optional<std::tstring> commandInifile = boost::none;
	Directives commandDirectives;
	std::vector<std::tstring> commandOptions;
	Commands commandCommand;
	boost::optional<std::tstring> myIniFile = boost::none;
	p_apps::IniFile launcherIni;
	std::filesystem::path profileDirectory;
	std::filesystem::path profileDomainDirectory;
	std::filesystem::path profileRoamingDirectory;
	std::filesystem::path profileLocalDirectory;

public:
	TccLauncher(const TccLauncher& other) = delete;
	TccLauncher(TccLauncher&& other) noexcept = delete;
	TccLauncher& operator=(const TccLauncher& other) = delete;
	TccLauncher& operator=(TccLauncher&& other) noexcept = delete;
	TccLauncher(int argc, TCHAR** argv, TCHAR** envp);
	~TccLauncher() override = default;

	void readIniSectionCommand();
	void logCommandLine() const;
	void readIniSectionProfile();

	void copyDefaultData();

	void launch() override;

	void readProfileIniFiles();

	template <class T>
	void readIniSectionCommandPartial(const std::tstring& iniName, T& consumer);

	[[nodiscard]] std::optional<std::filesystem::path> locateTccExe(const std::filesystem::path& argv0, const std::filesystem::path& tccExePath) const;
	void findHomeDirectory() override;
	void setupProfileDirectories();
	void readMainIniFile();
	void usage();
	void parseCmdLine(bool& getHelp);
};

Launcher::Launcher(int argc, TCHAR** argv, TCHAR** envp)
	: argc(argc)
  , argv(argv)
  , envp(envp) {
	p_apps::imbueIO();
}

void Launcher::getArgv0() {
	auto executableName = SysInfo::getExeName();
	argv0 = absolute(executableName
		                 ? executableName.value()
		                 : argv[0]);
	logger::trace(_T("[%s] argv0: %s"), _T(__FUNCTION__), argv0);
}

TccLauncher::TccLauncher(int argc, TCHAR** argv, TCHAR** envp) // NOLINT(clang-diagnostic-shadow-field)
	: Launcher(argc, argv, envp) {
}

std::optional<std::filesystem::path> TccLauncher::locateTccExe(const std::filesystem::path& argv0, const std::filesystem::path& tccExePath) const {
	// NOLINT(clang-diagnostic-shadow-field)

	std::filesystem::path possibleExeLocations[] = {
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

	static const std::filesystem::path exePaths[] = {
			PORTABLE_APPS_APP_LE_32 / TCC_EXE_LE_32,
			PORTABLE_APPS_APP_LE_64 / TCC_EXE_LE_64
		};

	for (const auto& exePath : exePaths) {
		if (const auto tccExe = locateTccExe(argv0, exePath)) {
			pAppsDir = canonical(tccExe.value());
			logger::trace(_T("[%s] pApps directory: %s"), _T(__FUNCTION__), pAppsDir);
			return;
		}
	}

	fail(_T("Unable to find tcc.exe from %s"), argv0.parent_path());
}

void TccLauncher::parseCmdLine(bool& getHelp) {
	++argv; // skip argv[0]

	getHelp = false;
	bool mode_command = false;

	for (auto argumentIndex = 1; argumentIndex < argc; ++argumentIndex, ++argv) {
		std::tstring argument(*argv);

		if (argument == _T("/?")) {
			getHelp = true;
			continue;
		}

		if (boost::iequals(argument, _T("/c")) || boost::iequals(argument, _T("/k"))) {
			mode_command = true;
		}

		if (mode_command) {
			// everything after /C or /K is a command
			commandCommand.push_back(p_apps::quote(argument));
			continue;
		}

		if (!myIniFile && boost::starts_with(argument, _T("@@"))) {
			myIniFile = p_apps::unquote(std::tstring(argument.begin() + 2, argument.end()));
			continue;
		}

		if (!commandInifile && boost::starts_with(argument, _T("/@"))) {
			commandInifile = p_apps::unquote(std::tstring(argument.begin() + 2, argument.end()));
			continue;
		}

		if (!commandInifile && boost::starts_with(argument, _T("@"))) {
			commandInifile = p_apps::unquote(std::tstring(argument.begin() + 1, argument.end()));
			continue;
		}

		if (boost::starts_with(argument, _T("//"))) {
			const auto equalSignPosition = argument.find(_T("="));

			if (equalSignPosition != std::string::npos) {
				argument = argument.substr(0, equalSignPosition + 1) + p_apps::quote(argument.substr(equalSignPosition + 1, std::string::npos));
			}

			commandDirectives.push_back(argument);
			continue;
		}

		if (boost::starts_with(argument, _T("/"))) {
			commandOptions.push_back(argument);
			continue;
		}

		if (!commandComspec && std::filesystem::is_directory(p_apps::unquote(argument))) {
			commandComspec = argument;
			continue;
		}

		mode_command = true;
		commandCommand.push_back(p_apps::quote(argument));
	}

	if (commandInifile) {
		logger::trace(_T("[%s] inifile: %s"), _T(__FUNCTION__), commandInifile.value());
	}
	logCommandLine();
};

void TccLauncher::usage() {
	std::wcout <<
		"Start a new instance of portable TCC/LE command processor\n\n"
		"Arguments:\n"
		"\t@@PathToIniFile: Alternative " VER_PRODUCTNAME_STR ".ini\n"
		"\tall other arguments are passed to tcc.exe\n"
		<< std::endl;
}

void TccLauncher::readMainIniFile() {
	launcherIni.setDefaults(defaults, _countof(defaults));

	for (const auto& knownEditor : knownEditors) {
		if (exists(pAppsDir / _T("..") / knownEditor)) {
			launcherIni.setDefaults(_SECTION_LAUNCHER, _INI_NAME_EDITOR, knownEditor);
			break;
		}
	}

	if (!myIniFile) {
		myIniFile = pAppsDir / p_apps::LAUNCHER_INI;
	}
	logger::trace(_T("[%s] using INI file: %s"), _T(__FUNCTION__), myIniFile.value());

	launcherIni.readIniFile(myIniFile.value(), false);
}

void TccLauncher::setupProfileDirectories() {
	// FIXME another profile should be supported by tcstart.btm
	profileDirectory = p_apps::canonical(std::filesystem::path(p_apps::Environment::expandEnv(launcherIni.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_PROFILE_DIRECTORY))),
	                                     pAppsDir / p_apps::PORTABLE_APPS_DATA);
	profileDomainDirectory = profileDirectory / p_apps::getDomainName();
	profileRoamingDirectory = profileDirectory / p_apps::getUserName();
	profileLocalDirectory = profileRoamingDirectory / p_apps::getComputerName();
}

void TccLauncher::readProfileIniFiles() {
	std::filesystem::path moreIniLocations[] = {
			profileDomainDirectory,
			profileRoamingDirectory,
			profileLocalDirectory
		};

	for (const auto& iniLocation : moreIniLocations) {
		p_apps::makeDirWriteable(iniLocation);
		launcherIni.readIniFile(iniLocation / p_apps::LAUNCHER_INI, false);
	}
}

template <typename T>
void TccLauncher::readIniSectionCommandPartial(const std::tstring& iniName, T& consumer) {
	if (auto iniValue = launcherIni.getValue(_SECTION_COMMAND, iniName)) {
		logger::trace(_T("[%s] %s: %s"), _T(__FUNCTION__), iniName, iniValue.value());
		for (const auto& token : p_apps::tokenize(iniValue.value())) {
			consumer.push_back(token);
		}
	}
}

void TccLauncher::readIniSectionCommand() {
	readIniSectionCommandPartial(_INI_NAME_COMMANDS, commandCommand);
	readIniSectionCommandPartial(_INI_NAME_OPTIONS, commandOptions);
	readIniSectionCommandPartial(_INI_NAME_DIRECTIVES, commandDirectives);
	logCommandLine();
}

void TccLauncher::logCommandLine() const {
#ifdef _DEBUG
	if (!commandDirectives.empty()) {
		logger::trace(_T("[%s] directives: %s"), _T(__FUNCTION__), boost::algorithm::join(commandDirectives, _T(" ")));
	}
	if (!commandOptions.empty()) {
		logger::trace(_T("[%s] options: %s"), _T(__FUNCTION__), boost::algorithm::join(commandOptions, _T(" ")));
	}
	if (!commandCommand.empty()) {
		logger::trace(_T("[%s] commands: %s"), _T(__FUNCTION__), boost::algorithm::join(commandCommand, _T(" ")));
	}
#endif
}

void TccLauncher::readIniSectionProfile() {
	if (yesNoOption(launcherIni.getValue(_SECTION_PROFILE, _INI_NAME_PROFILE_PORTABLE))) {

		static const std::tstring iniProfileNames[] = {
				_INI_NAME_PROFILE_USER,
				_INI_NAME_PROFILE_USER,
				_INI_NAME_PROFILE_LOCAL
			};

		for (const auto& profileName : iniProfileNames) {

			auto profileSettings = launcherIni.getValueNonEmpty(_SECTION_PROFILE, profileName);
			if (profileSettings == _INI_VALUE_SKIP_PROFILE) {
				continue;
			}
			auto profilePath = p_apps::canonical(p_apps::Environment::expandEnv(profileSettings), profileDirectory);

			p_apps::makeDirWriteable(profilePath);

			environment.set(profileName, profilePath._tstring(), _ENV_BACKUP_PREFIX + profileName);

			if (profileName == _INI_NAME_PROFILE_USER) {
				environment.set(_INI_NAME_ENV_HOME_PATH, (profilePath.root_directory() / profilePath.relative_path())._tstring(), _ENV_BACKUP_PREFIX + _INI_NAME_ENV_HOME_PATH);
				environment.set(_INI_NAME_ENV_HOME_DRIVE, profilePath.root_name()._tstring(), _ENV_BACKUP_PREFIX + _INI_NAME_ENV_HOME_DRIVE);
				environment.set(_INI_NAME_ENV_HOME_SHARE, p_apps::pathToUnc(profilePath.root_name()._tstring()), _ENV_BACKUP_PREFIX + _INI_NAME_ENV_HOME_SHARE);
			}
		}
	}
}

void TccLauncher::copyDefaultData() {
	const auto settingsDirectory = p_apps::canonical(
		std::filesystem::path(
			p_apps::Environment::expandEnv(
				launcherIni.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_SETTINGS_DIRECTORY))), pAppsDir / p_apps::PORTABLE_APPS_DATA);

	if (!is_directory(settingsDirectory)) {
		const auto sourceDirectory = p_apps::canonical(p_apps::PORTABLE_APPS_DEFAULT_DATA, pAppsDir);
		std::error_code errorCode;
		std::filesystem::copy(sourceDirectory, settingsDirectory, errorCode);
		if (errorCode) {
			fail(_T("[%s] Cann't copy directory\nfrom %s to %s\nReason: %s"), _T(__FUNCTION__), sourceDirectory._tstring(), settingsDirectory._tstring(), p_apps::string2wstring(errorCode.message()));
		}
	}

	// iniFileName can be passed as an argument or retrieved from pCli.ini

	// ReSharper disable once CppTooWideScopeInitStatement
	std::tstring tccIniFilename = commandInifile
		                      ? commandInifile.get()
		                      : p_apps::canonical(p_apps::Environment::expandEnv(
			                                          launcherIni.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_INI_FILE)), settingsDirectory);

	if (!std::filesystem::exists(tccIniFilename)) {
		const auto sourceFile = p_apps::canonical(p_apps::PORTABLE_APPS_DEFAULT_DATA / _INI_VALUE_TCC_INI, pAppsDir);

		std::error_code errorCode;
		std::filesystem::copy(sourceFile, tccIniFilename, errorCode);
		if (errorCode) {
			fail(_T("[%s] Cann't copy file\nfrom %s to %s\nReason: %s"), _T(__FUNCTION__), sourceFile._tstring(), tccIniFilename, p_apps::string2wstring(errorCode.message()));
		}
	}
}

void TccLauncher::launch() {
	logger::trace(_T("[%s] Start"), _T(__FUNCTION__));

	environment.setUp(envp);

	getArgv0();

	findHomeDirectory();

	auto showUsage = false;
	parseCmdLine(showUsage);
	if (showUsage) {
		usage();
		exit(0); // NOLINT(concurrency-mt-unsafe)
	}

	readMainIniFile();

	setupProfileDirectories();

	readProfileIniFiles();

	readIniSectionCommand();

	readIniSectionProfile();

	copyDefaultData();

	/*******************************************************/
	boost::optional<std::tstring> iniValueOpt;
	std::tstring iniValueStr;


	/*******************************************************
	*	use tcc.ini
	*******************************************************/

	const auto settingsDirectory = p_apps::canonical(
		std::filesystem::path(
			p_apps::Environment::expandEnv(
				launcherIni.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_SETTINGS_DIRECTORY))), pAppsDir / p_apps::PORTABLE_APPS_DATA);

	std::tstring tccIniFilename = commandInifile
		? commandInifile.get()
		: p_apps::canonical(p_apps::Environment::expandEnv(
			launcherIni.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_INI_FILE)), settingsDirectory);

	p_apps::IniFile tccIni;
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
			std::filesystem::current_path(pAppsDir / p_apps::PORTABLE_APPS_DATA);
			tempPath = absolute(std::filesystem::path(p_apps::Environment::expandEnv(iniValueOpt.get())));
		} else {
			tempPath = environment.get(_ENV_TEMP);
		}
	}
	tempPath = p_apps::normalize(tempPath);
	p_apps::makeDirWriteable(tempPath);
	environment.set(_ENV_TEMP, tempPath.c_str());
	environment.set(_ENV_TMP, tempPath.c_str());


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
	std::filesystem::current_path(pAppsDir / p_apps::PORTABLE_APPS_DATA);
	auto logsDirectory = absolute(
		std::filesystem::path(
			p_apps::Environment::expandEnv(launcherIni.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_LOGS_DIRECTORY))));

	p_apps::makeDirWriteable(logsDirectory);

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
		std::filesystem::current_path(pAppsDir / _T(".."));
		auto editorPath =
			absolute(
				std::filesystem::path(p_apps::Environment::expandEnv(iniValueOpt.get())));

		if (exists(editorPath)) {
			tccIni.setValue(_SECTION_4NT, _INI_NAME_EDITOR, p_apps::quote(p_apps::normalize(editorPath)));
		}
	}

	/*******************************************************
	*	32-bit? 64-bit?
	*******************************************************/

	auto runX64 = !yesNoOption(launcherIni.getValue(_SECTION_LAUNCHER, _INI_NAME_FORCE32)) && (SysInfo::isWow64());
	runX64 &= exists(pAppsDir / PORTABLE_APPS_APP_LE_64 / TCC_EXE_LE_64);

	std::filesystem::current_path(pAppsDir);
	auto appDir = absolute(runX64
		                       ? PORTABLE_APPS_APP_LE_64
		                       : PORTABLE_APPS_APP_LE_32);
	auto exeName = runX64
		               ? TCC_EXE_LE_64
		               : TCC_EXE_LE_32;

	/*******************************************************
	*	Wait for tcc to finish
	*******************************************************/
	auto pWait = yesNoOption(launcherIni.getValue(_SECTION_LAUNCHER, _INI_NAME_WAIT))
		             ? p_apps::tpWait::pWait_Wait
		             : p_apps::tpWait::pWait_NoWait;

	/*******************************************************
	*	Language
	*******************************************************/
	auto languageName = launcherIni.getValue(_SECTION_LAUNCHER, _INI_NAME_LANGUAGE);

	if (languageName && !exists(appDir / (languageName.get() + _T(".dll")))) {
		languageName = boost::none;
	}

	if (!languageName && environment.exists(_T("PortableApps.comLanguageName"))) {
		languageName = environment.get(_T("PortableApps.comLanguageName"));
	}

	if (languageName && !exists(appDir / (languageName.get() + _T(".dll")))) {
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
