#pragma once

#include "pApps.h"

// ReSharper disable CppInconsistentNaming

class CommandItems : public std::vector<std::wstring> {  // NOLINT(cppcoreguidelines-special-member-functions)
public:
	virtual ~CommandItems() = default;

	// ReSharper disable once CppMemberFunctionMayBeStatic
	void push_back_more() {}
};

class Commands final : public CommandItems {
	static bool isSlashC(const std::wstring& val) {
		return boost::iequals(val, L"/c") || boost::iequals(val, L"/k");
	}

public:

	void push_back(const std::wstring& command) {
		if (this->empty()) {
			if (!isSlashC(command)) {
				push_back(std::wstring{ L"/k" });
			}
		}
		else {
			if (isSlashC(command)) {
				push_back(std::wstring{ L"&" });
				return;
			}
		}
		std::vector<std::wstring>::push_back(p_apps::quote(command));
	}

	void push_back_more() {
		if (!this->empty()) {
			push_back(L"&");
		}
	}
};

class Directives final : public CommandItems {
public:
	void push_back(const std::wstring& directive) {
		const auto equalSignPosition = directive.find(L'=');

		if (equalSignPosition != std::string::npos) {
			auto key = directive.substr(0, equalSignPosition);
			if (!boost::starts_with(key, L"//")) {
				key = L"//" + key;
			}

			const auto value = p_apps::quote(directive.substr(equalSignPosition + 1, std::string::npos));

			std::vector<std::wstring>::push_back(key + L"=" + value);
		}
	}
	[[nodiscard]] std::wstring toString(const std::wstring& key, const std::wstring& value) const {
		return L"//" + key + L"=" + p_apps::quote(value);
	}

	[[nodiscard]] std::wstring toString(const std::wstring& key, const std::optional<std::wstring>& value) const {
		return L"//" + key + L"=" + p_apps::quote(value);
	}
	void set(const std::wstring& key, const std::wstring& value) {
		std::vector<std::wstring>::push_back(toString(key, value));
	}
	void setOptional(const std::wstring& key, const std::optional<std::wstring>& value) {
		std::vector<std::wstring>::push_back(toString(key, value));
	}
	bool exists(const std::wstring& key, const std::wstring& value) {
		return std::find(begin(), end(), toString(key, value)) != end();
	}
};

class Options final : public CommandItems {
public:
	bool exists(const std::wstring& option) {
		return std::find(begin(), end(), option) != end();
	}
};
