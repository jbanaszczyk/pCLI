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

static const std::filesystem::path LAUNCHER_INI(_T(VER_PRODUCTNAME_STR) L".ini");
static const std::filesystem::path PORTABLE_APPS = L"PortableApps";
static const std::filesystem::path PORTABLE_APPS_APP = L"App";
static const std::filesystem::path PORTABLE_APPS_DATA = L"Data";
static const std::filesystem::path PORTABLE_APPS_DEFAULT_DATA = L"App\\DefaultData";

static const std::filesystem::path PORTABLE_APPS_APP_LE_32 = PORTABLE_APPS_APP / L"TCCLE32";
static const std::filesystem::path PORTABLE_APPS_APP_LE_64 = PORTABLE_APPS_APP / L"TCCLE64";

static const std::filesystem::path TCC_EXE_LE_32 = L"tcc.exe";
static const std::filesystem::path TCC_EXE_LE_64 = L"tcc.exe";

static const auto SECTION_COMMAND = L"Command";
static const auto SECTION_ENVIRONMENT = L"Environment";
static const auto SECTION_LAUNCHER = L"pCli";
static const auto SECTION_PROFILE = L"Profile";

static const auto SECTION_4NT = L"4NT";

static const auto ENV_BACKUP_PREFIX = L"$TCC$";
static const auto ENV_LOCAL_ALIASES = L"$TCC$LocalAliases";
static const auto ENV_LOCAL_DIRHISTORY = L"$TCC$LocalDirHistory";
static const auto ENV_LOCAL_HISTORY = L"$TCC$LocalHistory";
static const auto ENV_TEMP = L"TEMP";
static const auto ENV_TMP = L"TMP";
static const auto ENV_PAPPS_LANGUAGE = L"PortableApps.comLanguageName";

static const auto INI_4NT_NAME_LOCAL_ALIASES = L"LocalAliases";
static const auto INI_4NT_NAME_LOCAL_HISTORY = L"LocalHistory";
static const auto INI_4NT_NAME_LOCALDIR_HISTORY = L"LocalDirHistory";
static const auto INI_4NT_NAME_TC_START_PATH = L"TCStartPath";
static const auto INI_4NT_NAME_TREE_PATH = L"TreePath";

static const auto INI_NAME_TEMP_DIRECTORY = L"TempDirectory";
static const auto INI_NAME_PROFILE_DIRECTORY = L"ProfileDirectory";
static const auto INI_NAME_SETTINGS_DIRECTORY = L"SettingsDirectory";

static const auto INI_NAME_LOGS_DIRECTORY = L"LogsDirectory";
static const auto INI_NAME_COMMAND_LOG_FILE = L"LogName";
static const auto INI_NAME_ERRORS_LOG_FILE = L"LogErrorsName";
static const auto INI_NAME_HISTORY_LOG_FILE = L"HistLogName";

static const auto INI_NAME_INI_FILE = L"TccIniFile";
static const auto INI_NAME_EDITOR = L"Editor";
static const auto INI_4NT_NAME_LANGUAGE_DLL = L"LanguageDLL";
static const auto INI_NAME_FORCE32 = L"Force32";
static const auto INI_NAME_WAIT = L"Wait";

static const auto INI_NAME_COMMANDS = L"Commands";
static const auto INI_NAME_OPTIONS = L"Options";
static const auto INI_NAME_DIRECTIVES = L"Directives";

static const auto INI_VALUE_TCC_INI = L"tcc.ini";

static const auto INI_NAME_PROFILE_USER = L"USERPROFILE";
static const auto INI_NAME_ENV_HOME_DRIVE = L"HOMEDRIVE";
static const auto INI_NAME_ENV_HOME_PATH = L"HOMEPATH";
static const auto INI_NAME_ENV_HOME_SHARE = L"HOMESHARE";

