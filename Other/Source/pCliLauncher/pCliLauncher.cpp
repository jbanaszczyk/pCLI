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
#include <boost/tokenizer.hpp>

p_apps::YesNoOption yesNoOption;

/******************************************************************************
 *
 * Some handy strings
 *
 * LAUNCHER_INI
 *   Name of launcher INI file
 *
 * PORTABLE_APPS
 *   Directory, where PortableApps platform is expected
 *
 * PORTABLE_APPS_APP
 *   Legacy name of application directory ( ie. "App" )
 *
 * PORTABLE_APPS_APP_LE_32
 *   Not legacy name of application 32-bit directory ( ie. "App\\32" )
 *
 * PORTABLE_APPS_APP_LE_64
 *   Not legacy name of application 64-bit directory ( ie. "App\\64" )
 *
 * PORTABLE_APPS_DATA
 *   Legacy name of data directory ( ie. "Data" )
 *
 * PORTABLE_APPS_DEFAULT
 *   Legacy name of defaults directory ( ie. "App\\DefaultData" )
 *
 *****************************************************************************/

static const std::filesystem::path LAUNCHER_INI(_T(VER_PRODUCTNAME_STR) _T(".ini"));
static const std::filesystem::path PORTABLE_APPS = _T("PortableApps");
static const std::filesystem::path PORTABLE_APPS_APP = _T("App");
static const std::filesystem::path PORTABLE_APPS_DATA = _T("Data");
static const std::filesystem::path PORTABLE_APPS_DEFAULT_DATA = _T("App\\DefaultData");

static const std::filesystem::path PORTABLE_APPS_APP_LE_32 = PORTABLE_APPS_APP / _T("TCCLE32");
static const std::filesystem::path PORTABLE_APPS_APP_LE_64 = PORTABLE_APPS_APP / _T("TCCLE64");

static const std::filesystem::path TCC_EXE_LE_32 = _T("tcc.exe");
static const std::filesystem::path TCC_EXE_LE_64 = _T("tcc.exe");

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
static const std::tstring _ENV_PAPPS_LANGUAGE = _T("PortableApps.comLanguageName");

static const std::tstring _INI_4NT_NAME_LOCAL_HISTORY = _T("LocalHistory");
static const std::tstring _INI_4NT_NAME_LOCALDIR_HISTORY = _T("LocalDirHistory");
static const std::tstring _INI_4NT_NAME_TC_START_PATH = _T("TCStartPath");
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
static const std::tstring _INI_4NT_NAME_LANGUAGE_DLL = _T("LanguageDLL");
static const std::tstring _INI_NAME_FORCE32 = _T("Force32");
static const std::tstring _INI_NAME_WAIT = _T("Wait");

static const std::tstring _INI_NAME_COMMANDS = _T("Commands");
static const std::tstring _INI_NAME_OPTIONS = _T("Options");
static const std::tstring _INI_NAME_DIRECTIVES = _T("Directives");

static const std::tstring _INI_VALUE_TCC_INI = _T("tcc.ini");

static const std::tstring _INI_NAME_PROFILE_PORTABLE = _T("PortableProfile");
static const std::tstring _INI_NAME_PROFILE_USER = _T("USERPROFILE");
static const std::tstring _INI_NAME_PROFILE_ROAMING = _T("APPDATA");
static const std::tstring _INI_NAME_PROFILE_LOCAL = _T("LOCALAPPDATA");
static const std::tstring _INI_NAME_ENV_HOME_DRIVE = _T("HOMEDRIVE");
static const std::tstring _INI_NAME_ENV_HOME_PATH = _T("HOMEPATH");
static const std::tstring _INI_NAME_ENV_HOME_SHARE = _T("HOMESHARE");

