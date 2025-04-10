﻿/*
Copyright (C) 2024 The XLang Foundation
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "module.h"
#include "builtin.h"
#include "object.h"
#include "function.h"
#include "event.h"
#include <iostream>
#include "PyEngObject.h"
#include "dbg.h"
#include "port.h"
#include "moduleobject.h"

namespace X 
{
	extern XLoad* g_pXload;
namespace AST 
{
void Module::SetDebug(bool b,XlangRuntime* runtime)
{
	if (b)
	{
		G::I().SetTrace(Dbg::xTraceFunc);
		if (g_pXload->GetConfig().enablePythonDebug)
		{
			PyEng::Object objRT((unsigned long long)runtime);
			PyEng::Object::SetTrace(Dbg::PythonTraceFunc,objRT);
		}
	}
	else
	{
		PyEng::Object::SetTrace(nullptr, nullptr);
	}
}
void Module::ScopeLayout()
{
	std::string self("self");
	SCOPE_FAST_CALL_AddOrGet0_NoRet(m_pMyScope,self,false);
	auto& funcs = Builtin::I().All();
	for (auto it : funcs)
	{
		SCOPE_FAST_CALL_AddOrGet0(idx,m_pMyScope,it.name, false);
}
	Builtin::I().ReturnMap();
	Block::ScopeLayout();
}
void Module::AddBuiltins(XlangRuntime* rt)
{
	auto& funcs = Builtin::I().All();
	m_stackFrame->SetVarCount(m_pMyScope->GetVarNum());
	//add self as this module
	{
		std::string selfName("self");
		SCOPE_FAST_CALL_AddOrGet0(idx,m_pMyScope,selfName, true);
		if (idx >= 0)
		{
			auto* pModuleObj = new ModuleObject(this);
			Value v0(pModuleObj);
			m_stackFrame->Set(idx, v0);
		}
	}
	for (auto it : funcs)
	{
		SCOPE_FAST_CALL_AddOrGet0(idx,m_pMyScope,it.name, true);
		if (idx >= 0)
		{
			Value v0(it.funcObj);
			m_stackFrame->Set(idx, v0);
		}
	}
	Builtin::I().ReturnMap();
}
void Module::ClearBreakpoints()
{
	m_lockBreakpoints.Lock();
	m_breakpoints.clear();
	m_lockBreakpoints.Unlock();
}
//return the actual line
//-1 means no actual line matched with input line
int Module::SetBreakpoint(int line, int sessionTid)
{
	m_lockBreakpoints.Lock();
	m_breakpoints.push_back({ line,sessionTid });
	m_lockBreakpoints.Unlock();
	return line;
}
bool Module::HitBreakpoint(XlangRuntime* rt,int line)
{
	bool bHit = false;
	int hitSessionTid = 0;
	m_lockBreakpoints.Lock();
	for (auto it : m_breakpoints)
	{
		if (it.line == line)
		{
			bHit = true;
			hitSessionTid = it.sessionTid;
			break;
		}
	}
	m_lockBreakpoints.Unlock();
	if (bHit)
	{
		if (rt->m_pFirstStepOutExp) // stop continuous step out
			rt->m_pFirstStepOutExp = nullptr;
		KWARGS kwParams;
		X::Value valTid(hitSessionTid);
		kwParams.Add("tid", valTid);
		X::Value valAction("notify");
		kwParams.Add("action", valAction);
		const int online_len = 1000;
		char strBuf[online_len];
		int thread = GetThreadID();
		SPRINTF(strBuf, online_len, "[{\"HitBreakpoint\":%d, \"threadId\":%d}]", line, thread);
		X::Value valParam(strBuf);
		kwParams.Add("param", valParam);
		std::cout << "HitBreakpoint in file: " << m_moduleName.substr(m_moduleName.rfind("/") + 1) << "   line: " << line << "   threadId: " << thread << std::endl;
		std::string evtName("devops.dbg");
		ARGS params(0);
		X::EventSystem::I().Fire(nullptr,nullptr,evtName,params,kwParams);
	}
	return bHit;
}

void Module::StopOn(const char* stopType)
{
	KWARGS kwParams;
	X::Value valAction("notify");
	kwParams.Add("action", valAction);
	const int online_len = 1000;
	char strBuf[online_len];
	int thread = GetThreadID();
	SPRINTF(strBuf, online_len, "[{\"%s\":%d}]", stopType, thread);
	X::Value valParam(strBuf);
	kwParams.Add("param", valParam);
	std::cout << stopType << " threadId:" << thread << std::endl;
	std::string evtName("devops.dbg");
	ARGS params(0);
	X::EventSystem::I().Fire(nullptr, nullptr, evtName, params, kwParams);
}
}
}