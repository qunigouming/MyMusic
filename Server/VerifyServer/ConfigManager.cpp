#include "ConfigManager.h"
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <iostream>

ConfigManager::ConfigManager()
{
	boost::filesystem::path current_path = boost::filesystem::current_path();
	boost::filesystem::path config_path = current_path / "config.ini";
	std::cout << "config_path: " << config_path << std::endl;
	boost::property_tree::ptree pt;
	boost::property_tree::read_ini(config_path.string(), pt);

	for (const auto& section : pt) {
		const std::string& section_name = section.first;
		const boost::property_tree::ptree& section_tree = section.second;
		std::map<std::string, std::string> section_config;
		for (const auto& key_value : section_tree) {
			const std::string& key = key_value.first;
			const std::string& value = key_value.second.get_value<std::string>();
			section_config[key] = value;
		}

		SectionInfo sectionInfo;
		sectionInfo._section_map = section_config;
		_config_map[section_name] = sectionInfo;
	}

	for (const auto& section : _config_map) {
		const std::string& section_name = section.first;
		const auto& section_config = section.second;
		std::cout << "[" << section_name << "]" << std::endl;
		for (const auto& key_value : section_config._section_map)
			std::cout << key_value.first << "=" << key_value.second << std::endl;
	}
}