static const p_apps::IniFile::iniDefaults defaults[] = {
		{_SECTION_LAUNCHER, _INI_NAME_SETTINGS_DIRECTORY, _T("settings")},
		{_SECTION_LAUNCHER, _INI_NAME_INI_FILE, _INI_VALUE_TCC_INI},
		{_SECTION_LAUNCHER, _INI_NAME_PROFILE_DIRECTORY, _T("profile")},
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

// ReSharper disable CppInconsistentNaming

class CommandItems : public std::vector<std::wstring> {
public:
	// ReSharper disable once CppMemberFunctionMayBeStatic
	void push_back_more() {
	}
};

class Commands : public CommandItems {
	static bool isSlashC(const std::wstring& val) {
		return boost::iequals(val, _T("/c")) || boost::iequals(val, _T("/k"));
	}

public:
	void push_back(const std::wstring& command);
	void push_back_more();
};

void Commands::push_back(const std::wstring& command) {
	if (this->empty()) {
		if (! isSlashC(command)) {
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

void Commands::push_back_more() {
	if (! this->empty()) {
		push_back(_T("&"));
	}
}

class Directives : public CommandItems {
public:
	void push_back(const std::wstring& directive);
	std::wstring toString(const std::wstring& key, const std::wstring& value) const;
	std::wstring toString(const std::wstring& key, const std::optional<std::wstring>& value) const;
	void set(const std::wstring& key, const std::wstring& value);
	void setOptional(const std::wstring& key, const std::optional<std::wstring>& value);
	bool exists(const std::wstring& key, const std::wstring& value);
};

void Directives::push_back(const std::wstring& directive) {
	const auto equalSignPosition = directive.find(_T("="));

	if (equalSignPosition != std::string::npos) {
		auto key = directive.substr(0, equalSignPosition);
		if (! boost::starts_with(key, _T("//"))) {
			key = _T("//") + key;
		}

		const auto value = p_apps::quote(directive.substr(equalSignPosition + 1, std::string::npos));

		std::vector<std::wstring>::push_back(key + _T("=") + value);
	}
}

std::wstring Directives::toString(const std::wstring& key, const std::wstring& value) const {
	return _T("//") + key + _T("=") + p_apps::quote(value);
}

std::wstring Directives::toString(const std::wstring& key, const std::optional<std::wstring>& value) const {
	return _T("//") + key + _T("=") + p_apps::quote(value);
}

void Directives::set(const std::wstring& key, const std::wstring& value) {
	std::vector<std::wstring>::push_back(toString(key, value));
}

void Directives::setOptional(const std::wstring& key, const std::optional<std::wstring>& value) {
	std::vector<std::wstring>::push_back(toString(key, value));
}

bool Directives::exists(const std::wstring& key, const std::wstring& value) {
	return std::find(begin(), end(), toString(key, value)) != end();
}

class Options : public CommandItems {
public:
	bool exists(const std::wstring& option);
	// std::find(options.begin(), options.end(), _T("/L:")) != options.end()
};

bool Options::exists(const std::wstring& option) {
	return std::find(begin(), end(), option) != end();
}

// ReSharper restore CppInconsistentNaming

class TccLauncher final : public Launcher {
	std::filesystem::path currentPath;
	std::optional<std::tstring> comspec = std::nullopt;
	std::optional<std::tstring> tccIniFilename = std::nullopt;
	std::optional<std::tstring> pCliIniFilename = std::nullopt;
	Directives directives;
	Options options;
	Commands commands;

	p_apps::IniFile pCliInifile;
	p_apps::IniFile tccIniFile;

	std::filesystem::path profileDirectory;
	std::filesystem::path profileDomainDirectory;
	std::filesystem::path profileRoamingDirectory;
	std::filesystem::path profileLocalDirectory;
	std::filesystem::path settingsDirectory;

	std::filesystem::path tccExeDirectory;
	std::filesystem::path tccExeName;

public:
	TccLauncher(const TccLauncher& other) = delete;
	TccLauncher(TccLauncher&& other) noexcept = delete;
	TccLauncher& operator=(const TccLauncher& other) = delete;
	TccLauncher& operator=(TccLauncher&& other) noexcept = delete;
	TccLauncher(int argc, TCHAR** argv, TCHAR** envp);
	~TccLauncher() override = default;

	void processIniSectionCommand();
	void logCommandLine() const;
	void processIniSectionProfile();

	void locateSettingsDirectory();

	void fixTccIniFilename();

	void copyDefaultData();

	void setupTempDirectory();
	void processHistoryFileHelper(const std::tstring& pCLiHistoryName, const std::tstring& pCLiLocalHistoryName, const std::tstring& commandOptionLocalHistory, const std::tstring& environmentHistoryFile);
	void processHistoryFile();
	void processDirHistoryFile();
	void processDirDrivesFile();
	void processLogs();
	[[nodiscard]] std::optional<std::tstring> locateKnownEditor() const;
	void processEditor();
	void selectTccExe();
	void selectLanguage();
	void copyMoreDirectives();
	void copyMoreEnvironment();
	[[nodiscard]] bool ifWaitForTccToFinish() const;
	void captureCurrentPath();
	void launch() override;

	void readProfileIniFiles();

	template <class T>
	bool processIniSectionCommandHelper(const std::tstring& iniName, T& consumer);

	[[nodiscard]] std::optional<std::filesystem::path> locateTccExe(const std::filesystem::path& argv0, const std::filesystem::path& tccExePath) const;
	void findHomeDirectory() override;
	void setupProfileDirectories();
	void readPCliInifile();
	static void usage();
	void parseCmdLine(bool& getHelp);
};

Launcher::Launcher(int argc, TCHAR** argv, TCHAR** envp)
	: argc(argc)
  , argv(argv)
  , envp(envp) {
	p_apps::imbueIO();
}

void TccLauncher::captureCurrentPath() {
	currentPath = std::filesystem::current_path();
}

void Launcher::getArgv0() {
	auto executableName = SysInfo::getExeName();
	argv0 = absolute(executableName
		                 ? executableName.value()
		                 : argv[0]);
	logger::trace(_T("[%s] argv0: %s"), _T(__FUNCTION__), argv0.c_str());
}


TccLauncher::TccLauncher(int argc, TCHAR** argv, TCHAR** envp) // NOLINT(clang-diagnostic-shadow-field)
	: Launcher(argc, argv, envp) {
}

std::optional<std::filesystem::path> TccLauncher::locateTccExe(const std::filesystem::path& argv0, const std::filesystem::path& tccExePath) const {
	// NOLINT(clang-diagnostic-shadow-field)

	std::filesystem::path possibleExeLocations[] = {
			PORTABLE_APPS / argv0.stem(),
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
			logger::trace(_T("[%s] pApps directory: %s"), _T(__FUNCTION__), pAppsDir.c_str());
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
			commands.push_back(p_apps::quote(argument));
			continue;
		}

		if (! pCliIniFilename && boost::starts_with(argument, _T("@@"))) {
			pCliIniFilename = p_apps::unquote(std::tstring(argument.begin() + 2, argument.end()));
			continue;
		}

		if (! tccIniFilename && boost::starts_with(argument, _T("/@"))) {
			tccIniFilename = p_apps::unquote(std::tstring(argument.begin() + 2, argument.end()));
			continue;
		}

		if (! tccIniFilename && boost::starts_with(argument, _T("@"))) {
			tccIniFilename = p_apps::unquote(std::tstring(argument.begin() + 1, argument.end()));
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
			options.push_back(argument);
			continue;
		}

		if (! comspec && std::filesystem::is_directory(p_apps::unquote(argument))) {
			comspec = argument;
			continue;
		}

		mode_command = true;
		commands.push_back(p_apps::quote(argument));
	}

	if (tccIniFilename) {
		logger::trace(_T("[%s] inifile: %s"), _T(__FUNCTION__), tccIniFilename.value().c_str());
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

void TccLauncher::readPCliInifile() {
	pCliInifile.setDefaults(defaults, _countof(defaults));

	if (! pCliIniFilename) {
		pCliIniFilename = pAppsDir / LAUNCHER_INI;
	}
	logger::trace(_T("[%s] using INI file: %s"), _T(__FUNCTION__), pCliIniFilename.value().c_str());

	pCliInifile.readIniFile(pCliIniFilename.value(), false);
}

void TccLauncher::setupProfileDirectories() {
	// FIXME another profile should be supported by tcstart.btm
	profileDirectory = p_apps::canonical(std::filesystem::path(p_apps::Environment::expandEnv(pCliInifile.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_PROFILE_DIRECTORY))),
	                                     pAppsDir / PORTABLE_APPS_DATA);
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
		pCliInifile.readIniFile(iniLocation / LAUNCHER_INI, false);
	}
}

template <typename T>
bool TccLauncher::processIniSectionCommandHelper(const std::tstring& iniName, T& consumer) {
	if (auto iniValue = pCliInifile.getValue(_SECTION_COMMAND, iniName)) {
		const auto tokens = p_apps::tokenize(iniValue.value());
		if (! tokens.empty()) {
			consumer.push_back_more();
		}
		for (const auto& token : tokens) {
			consumer.push_back(token);
		}
		logger::trace(_T("[%s] %s: %s"), _T(__FUNCTION__), iniName, iniValue.value().c_str());
		return true;
	}
	return false;
}

void TccLauncher::processIniSectionCommand() {
	auto done = false;
	done |= processIniSectionCommandHelper(_INI_NAME_COMMANDS, commands);
	done |= processIniSectionCommandHelper(_INI_NAME_OPTIONS, options);
	done |= processIniSectionCommandHelper(_INI_NAME_DIRECTIVES, directives);
	if (done) {
		logCommandLine();
	}
}

void TccLauncher::logCommandLine() const {
	if (logger::isTrace()) {
		if (!directives.empty()) {
			logger::trace(_T("[%s] directives: %s"), _T(__FUNCTION__), boost::algorithm::join(directives, _T(" ")));
		}
		if (!options.empty()) {
			logger::trace(_T("[%s] options: %s"), _T(__FUNCTION__), boost::algorithm::join(options, _T(" ")));
		}
		if (!commands.empty()) {
			logger::trace(_T("[%s] commands: %s"), _T(__FUNCTION__), boost::algorithm::join(commands, _T(" ")));
		}
	}
}

void TccLauncher::processIniSectionProfile() {
	if (yesNoOption(pCliInifile.getValue(_SECTION_PROFILE, _INI_NAME_PROFILE_PORTABLE))) {

		static const std::tstring iniProfileNames[] = {
				_INI_NAME_PROFILE_USER,
				_INI_NAME_PROFILE_USER,
				_INI_NAME_PROFILE_LOCAL
			};

		for (const auto& profileName : iniProfileNames) {

			auto profileSettings = pCliInifile.getValue(_SECTION_PROFILE, profileName);
			if (! profileSettings) {
				continue;
			}
			auto profilePath = p_apps::canonical(p_apps::Environment::expandEnv(profileSettings.value()), profileDirectory);

			p_apps::makeDirWriteable(profilePath);

			environment.set(profileName, profilePath._tstring(), true);

			if (profileName == _INI_NAME_PROFILE_USER) {
				environment.set(_INI_NAME_ENV_HOME_PATH, (profilePath.root_directory() / profilePath.relative_path())._tstring(), true);
				environment.set(_INI_NAME_ENV_HOME_DRIVE, profilePath.root_name()._tstring(), true);
				environment.set(_INI_NAME_ENV_HOME_SHARE, p_apps::pathToUnc(profilePath.root_name()._tstring()), true);
			}

			logger::trace(_T("[%s] profile: %s in %s"), _T(__FUNCTION__), profileName, profilePath._tstring());
		}
	}
}

void TccLauncher::locateSettingsDirectory() {
	settingsDirectory = p_apps::canonical(
		std::filesystem::path(
			p_apps::Environment::expandEnv(
				pCliInifile.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_SETTINGS_DIRECTORY))),
		pAppsDir / PORTABLE_APPS_DATA);
	logger::trace(_T("[%s] settings directory: %s"), _T(__FUNCTION__), settingsDirectory.c_str());
}

void TccLauncher::fixTccIniFilename() {
	if (! tccIniFilename) {
		tccIniFilename = p_apps::Environment::expandEnv(pCliInifile.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_INI_FILE));
	}

	tccIniFilename = p_apps::canonical(tccIniFilename.value(), settingsDirectory);
	logger::trace(_T("[%s] effective tcc.ini location: %s"), _T(__FUNCTION__), tccIniFilename.value());
}

void TccLauncher::copyDefaultData() {

	if (! is_directory(settingsDirectory)) {
		const auto sourceDirectory = p_apps::canonical(PORTABLE_APPS_DEFAULT_DATA, pAppsDir);
		std::error_code errorCode;
		std::filesystem::copy(sourceDirectory, settingsDirectory, errorCode);
		if (errorCode) {
			fail(_T("[%s] Cann't copy directory\nfrom %s to %s\nReason: %s"), _T(__FUNCTION__), sourceDirectory._tstring(), settingsDirectory._tstring(), p_apps::string2wstring(errorCode.message()));
		}
	}

	// iniFileName can be passed as an argument or retrieved from pCli.ini
	if (! std::filesystem::exists(tccIniFilename.value())) {
		const auto sourceFile = p_apps::canonical(PORTABLE_APPS_DEFAULT_DATA / _INI_VALUE_TCC_INI, pAppsDir);

		std::error_code errorCode;
		std::filesystem::copy(sourceFile, tccIniFilename.value(), errorCode);
		if (errorCode) {
			fail(_T("[%s] Cann't copy file\nfrom %s to %s\nReason: %s"), _T(__FUNCTION__), sourceFile._tstring(), tccIniFilename.value(), p_apps::string2wstring(errorCode.message()));
		}
	}
}

void TccLauncher::setupTempDirectory() {
	/*******************************************************
*	Temp directory
*	  Since PortableApps 10.0.2, if there is directory TempForPortableApps in the root of portable apps, it is used as TEMP
*******************************************************/

	std::optional<std::filesystem::path> tempPath;

	if (auto tempPathName = pCliInifile.getValue(_SECTION_LAUNCHER, _INI_NAME_TEMP_DIRECTORY)) {
		tempPath = p_apps::canonical(std::filesystem::path(p_apps::Environment::expandEnv(tempPathName.value())), pAppsDir / PORTABLE_APPS_DATA);
	}

	if (! tempPath) {
		auto TempForPortableApps = p_apps::canonical(pAppsDir / _T("..\\..\\TempForPortableApps"), pAppsDir);
		if (is_directory(TempForPortableApps)) {
			tempPath = TempForPortableApps;
		}
	}

	if (!tempPath) {
		auto systemTempPathName = p_apps::canonical(environment.get(_ENV_TEMP), _T(""));
		if (is_directory(systemTempPathName)) {
			tempPath = systemTempPathName;
		}
	}

	if (tempPath) {
		p_apps::makeDirWriteable(tempPath.value());
		environment.set(_ENV_TEMP, tempPath.value().c_str());
		environment.set(_ENV_TMP, tempPath.value().c_str());
		logger::trace(_T("[%s] TEMP directory: %s"), _T(__FUNCTION__), tempPath.value().c_str());
	} else {
		logger::warning(_T("[%s] unable to locate TEMP"), _T(__FUNCTION__));
	}
}

void TccLauncher::processHistoryFileHelper(const std::tstring& pCLiHistoryName, const std::tstring& pCLiLocalHistoryName, const std::tstring& commandOptionLocalHistory, const std::tstring& environmentHistoryFile) {

	//	History. DirHistory, etc
	//	Workaround for bug: http://jpsoft.com/forums/threads/lost-history.5878/
	//	  If History / DirHistory is local, it is handled by TCC.exe
	//	  If History / DirHistory is global, it is handled by tcStart

	const auto iniValue = p_apps::Environment::expandEnv(pCliInifile.getValueNonEmpty(_SECTION_LAUNCHER, pCLiHistoryName));
	auto isLocal =
		yesNoOption(tccIniFile.getValue(_SECTION_4NT, pCLiLocalHistoryName))
		|| options.exists(_T("/L:"))
		|| options.exists(commandOptionLocalHistory)
		|| directives.exists(commandOptionLocalHistory,_T("Yes"));

	if (isLocal) {
		directives.set(pCLiHistoryName, profileLocalDirectory / iniValue);
		logger::trace(_T("[%s] processing local %s"), _T(__FUNCTION__), pCLiHistoryName);
	} else {
		directives.set(pCLiHistoryName, _T(""));
		environment.set(environmentHistoryFile, profileLocalDirectory / iniValue);
		logger::trace(_T("[%s] processing global %s"), _T(__FUNCTION__), pCLiHistoryName);
	}
}

void TccLauncher::processHistoryFile() {
	processHistoryFileHelper(_INI_NAME_HISTORY_FILE, _INI_4NT_NAME_LOCAL_HISTORY, L"/LH", _ENV_HIST_FILE);
}

void TccLauncher::processDirHistoryFile() {
	processHistoryFileHelper(_INI_NAME_DIRHISTORY_FILE, _INI_4NT_NAME_LOCALDIR_HISTORY, L"/LD", _ENV_DIRHIST_FILE);
}

void TccLauncher::processDirDrivesFile() {
	environment.set(_ENV_DIRDRIVES_FILE, p_apps::canonical(p_apps::Environment::expandEnv(pCliInifile.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_DIRDRIVES_FILE)), profileLocalDirectory));
}

