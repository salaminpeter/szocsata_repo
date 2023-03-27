#pragma once

#include <map>
#include <string>
#include <sstream>

class CSettingBase
{
public:
	
	CSettingBase(bool saveToFile) : m_SaveToFile(saveToFile) {}
	virtual std::string GetValueAsString() = 0;

	bool m_SaveToFile = true;
};

template <class T>
class CSetting : public CSettingBase
{
public:
	CSetting(T value, bool saveToFile) : CSettingBase(saveToFile), m_Value(value) {}
	
	virtual std::string GetValueAsString()
	{
		std::stringstream ss;
		ss.precision(2);
		ss.setf(std::ios::fixed);
		ss << m_Value;
		return ss.str();
	}


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
	static void AddConfig(std::string name, T value, bool saveToFile = true)
	{
		m_Configs[name] = (CSettingBase*)(new CSetting<T>(value, saveToFile));
	}

	static bool LoadConfigs(const char* path, bool loadDefaults);
	static void SaveConfigs(const char* path);

private:

	static std::map<std::string, CSettingBase*> m_Configs;
};