static const p_apps::IniFile::iniDefaults defaults[] = {
		{SECTION_LAUNCHER, INI_NAME_SETTINGS_DIRECTORY, L"settings"},
		{SECTION_LAUNCHER, INI_NAME_INI_FILE, INI_VALUE_TCC_INI},
		{SECTION_LAUNCHER, INI_NAME_LOGS_DIRECTORY, L"logs"},
		{SECTION_LAUNCHER, INI_NAME_COMMAND_LOG_FILE, L"command.log"},
		{SECTION_LAUNCHER, INI_NAME_ERRORS_LOG_FILE, L"errors.log"},
		{SECTION_LAUNCHER, INI_NAME_HISTORY_LOG_FILE, L"history.log"},
		{SECTION_LAUNCHER, INI_NAME_FORCE32, L"false"},
		{SECTION_LAUNCHER, INI_NAME_PROFILE_DIRECTORY, L"profile"},
		{SECTION_COMMAND, INI_NAME_OPTIONS, L"/D"},
	};

static const std::wstring knownEditors[] = {
		L"UltraEdit Portable\\UltraEditPortable.exe",
		L"AkelPadPortable\\AkelPadPortable.exe",
		L"GeanyPortable\\GeanyPortable.exe",
		L"Notepad++Portable\\Notepad++Portable.exe",
		L"Notepad2Portable\\Notepad2Portable.exe",
		L"Notepad2-modPortable\\Notepad2-modPortable.exe",
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
	wchar_t** argv;
	wchar_t** envp;
	p_apps::Environment environment{};
	std::filesystem::path argv0{};
	std::filesystem::path pAppsDir{};
	std::filesystem::path iniFileName{LAUNCHER_INI};

	Launcher(const Launcher& other) = delete;
	Launcher(Launcher&& other) noexcept = delete;
	Launcher& operator=(const Launcher& other) = delete;
	Launcher& operator=(Launcher&& other) noexcept = delete;
	Launcher(int argc, wchar_t** argv, wchar_t** envp);
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
		return boost::iequals(val, L"/c") || boost::iequals(val, L"/k");
	}

public:
	void push_back(const std::wstring& command);
	void push_back_more();
};

void Commands::push_back(const std::wstring& command) {
	if (this->empty()) {
		if (! isSlashC(command)) {
			push_back(std::wstring{L"/k"});
		}
	} else {
		if (isSlashC(command)) {
			push_back(std::wstring{L"&"});
			return;
		}
	}
	std::vector<std::wstring>::push_back(p_apps::quote(command));
}

void Commands::push_back_more() {
	if (! this->empty()) {
		push_back(L"&");
	}
}

class Directives : public CommandItems {
public:
	void push_back(const std::wstring& directive);
	[[nodiscard]] std::wstring toString(const std::wstring& key, const std::wstring& value) const;
	[[nodiscard]] std::wstring toString(const std::wstring& key, const std::optional<std::wstring>& value) const;
	void set(const std::wstring& key, const std::wstring& value);
	void setOptional(const std::wstring& key, const std::optional<std::wstring>& value);
	bool exists(const std::wstring& key, const std::wstring& value);
};

void Directives::push_back(const std::wstring& directive) {
	const auto equalSignPosition = directive.find(L"=");

	if (equalSignPosition != std::string::npos) {
		auto key = directive.substr(0, equalSignPosition);
		if (! boost::starts_with(key, L"//")) {
			key = L"//" + key;
		}

		const auto value = p_apps::quote(directive.substr(equalSignPosition + 1, std::string::npos));
		// const auto value = directive.substr(equalSignPosition + 1, std::string::npos);

		std::vector<std::wstring>::push_back(key + L"=" + value);
	}
}

std::wstring Directives::toString(const std::wstring& key, const std::wstring& value) const {
	return L"//" + key + L"=" + p_apps::quote(value);
}

std::wstring Directives::toString(const std::wstring& key, const std::optional<std::wstring>& value) const {
	return L"//" + key + L"=" + p_apps::quote(value);
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
};

bool Options::exists(const std::wstring& option) {
	return std::find(begin(), end(), option) != end();
}

// ReSharper restore CppInconsistentNaming

class TccLauncher final : public Launcher {
	std::filesystem::path currentPath;
	std::optional<std::wstring> comspec = std::nullopt;
	std::optional<std::wstring> tccIniFilename = std::nullopt;
	std::optional<std::wstring> pCliIniFilename = std::nullopt;
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
	TccLauncher(int argc, wchar_t** argv, wchar_t** envp);
	~TccLauncher() override = default;

