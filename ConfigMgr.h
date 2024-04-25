#pragma once
#include <map>
#include <string>
struct SectionInfo {
	SectionInfo();
	~SectionInfo();
	SectionInfo(const SectionInfo& src);
	SectionInfo& operator=(const SectionInfo& src);

	std::map<std::string, std::string> _section_datas;
	std::string operator[](const std::string& key);

};

class ConfigMgr
{
public:
	~ConfigMgr();
	SectionInfo operator[](const std::string& section);
	ConfigMgr();
	ConfigMgr(const ConfigMgr& src);
	ConfigMgr& operator=(const ConfigMgr& src);
private:
	std::map<std::string,SectionInfo> _config_map;
};

