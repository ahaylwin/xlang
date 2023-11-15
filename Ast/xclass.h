#pragma once
#include "exp.h"
#include "scope.h"
#include "func.h"
#include "variable_frame.h"

namespace X
{
namespace AST
{
#define FastMatchThis(name) (name.size() ==4 \
	&& name[0] =='t' && name[0] =='h' && name[0] =='i' && name[0] =='s')
class XClass
	:public Func
{
	Func* m_constructor = nil;
	AST::VariableFrame* m_stackFrame = nullptr;//to hold non-instance properties
	std::vector<Value> m_bases;
	XClass* FindBase(XlangRuntime* rt, std::string& strName);
public:
	XClass() :
		Func()
	{
		m_type = ObType::Class;
	}
	~XClass()
	{
		if (m_stackFrame)
		{
			delete m_stackFrame;
		}
	}
	inline int QueryConstructor()
	{
		auto it = m_Vars.find(GetNameString());
		if (it != m_Vars.end())
		{
			return it->second;
		}
		it = m_Vars.find("constructor");
		if (it != m_Vars.end())
		{
			return it->second;
		}
		it = m_Vars.find("__init__");
		if (it != m_Vars.end())
		{
			return it->second;
		}
		return -1;
	}
	inline VariableFrame* GetClassStack()
	{
		return m_stackFrame;
	}
	virtual bool ToBytes(XlangRuntime* rt, XObj* pContext, X::XLangStream& stream) override
	{
		std::string code;
		for (auto* decor : m_decors)
		{
			code += decor->GetCode() + "\n";
		}
		code += GetCode();
		//change current scope of stream
		Scope* pOldClassScope = stream.ScopeSpace().GetCurrentClassScope();
		Scope* pOldScope = stream.ScopeSpace().GetCurrentScope();
		auto* pCurScope = dynamic_cast<Scope*>(this);
		stream.ScopeSpace().SetCurrentScope(pCurScope);
		stream.ScopeSpace().SetCurrentClassScope(pCurScope);
		Block::ToBytes(rt, pContext, stream);
		SaveToStream(rt, pContext, Params, stream);
		SaveToStream(rt, pContext, RetType, stream);
		//restore old scope
		stream.ScopeSpace().SetCurrentScope(pOldScope);
		stream.ScopeSpace().SetCurrentClassScope(pOldClassScope);

		//Coding itself
		stream << m_Name.size;
		if (m_Name.size > 0)
		{
			stream.append(m_Name.s, m_Name.size);
		}
		stream << (int)m_IndexofParamList.size();
		for (auto idx : m_IndexofParamList)
		{
			stream << idx;
		}
		stream << m_Index << m_IndexOfThis << m_needSetHint;
		Scope::ToBytes(rt, pContext, stream);

		return true;
	}
	virtual bool FromBytes(X::XLangStream& stream) override;
	virtual int AddOrGet(std::string& name, bool bGetOnly, Scope** ppRightScope = nullptr) override;
	virtual int AddAndSet(XlangRuntime* rt, XObj* pContext, std::string& name, Value& v) override
	{
		int idx = AddOrGet(name, false);
		if (idx >= 0)
		{
			int cnt = m_stackFrame->GetVarCount();
			if (cnt <= idx)
			{
				m_stackFrame->SetVarCount(idx + 1);
			}
			Set(rt, pContext, idx, v);
		}
		return idx;
	}
	virtual bool Set(XlangRuntime* rt, XObj* pContext, int idx, Value& v) override;
	virtual bool Get(XlangRuntime* rt, XObj* pContext, int idx, Value& v,
		LValue* lValue = nullptr) override;
	inline std::vector<Value>& GetBases() { return m_bases; }
	bool Exec_i(XlangRuntime* rt, ExecAction& action, XObj* pContext, Value& v, LValue* lValue = nullptr);
	bool BuildBaseInstances(XlangRuntime* rt, XObj* pClassObj);
	virtual bool Exec(XlangRuntime* rt,ExecAction& action, XObj* pContext, Value& v, LValue* lValue = nullptr) override;
	virtual void ScopeLayout() override;
	virtual void Add(Expression* item) override;
	virtual bool Call(XlangRuntime* rt,
		XObj* pContext,
		ARGS& params,
		KWARGS& kwParams,
		Value& retValue);
	virtual bool CalcCallables(XlangRuntime* rt, XObj* pContext,
		std::vector<Scope*>& callables) override
	{
		bool bHave = false;
		if (m_constructor)
		{
			bHave = m_constructor->CalcCallables(rt, pContext, callables);
		}
		return bHave;
	}
};
}
}