	void processIniSectionCommand();
	void logCommandLine() const;
	void processIniSectionProfile();

	void locateSettingsDirectory();

	void fixTccIniFilename();

	void copyDefaultData();

	void setupTempDirectory();
	void processLocalListsFileHelper(const std::wstring& pCLiLocalHistoryName, const std::wstring& commandOptionLocalHistory, const std::wstring& environmentHistoryFile);
	void processLocalLists();
	void processLogs();
	[[nodiscard]] std::optional<std::wstring> locateKnownEditor() const;
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
	bool processIniSectionCommandHelper(const std::wstring& iniName, T& consumer);

	[[nodiscard]] std::optional<std::filesystem::path> locateTccExe(const std::filesystem::path& argv0, const std::filesystem::path& tccExePath) const;
	void findHomeDirectory() override;
	void setupProfileDirectories();
	void readPCliInifile();
	static void usage();
	void parseCmdLine(bool& getHelp);
};

Launcher::Launcher(int argc, wchar_t** argv, wchar_t** envp)
	: argc(argc)
  , argv(argv)
  , envp(envp) {
	p_apps::imbueIO();
}

void TccLauncher::captureCurrentPath() {
	currentPath = std::filesystem::current_path();
}

void Launcher::getArgv0() {
	const auto executableName = SysInfo::getExeName();
	argv0 = absolute(executableName
		                 ? executableName.value()
		                 : argv[0]);
	logger::trace(L"[%s] argv0: %s", _T(__FUNCTION__), argv0.c_str());
}


TccLauncher::TccLauncher(int argc, wchar_t** argv, wchar_t** envp) // NOLINT(clang-diagnostic-shadow-field)
	: Launcher(argc, argv, envp) {
}

std::optional<std::filesystem::path> TccLauncher::locateTccExe(const std::filesystem::path& argv0, const std::filesystem::path& tccExePath) const {

	std::filesystem::path possibleExeLocations[] = {
			PORTABLE_APPS / argv0.stem(),
			L".\\",
			L"..\\",
			L"..\\..\\..\\..\\..\\..\\..\\..\\",
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
			logger::trace(L"[%s] pApps directory: %s", _T(__FUNCTION__), pAppsDir.c_str());
			return;
		}
	}

	fail(L"Unable to find tcc.exe from %s", argv0.parent_path());
}