void TccLauncher::processLogs() {
	auto logsDirectory = p_apps::canonical(p_apps::Environment::expandEnv(pCliInifile.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_LOGS_DIRECTORY)), pAppsDir / PORTABLE_APPS_DATA);
	p_apps::makeDirWriteable(logsDirectory);
	logger::trace(_T("[%s] logs directory %s"), _T(__FUNCTION__), logsDirectory.c_str());

	auto commandLogFile = p_apps::Environment::expandEnv(pCliInifile.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_COMMANDLOG_FILE));
	directives.set(_INI_NAME_COMMANDLOG_FILE, logsDirectory / commandLogFile);
	logger::trace(_T("[%s] command log file: %s"), _T(__FUNCTION__), commandLogFile);

	auto errorLogFile = p_apps::Environment::expandEnv(pCliInifile.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_ERROSLOG_FILE));
	directives.set(_INI_NAME_ERROSLOG_FILE, logsDirectory / errorLogFile);
	logger::trace(_T("[%s] error log file: %s"), _T(__FUNCTION__), errorLogFile);

	auto historyLogFile = p_apps::Environment::expandEnv(pCliInifile.getValueNonEmpty(_SECTION_LAUNCHER, _INI_NAME_HISTORYLOG_FILE));
	directives.set(_INI_NAME_HISTORYLOG_FILE, logsDirectory / historyLogFile);
	logger::trace(_T("[%s] history log file: %s"), _T(__FUNCTION__), historyLogFile);
}

