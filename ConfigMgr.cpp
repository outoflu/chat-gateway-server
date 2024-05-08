#include "ConfigMgr.h"
#include "boost/filesystem.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/ini_parser.hpp"
SectionInfo::SectionInfo()
{
}

SectionInfo::~SectionInfo()
{
	_section_datas.clear();
}

SectionInfo::SectionInfo(const SectionInfo& src)
{
	_section_datas = src._section_datas;
}

SectionInfo& SectionInfo::operator=(const SectionInfo& src)
{
	if (this == &src) {
		return *this;
	}
	_section_datas = src._section_datas;
	return *this;
}

std::string SectionInfo::operator[](const std::string& key)
{
	if (_section_datas.find(key) == _section_datas.end()) {
		return "";
	}
	return _section_datas[key];
}

ConfigMgr::~ConfigMgr()
{
}

SectionInfo ConfigMgr::operator[](const std::string& section)
{
	if (_config_map.find(section) == _config_map.end()) {
		return SectionInfo();
	}
	else {
		return _config_map[section];
	}
}

ConfigMgr& ConfigMgr::getInstance()
{
	static ConfigMgr cfg_Mgr;
	return cfg_Mgr;
}

ConfigMgr::ConfigMgr()
{
	boost::filesystem::path current_path = boost::filesystem::current_path();
	boost::filesystem::path config_path = current_path / "config.ini";

	boost::property_tree::ptree pt;
	boost::property_tree::read_ini(config_path.string(), pt);

	for (const auto& section_pair : pt) {
		const std::string& section_name = section_pair.first;
		const boost::property_tree::ptree& section_tree = section_pair.second;
		std::map<std::string, std::string> section_config;
		for (const auto& key_value_pair:section_tree) {
			const std::string& key = key_value_pair.first;
			const std::string& value = key_value_pair.second.get_value<std::string>();
			section_config.insert(std::pair<std::string,std::string>(key, value));
		}
		SectionInfo sectionInfo;
		sectionInfo._section_datas = section_config;
		_config_map[section_name] = sectionInfo;
	}
}

ConfigMgr::ConfigMgr(const ConfigMgr& src)
{
	_config_map = src._config_map;
}

ConfigMgr& ConfigMgr::operator=(const ConfigMgr& src)
{
	// TODO: 在此处插入 return 语句
	if (this == &src) {
		return *this;
	}
	else {
		_config_map = src._config_map;
		return *this;
	}
}
