#pragma once
#include "Singleton.h"
#include <map>

class SectionInfo {
public:
	SectionInfo() = default;
	
	SectionInfo(const SectionInfo& src) {
		_section_map = src._section_map;
	}

	SectionInfo& operator= (const SectionInfo& src) {
		if (&src == this) return *this;
		_section_map = src._section_map;
		return *this;
	}

	std::string operator[] (const std::string& key) {
		if (_section_map.find(key) == _section_map.end())	return "";
		return _section_map[key];
	}

	~SectionInfo() {
		_section_map.clear();
	}

	std::map<std::string, std::string> _section_map;
};

class ConfigManager
{
public:
	ConfigManager();

	ConfigManager(const ConfigManager& src) {
		_config_map = src._config_map;
	}

	ConfigManager& operator=(const ConfigManager& src) {
		if (&src == this) return *this;
		_config_map = src._config_map;
	}

	SectionInfo operator[] (const std::string& section) {
		if (_config_map.find(section) == _config_map.end())	return SectionInfo();
		return _config_map[section];
	}

	~ConfigManager() {
		_config_map.clear();
	}

	static ConfigManager& GetInstance() {
		static ConfigManager instance;
		return instance;
	}

private:
	std::map<std::string, SectionInfo> _config_map;
};