std::optional<std::tstring> TccLauncher::locateKnownEditor() const {
	for (const auto& proposedEditor : knownEditors) {
		if (exists(pAppsDir / _T("..") / proposedEditor)) {
			return proposedEditor;
		}
	}
	return std::nullopt;
}

void TccLauncher::processEditor() {

	const auto externalEditorInit = tccIniFile.getValue(_SECTION_4NT, _INI_NAME_EDITOR);
	auto externalEditor = externalEditorInit;

	if (!externalEditor) {
		externalEditor = pCliInifile.getValue(_SECTION_LAUNCHER, _INI_NAME_EDITOR);
	}

	if (externalEditor) {
		externalEditor = p_apps::Environment::expandEnv(externalEditor.value());
	}

	if (externalEditor && externalEditor.value().empty()) {
		externalEditor = std::nullopt;
	}

	if (!externalEditor) {
		externalEditor = locateKnownEditor();
	}

	if (!externalEditor) {
		externalEditor = _T("notepad.exe");
	}

	if (externalEditor) {
		auto externalEditorFullPath = p_apps::canonical(externalEditor.value(), pAppsDir / _T(".."));
		if (exists(externalEditorFullPath)) {
			externalEditor = externalEditorFullPath;
		}
	}

	if (externalEditorInit != externalEditor) {
		directives.setOptional(_INI_NAME_EDITOR, externalEditor);
		logger::trace(_T("[%s] external editor: %s"), _T(__FUNCTION__), externalEditor.value().c_str());
	}
}

