#pragma once

#include <functional>


class CEventBase
{
public:
	
	virtual void HandleEvent() = 0;
};

template <typename ClassType, typename... ArgTypes> 
class CEvent : public CEventBase
{
public:
	
	typedef void (ClassType::*TFuncPtrType)(ArgTypes...);
	
	CEvent(ClassType* funcClass, TFuncPtrType funcPtr, ArgTypes&&... args) :
		m_Class(funcClass),
		m_Function(funcPtr),
		m_Tuple(std::forward<ArgTypes>(args)...)
	{
	}
	
	void HandleEvent() override
	{
		CallFunc(std::make_index_sequence<std::tuple_size<decltype(m_Tuple)>::value>{});
	}

private:
	
	template <size_t... Idx>
	void CallFunc(std::index_sequence<Idx...>)
	{
		(m_Class->*m_Function)(std::get<Idx>(m_Tuple)...);
	}
	
private:
	
	ClassType* m_Class;
	std::tuple<ArgTypes...> m_Tuple;
	TFuncPtrType m_Function;
};
