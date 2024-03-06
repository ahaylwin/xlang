#pragma once

#include "xpackage.h"
#include "xlang.h"

namespace X 
{
	class JsonWrapper
	{
	public:
		BEGIN_PACKAGE(JsonWrapper)
			APISET().AddFunc<1>("loads", &JsonWrapper::LoadFromString);
			APISET().AddRTFunc<1>("load", &JsonWrapper::LoadFromFile);
			APISET().AddFunc<1>("saves", &JsonWrapper::SaveToString);
			END_PACKAGE
	public:
		JsonWrapper()
		{
		}
		std::string SaveToString(X::Value v)
		{
			return v.ToString(true);
		}
		X::Value LoadFromString(std::string jsonStr);
		X::Value  LoadFromFile(X::XRuntime* rt, X::XObj* pContext,std::string fileName);
	};
}