void TccLauncher::selectTccExe() {
	const auto runX64 =
		SysInfo::isWow64()
		&& ! yesNoOption(pCliInifile.getValue(_SECTION_LAUNCHER, _INI_NAME_FORCE32))
		&& exists(pAppsDir / PORTABLE_APPS_APP_LE_64 / TCC_EXE_LE_64);

	tccExeDirectory = p_apps::canonical(runX64
		                                    ? PORTABLE_APPS_APP_LE_64
		                                    : PORTABLE_APPS_APP_LE_32,
	                                    pAppsDir);
	tccExeName = runX64
		             ? TCC_EXE_LE_64
		             : TCC_EXE_LE_32;

	const auto runner = tccExeDirectory / tccExeName;
	if (is_regular_file(runner)) {
		logger::trace(_T("[%s] exe: %s"), _T(__FUNCTION__), runner.c_str());
	} else {
		fail(_T("[%s] expected tcc.exe not found: %s"), _T(__FUNCTION__), runner.c_str());
	}
}

void TccLauncher::selectLanguage() {
	const auto languageDllInit = tccIniFile.getValue(_SECTION_4NT, _INI_4NT_NAME_LANGUAGE_DLL);

	auto languageDll = languageDllInit;

	if (!languageDll) {
		languageDll = pCliInifile.getValue(_SECTION_LAUNCHER, _INI_4NT_NAME_LANGUAGE_DLL);
	}

	if (!languageDll && environment.exists(_ENV_PAPPS_LANGUAGE)) {
		languageDll = environment.get(_ENV_PAPPS_LANGUAGE) + _T(".dll");
	}

	if (languageDll) {
		auto zz = languageDll.value();
	}

	if (languageDll && !exists(tccExeDirectory / languageDll.value())) {
		languageDll = std::nullopt;
	}

	if (languageDllInit != languageDll) {
		directives.setOptional(_INI_4NT_NAME_LANGUAGE_DLL, languageDll);
		logger::trace(_T("[%s] using LanguageDLL: %s"), _T(__FUNCTION__), languageDll.value().c_str());
	}
}