void TccLauncher::parseCmdLine(bool& getHelp) {
	++argv; // skip argv[0]

	getHelp = false;
	bool mode_command = false;

	for (auto argumentIndex = 1; argumentIndex < argc; ++argumentIndex, ++argv) {
		std::wstring argument(*argv);

		if (argument == L"/?") {
			getHelp = true;
			continue;
		}

		if (boost::iequals(argument, L"/c") || boost::iequals(argument, L"/k")) {
			mode_command = true;
		}

		if (mode_command) {
			// everything after /C or /K is a command
			commands.push_back(p_apps::quote(argument));
			continue;
		}

		if (! pCliIniFilename && boost::starts_with(argument, L"@@")) {
			pCliIniFilename = p_apps::unquote(std::wstring(argument.begin() + 2, argument.end()));
			continue;
		}

		if (! tccIniFilename && boost::starts_with(argument, L"/@")) {
			tccIniFilename = p_apps::unquote(std::wstring(argument.begin() + 2, argument.end()));
			continue;
		}

		if (! tccIniFilename && boost::starts_with(argument, L"@")) {
			tccIniFilename = p_apps::unquote(std::wstring(argument.begin() + 1, argument.end()));
			continue;
		}

		if (boost::starts_with(argument, L"//")) {
			directives.push_back(argument);
			continue;
		}

		if (boost::starts_with(argument, L"/")) {
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
		logger::trace(L"[%s] inifile: %s", _T(__FUNCTION__), tccIniFilename.value().c_str());
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
	logger::trace(L"[%s] using INI file: %s", _T(__FUNCTION__), pCliIniFilename.value().c_str());

	pCliInifile.readIniFile(pCliIniFilename.value());
}

void TccLauncher::setupProfileDirectories() {
	// FIXME another profile should be supported by tcstart.btm
	profileDirectory = p_apps::canonical(std::filesystem::path(p_apps::Environment::expandEnv(pCliInifile.getValueNonEmpty(SECTION_LAUNCHER, INI_NAME_PROFILE_DIRECTORY))),
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
		pCliInifile.readIniFile(iniLocation / LAUNCHER_INI);
	}
}

template <typename T>
bool TccLauncher::processIniSectionCommandHelper(const std::wstring& iniName, T& consumer) {
	if (const auto iniValue = pCliInifile.getValue(SECTION_COMMAND, iniName)) {
		if (iniValue) {
			auto zz = iniValue.value();
		}
		const auto tokens = p_apps::tokenize(iniValue.value());
		if (! tokens.empty()) {
			consumer.push_back_more();
		}
		for (const auto& token : tokens) {
			consumer.push_back(token);
		}
		logger::trace(L"[%s] %s: %s", _T(__FUNCTION__), iniName, iniValue.value().c_str());
		return true;
	}
	return false;
}

void TccLauncher::processIniSectionCommand() {
	auto done = false;
	done |= processIniSectionCommandHelper(INI_NAME_COMMANDS, commands);
	done |= processIniSectionCommandHelper(INI_NAME_OPTIONS, options);
	done |= processIniSectionCommandHelper(INI_NAME_DIRECTIVES, directives);
	if (done) {
		logCommandLine();
	}
}

void TccLauncher::logCommandLine() const {
	if (logger::isTrace()) {
		if (!directives.empty()) {
			logger::trace(L"[%s] directives: %s", _T(__FUNCTION__), boost::algorithm::join(directives, L" "));
		}
		if (!options.empty()) {
			logger::trace(L"[%s] options: %s", _T(__FUNCTION__), boost::algorithm::join(options, L" "));
		}
		if (!commands.empty()) {
			logger::trace(L"[%s] commands: %s", _T(__FUNCTION__), boost::algorithm::join(commands, L" "));
		}
	}
}

void TccLauncher::processIniSectionProfile() {
	p_apps::SetInsensitiveTChar names4NT;
	pCliInifile.enumNames(SECTION_PROFILE, names4NT);

	for (const auto& key : names4NT) {
		auto value = p_apps::Environment::expandEnv(pCliInifile.getValueNonEmpty(SECTION_PROFILE, key));
		auto profilePath = p_apps::canonical(p_apps::Environment::expandEnv(value), profileDirectory);

		if (value.back() == std::filesystem::path::preferred_separator) {
			p_apps::makeDirWriteable(profilePath);
		}

		environment.set(key, profilePath.wstring(), true);
		logger::trace(L"[%s] profile: %s = %s", _T(__FUNCTION__), key, profilePath.c_str());

		if (key == INI_NAME_PROFILE_USER) {
			environment.set(INI_NAME_ENV_HOME_PATH, (profilePath.root_directory() / profilePath.relative_path()).wstring(), true);
			environment.set(INI_NAME_ENV_HOME_DRIVE, profilePath.root_name().wstring(), true);
			environment.set(INI_NAME_ENV_HOME_SHARE, p_apps::pathToUnc(profilePath.root_name().wstring()), true);
		}
	}
}

void TccLauncher::locateSettingsDirectory() {
	settingsDirectory = p_apps::canonical(
		std::filesystem::path(
			p_apps::Environment::expandEnv(
				pCliInifile.getValueNonEmpty(SECTION_LAUNCHER, INI_NAME_SETTINGS_DIRECTORY))),
		pAppsDir / PORTABLE_APPS_DATA);
	logger::trace(L"[%s] settings directory: %s", _T(__FUNCTION__), settingsDirectory.c_str());
}

void TccLauncher::fixTccIniFilename() {
	if (! tccIniFilename) {
		tccIniFilename = p_apps::Environment::expandEnv(pCliInifile.getValueNonEmpty(SECTION_LAUNCHER, INI_NAME_INI_FILE));
	}

	tccIniFilename = p_apps::canonical(tccIniFilename.value(), settingsDirectory);
	logger::trace(L"[%s] effective tcc.ini location: %s", _T(__FUNCTION__), tccIniFilename.value());
}

void TccLauncher::copyDefaultData() {

	if (! is_directory(settingsDirectory)) {
		const auto sourceDirectory = p_apps::canonical(PORTABLE_APPS_DEFAULT_DATA, pAppsDir);
		std::error_code errorCode;
		std::filesystem::copy(sourceDirectory, settingsDirectory, errorCode);
		if (errorCode) {
			fail(L"[%s] Cannot copy directory\nfrom %s to %s\nReason: %s", _T(__FUNCTION__), sourceDirectory.wstring(), settingsDirectory.wstring(), p_apps::string2wstring(errorCode.message()));
		}
	}

	// iniFileName can be passed as an argument or retrieved from pCli.ini
	if (! std::filesystem::exists(tccIniFilename.value())) {
		const auto sourceFile = p_apps::canonical(PORTABLE_APPS_DEFAULT_DATA / INI_VALUE_TCC_INI, pAppsDir);

		std::error_code errorCode;
		std::filesystem::copy(sourceFile, tccIniFilename.value(), errorCode);
		if (errorCode) {
			fail(L"[%s] Cann't copy file\nfrom %s to %s\nReason: %s", _T(__FUNCTION__), sourceFile.wstring(), tccIniFilename.value(), p_apps::string2wstring(errorCode.message()));
		}
	}
}

void TccLauncher::setupTempDirectory() {
	/*******************************************************
	*	Temp directory
	*	  Since PortableApps 10.0.2, if there is directory TempForPortableApps in the root of portable apps, it is used as TEMP
	*******************************************************/

	std::optional<std::filesystem::path> tempPath;

	if (const auto tempPathName = pCliInifile.getValue(SECTION_LAUNCHER, INI_NAME_TEMP_DIRECTORY)) {
		tempPath = p_apps::canonical(std::filesystem::path(p_apps::Environment::expandEnv(tempPathName.value())), pAppsDir / PORTABLE_APPS_DATA);
	}

	if (! tempPath) {
		auto TempForPortableApps = p_apps::canonical(pAppsDir / L"..\\..\\TempForPortableApps", pAppsDir);
		if (is_directory(TempForPortableApps)) {
			tempPath = TempForPortableApps;
		}
	}

	if (!tempPath) {
		auto systemTempPathName = p_apps::canonical(environment.get(ENV_TEMP), L"");
		if (is_directory(systemTempPathName)) {
			tempPath = systemTempPathName;
		}
	}

	if (tempPath) {
		p_apps::makeDirWriteable(tempPath.value());
		environment.set(ENV_TEMP, tempPath.value().c_str());
		environment.set(ENV_TMP, tempPath.value().c_str());
		logger::trace(L"[%s] TEMP directory: %s", _T(__FUNCTION__), tempPath.value().c_str());
	} else {
		logger::warning(L"[%s] unable to locate TEMP", _T(__FUNCTION__));
	}
}

void TccLauncher::processLocalListsFileHelper(const std::wstring& pCLiLocalHistoryName, const std::wstring& commandOptionLocalHistory, const std::wstring& environmentHistoryFile) {

	const auto isLocal =
		yesNoOption(tccIniFile.getValue(SECTION_4NT, pCLiLocalHistoryName))
		|| options.exists(L"/L:")
		|| options.exists(commandOptionLocalHistory)
		|| directives.exists(commandOptionLocalHistory, L"Yes");

	environment.set(environmentHistoryFile,
	                isLocal
		                ? L"1"
		                : L"0");

	logger::trace(L"[%s] %s is %s", _T(__FUNCTION__), pCLiLocalHistoryName, isLocal
		                                                                        ? L"local"
		                                                                        : L"global");
}

void TccLauncher::processLocalLists() {
	processLocalListsFileHelper(INI_4NT_NAME_LOCAL_ALIASES, L"/LA", ENV_LOCAL_ALIASES);
	processLocalListsFileHelper(INI_4NT_NAME_LOCALDIR_HISTORY, L"/LD", ENV_LOCAL_DIRHISTORY);
	processLocalListsFileHelper(INI_4NT_NAME_LOCAL_HISTORY, L"/LH", ENV_LOCAL_HISTORY);
}

void TccLauncher::processLogs() {
	const auto logsDirectory = p_apps::canonical(p_apps::Environment::expandEnv(pCliInifile.getValueNonEmpty(SECTION_LAUNCHER, INI_NAME_LOGS_DIRECTORY)), pAppsDir / PORTABLE_APPS_DATA);
	p_apps::makeDirWriteable(logsDirectory);
	logger::trace(L"[%s] logs directory %s", _T(__FUNCTION__), logsDirectory.c_str());

	auto commandLogFile = p_apps::Environment::expandEnv(pCliInifile.getValueNonEmpty(SECTION_LAUNCHER, INI_NAME_COMMAND_LOG_FILE));
	directives.set(INI_NAME_COMMAND_LOG_FILE, logsDirectory / commandLogFile);
	logger::trace(L"[%s] command log file: %s", _T(__FUNCTION__), commandLogFile);

	auto errorLogFile = p_apps::Environment::expandEnv(pCliInifile.getValueNonEmpty(SECTION_LAUNCHER, INI_NAME_ERRORS_LOG_FILE));
	directives.set(INI_NAME_ERRORS_LOG_FILE, logsDirectory / errorLogFile);
	logger::trace(L"[%s] error log file: %s", _T(__FUNCTION__), errorLogFile);

	auto historyLogFile = p_apps::Environment::expandEnv(pCliInifile.getValueNonEmpty(SECTION_LAUNCHER, INI_NAME_HISTORY_LOG_FILE));
	directives.set(INI_NAME_HISTORY_LOG_FILE, logsDirectory / historyLogFile);
	logger::trace(L"[%s] history log file: %s", _T(__FUNCTION__), historyLogFile);
}

std::optional<std::wstring> TccLauncher::locateKnownEditor() const {
	for (const auto& proposedEditor : knownEditors) {
		if (exists(pAppsDir / L".." / proposedEditor)) {
			return proposedEditor;
		}
	}
	return std::nullopt; 
}

void TccLauncher::processEditor() {

	const auto externalEditorInit = tccIniFile.getValue(SECTION_4NT, INI_NAME_EDITOR);
	auto externalEditor = externalEditorInit;

	if (!externalEditor) {
		externalEditor = pCliInifile.getValue(SECTION_LAUNCHER, INI_NAME_EDITOR);
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
		externalEditor = L"notepad.exe";
	}

	if (externalEditor) {
		auto externalEditorFullPath = p_apps::canonical(externalEditor.value(), pAppsDir / L"..");
		if (exists(externalEditorFullPath)) {
			externalEditor = externalEditorFullPath;
		}
	}

	if (externalEditorInit != externalEditor) {
		directives.setOptional(INI_NAME_EDITOR, externalEditor);
		logger::trace(L"[%s] external editor: %s", _T(__FUNCTION__), externalEditor.value().c_str());
	}
}

void TccLauncher::selectTccExe() {
	const auto runX64 =
		SysInfo::isWow64()
		&& ! yesNoOption(pCliInifile.getValue(SECTION_LAUNCHER, INI_NAME_FORCE32))
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
		logger::trace(L"[%s] exe: %s", _T(__FUNCTION__), runner.c_str());
	} else {
		fail(L"[%s] expected tcc.exe not found: %s", _T(__FUNCTION__), runner.c_str());
	}
}

void TccLauncher::selectLanguage() {
	const auto languageDllInit = tccIniFile.getValue(SECTION_4NT, INI_4NT_NAME_LANGUAGE_DLL);

	auto languageDll = languageDllInit;

	if (!languageDll) {
		languageDll = pCliInifile.getValue(SECTION_LAUNCHER, INI_4NT_NAME_LANGUAGE_DLL);
	}

	if (!languageDll && environment.exists(ENV_PAPPS_LANGUAGE)) {
		languageDll = environment.get(ENV_PAPPS_LANGUAGE) + L".dll";
	}

	if (languageDll && !exists(tccExeDirectory / languageDll.value())) {
		languageDll = std::nullopt;
	}

	if (languageDllInit != languageDll) {
		directives.setOptional(INI_4NT_NAME_LANGUAGE_DLL, languageDll);
		logger::trace(L"[%s] using LanguageDLL: %s", _T(__FUNCTION__), languageDll.value_or(L"<none>").c_str());
	}
}

void TccLauncher::copyMoreDirectives() {
	// ReSharper disable once CppInconsistentNaming
	p_apps::SetInsensitiveTChar names4NT;
	pCliInifile.enumNames(SECTION_4NT, names4NT);

	for (const auto& key : names4NT) {
		auto value = pCliInifile.getValue(SECTION_4NT, key);

		if (value) {
			value = p_apps::Environment::expandEnv(value.value());
		}

		directives.setOptional(key, value);
	}
	logger::trace(L"[%s] more tcc.ini %s entries: %d values", _T(__FUNCTION__), SECTION_4NT, names4NT.size());
}

void TccLauncher::copyMoreEnvironment() {
	p_apps::SetInsensitiveTChar namesEnv;
	pCliInifile.enumNames(SECTION_ENVIRONMENT, namesEnv);
	auto erasedCounter = 0;

	for (const auto& key : namesEnv) {
		auto value = pCliInifile.getValue(SECTION_ENVIRONMENT, key);

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

	logger::trace(L"[%s] more tcc.ini %s entries: %d values, %d erased", _T(__FUNCTION__), SECTION_4NT, namesEnv.size(), erasedCounter);
}

bool TccLauncher::ifWaitForTccToFinish() const {

	// ReSharper disable once CppTooWideScope
	const auto waitSettings = pCliInifile.getValue(SECTION_LAUNCHER, INI_NAME_WAIT);
	if (waitSettings) {
		return yesNoOption(waitSettings.value());
	}

	if (!SysInfo::ownsConsole()) {
		logger::trace(L"[%s] ownsConsole() == false", _T(__FUNCTION__));
		return true;
	}

	if (SysInfo::getDllName(L"ConEmuHk.dll") || SysInfo::getDllName(L"ConEmuHk64.dll")) {
		logger::trace(L"[%s] conEmu detected", _T(__FUNCTION__));
		return true;
	}

	return false;
}

void TccLauncher::launch() {
	logger::trace(L"[%s] Start", _T(__FUNCTION__));

	environment.setUp(envp);
	environment.setNameBackup(ENV_BACKUP_PREFIX);

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

	tccIniFile.readIniFile(tccIniFilename.value());

	directives.set(INI_4NT_NAME_TC_START_PATH, settingsDirectory);

	directives.set(INI_4NT_NAME_TREE_PATH, profileLocalDirectory);

	setupTempDirectory();

	processLocalLists();

	processLogs();

	processEditor();

	selectTccExe();

	selectLanguage();

	copyMoreDirectives();

	copyMoreEnvironment();

	const auto pWait = ifWaitForTccToFinish();

	/*******************************************************
	*	Ready to execute
	*******************************************************/
	const auto effectiveArgv0 = tccExeDirectory / tccExeName;
	std::vector<std::wstring> effectiveArgv;
	effectiveArgv.push_back(p_apps::quote(effectiveArgv0));
	// commandComspec: ignored	
	effectiveArgv.push_back(std::wstring(L"@") + p_apps::quote(tccIniFilename.value()));
	p_apps::appendContainer(effectiveArgv, directives);
	p_apps::appendContainer(effectiveArgv, options);
	p_apps::appendContainer(effectiveArgv, commands);
	if (logger::isTrace()) {
		logger::trace(L"[%s] finally argv:\n%s", _T(__FUNCTION__), boost::algorithm::join(effectiveArgv, L" "));
	}
	execute(pWait, effectiveArgv0, effectiveArgv, environment, currentPath);
}

/******************************************************************************
*
*	main
*
******************************************************************************/

int _tmain(int argc, wchar_t* argv[], wchar_t* envp[]) {
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	// _CrtSetBreakAlloc(522);
#endif

	logger::init();

	TccLauncher tccLauncher{argc, argv, envp};
	tccLauncher.launch();

	return 0;
}
