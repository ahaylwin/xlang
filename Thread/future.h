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

#pragma once

#include "object.h"

namespace X
{
	class Task;
	namespace Data
	{
		class Future :
			public virtual Object
		{
			Task* m_pTask = nullptr;
			std::vector<XWait*> m_waits;
			std::vector<X::Value> m_thenProcs;//when reach the future, call this object
			bool m_GotVal = false;
			X::Value m_Val;//hold return value
		public:
			static void Init();
			static void cleanup();
			Future() :
				Object()
			{
				m_t = ObjType::Future;
			}
			Future(Task* task)
				:Future()
			{
				m_pTask = task;
			}
			void RemoveTask()
			{
				//when task finished, call this function
				m_pTask = nullptr;
			}
			void SetVal(X::Value& v);

			bool GetResult(X::Value& retVal,int timeout);
			void SetThenProc(X::Value& v)
			{
				m_thenProcs.push_back(v);
			}
			virtual bool Call(XRuntime* rt, XObj* pContext, ARGS& params,
				KWARGS& kwParams,
				X::Value& retValue) override
			{
				return false;
			}
			virtual void GetBaseScopes(std::vector<AST::Scope*>& bases) override;
			virtual const char* ToString(bool WithFormat = false)
			{
				char v[1000];
				snprintf(v, sizeof(v), "Future:%llu",
					(long long)this);
				std::string retStr(v);
				return GetABIString(retStr);
			}
		};
	}
}