void TccLauncher::copyMoreDirectives() {
	// ReSharper disable once CppInconsistentNaming
	p_apps::SetInsensitiveTChar names4NT;
	pCliInifile.enumNames(_SECTION_4NT, names4NT);

	for (const auto& key : names4NT) {
		auto value = pCliInifile.getValue(_SECTION_4NT, key);

		if (value) {
			value = p_apps::Environment::expandEnv(value.value());
		}

		directives.setOptional(key, value);
	}
	logger::trace(_T("[%s] more tcc.ini %s entries: %d values"), _T(__FUNCTION__), _SECTION_4NT, names4NT.size());
}

void TccLauncher::copyMoreEnvironment() {
	p_apps::SetInsensitiveTChar namesEnv;
	pCliInifile.enumNames(_SECTION_ENVIRONMENT, namesEnv);
	auto erasedCounter = 0;

	for (const auto& key : namesEnv) {
		auto value = pCliInifile.getValue(_SECTION_ENVIRONMENT, key);

		if (value) {
			value = p_apps::Environment::expandEnv(value.value());
		}

		if (value && ! value.value().empty()) {
			environment.set(key, p_apps::Environment::expandEnv(value.value()));
		} else {
			environment.erase(key);
			erasedCounter += 1;
		}
	}

	logger::trace(_T("[%s] more tcc.ini %s entries: %d values, %d erased"), _T(__FUNCTION__), _SECTION_4NT, namesEnv.size(), erasedCounter);
}

