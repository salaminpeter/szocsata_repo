#include "stdafx.h"
#include "Config.h"
#include "FileHandler.h"

#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

#include "IOManager.h"

std::map<std::string, CSettingBase*> CConfig::m_Configs;

void CConfig::SaveConfigs(const char* path)
{
	std::ofstream ConfigFile(path, std::ofstream::out | std::ofstream::trunc);

	if (!ConfigFile.good())
		return;

	for (auto& Config : m_Configs)
	{
		if (Config.second->m_SaveToFile)
			ConfigFile << Config.first << " = " << Config.second->GetValueAsString() << '\n';
	}

	ConfigFile.close();
}

bool CConfig::LoadConfigs(const char* path, bool loadDefaults)
{
	std::stringstream StrStream;
	std::string Str;


	if (loadDefaults && !CIOManager::GetStreamStrForFile(path, StrStream))
		return false;

	if (!loadDefaults)
	{
		std::ifstream ConfigFile(path, std::ofstream::in);

		if (!ConfigFile.good())
			return false;

		StrStream << ConfigFile.rdbuf();
	}

	while (std::getline(StrStream, Str))
	{
        Str.erase(std::remove(Str.begin(), Str.end(), '\r'), Str.end());

        int EqPos = Str.find_first_of(L'=');

		if (EqPos == std::string::npos)
			continue;

		std::string ConfigName = Str.substr(0, EqPos);
		std::string ConfigValue = Str.substr(EqPos + 1, Str.length() - EqPos - 1);

		ConfigName.erase(remove_if(ConfigName.begin(), ConfigName.end(), isspace), ConfigName.end());
		ConfigValue.erase(remove_if(ConfigValue.begin(), ConfigValue.end(), isspace), ConfigValue.end());

		float FloatVal;
		std::string StringVal;
		std::stringstream ss(ConfigValue);

		if (Str.find(L'"') != std::string::npos)
		{
			AddConfig(ConfigName, ConfigValue.substr(1, ConfigValue.length() - 2));
		}
		else if (Str.find(L'.') != std::string::npos)
		{
			float n;
			ss >> n;
			AddConfig(ConfigName, n);
		}
		else
		{
			int n;
			ss >> n;
			AddConfig<int>(ConfigName, n);
		}
	}

	return true;
}