#pragma once
#include "scope.h"
#include "singleton.h"
#include "variable_frame.h"

namespace X
{
	namespace AST
	{
		class MetaScope :
			public Scope,
			public Singleton<MetaScope>
		{
			VariableFrame* m_stack = nullptr;
		public:
			MetaScope()
			{
				m_stack = new VariableFrame();
			}
			void Init()
			{

			}
			void Cleanup()
			{
				if (m_stack)
				{
					delete m_stack;
					m_stack = nullptr;
				}
			}
			~MetaScope()
			{
				if (m_stack)
				{
					delete m_stack;
				}
			}
			virtual Scope* GetParentScope()
			{
				return nullptr;
			}
			virtual int AddOrGet(std::string& name, bool bGetOnly,Scope** ppRightScope = nullptr) override
			{
				int retIdx = Scope::AddOrGet(name, bGetOnly, ppRightScope);
				if (!bGetOnly)
				{
					m_stack->SetVarCount(GetVarNum());
				}
				return retIdx;
			}
			inline virtual bool Get(XlangRuntime* rt, XObj* pContext,
				int idx, X::Value& v, LValue* lValue = nullptr) override
			{
				m_stack->Get(idx, v, lValue);
				return true;
			}
			inline virtual bool Set(XlangRuntime* rt, XObj* pContext,
				int idx, X::Value& v) override
			{
				m_stack->Set(idx, v);
				return true;
			}
		};
	}
}