bool TccLauncher::ifWaitForTccToFinish() const {


	auto waitSettings = pCliInifile.getValue(_SECTION_LAUNCHER, _INI_NAME_WAIT);
	if (waitSettings) {
		return yesNoOption(waitSettings.value());
	}

	if (!SysInfo::ownsConsole()) {
		logger::trace(_T("[%s] ownsConsole() == false"), _T(__FUNCTION__));
		return true;
	}

	if (SysInfo::getDllName(_T("ConEmuHk.dll")) || SysInfo::getDllName(_T("ConEmuHk64.dll"))) {
		logger::trace(_T("[%s] conEmu detected"), _T(__FUNCTION__));
		return true;
	}

	return false;
}

void TccLauncher::launch() {
	logger::trace(_T("[%s] Start"), _T(__FUNCTION__));

	environment.setUp(envp);
	environment.setNameBackup(_ENV_BACKUP_PREFIX);

	captureCurrentPath();

	getArgv0();

	findHomeDirectory();

	auto showUsage = false;
	parseCmdLine(showUsage);
	if (showUsage) {
		usage();
		exit(0); // NOLINT(concurrency-mt-unsafe)
	}

	readPCliInifile();

	setupProfileDirectories();

	readProfileIniFiles();

	processIniSectionCommand();

	processIniSectionProfile();

	locateSettingsDirectory();

	fixTccIniFilename();

	copyDefaultData();

	tccIniFile.readIniFile(tccIniFilename.value(), true);

	directives.set(_INI_4NT_NAME_TC_START_PATH, settingsDirectory);

	directives.set(_INI_4NT_NAME_TREE_PATH, profileLocalDirectory);

	setupTempDirectory();

	processHistoryFile();
	processDirHistoryFile();

	processDirDrivesFile();

	processLogs();

	processEditor();

	selectTccExe();

	selectLanguage();

	copyMoreDirectives();

	copyMoreEnvironment();

	/*******************************************************
	*	Wait for tcc to finish
	*******************************************************/

	auto pWait = ifWaitForTccToFinish();

	/*******************************************************
	*	Ready to execute
	*******************************************************/
	std::vector<std::tstring> effectiveArgv;
	const auto effectiveArgv0 = tccExeDirectory / tccExeName;
	effectiveArgv.push_back(p_apps::quote(effectiveArgv0));
	// commandComspec: ignored	
	effectiveArgv.push_back(std::tstring(_T("@")) + p_apps::quote(tccIniFilename.value()));
	p_apps::appendContainer(effectiveArgv, directives);
	p_apps::appendContainer(effectiveArgv, options);
	p_apps::appendContainer(effectiveArgv, commands);
	if (logger::isTrace())
		logger::trace(_T("[%s] finally argv:\n%s"), _T(__FUNCTION__), boost::algorithm::join(effectiveArgv, _T(" ")));
	execute(pWait, effectiveArgv0, effectiveArgv, environment, currentPath);
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
