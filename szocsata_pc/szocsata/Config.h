#pragma once

#include <map>
#include <string>

class CSettingBase
{
};

template <class T>
class CSetting : public CSettingBase
{
public:
	CSetting(T value) : m_Value(value) {}
	T m_Value;
};

class CConfig
{
public:

	template <class T>
	static bool GetConfig(const std::string& name, T& value)
	{
		if (m_Configs.find(name) == m_Configs.end())
			return false;

		value = static_cast<CSetting<T>*>(m_Configs[name])->m_Value;

		return true;
	}

	template <class T>
	static void AddConfig(std::string name, T value)
	{
		m_Configs[name] = (CSettingBase*)(new CSetting<T>(value));
	}

	static void LoadConfigs(const char* path);

private:

	static std::map<std::string, CSettingBase*> m_Configs;
};