#pragma once

#include <string>
#include <unordered_map>
#include "value.h"
#include "runtime.h"
#include <assert.h>
#include <functional>
#include "objref.h"
#include "def.h"
#include "XLangStream.h"
#include "Locker.h"

namespace X 
{ 
namespace AST 
{
class Var;
enum class ScopeWaitingStatus
{
	NoWaiting,
	HasWaiting,
	NeedFurtherCallWithName
};
enum class ScopeVarIndex
{
	INVALID =-1,
	EXTERN =-2
};
class Scope:
	virtual public ObjRef
{//variables scope support, for Module and Func/Class
	Locker m_lock;
protected:
	std::unordered_map <std::string, int> m_Vars;
	std::unordered_map <std::string, AST::Var*> m_ExternVarMap;
public:
	Scope():
		ObjRef()
	{
	}
	//use address as ID, just used Serialization
	ExpId ID() { return (ExpId)this; }
	void AddExternVar(AST::Var* var);
	inline virtual int IncRef()
	{
		AutoLock autoLock(m_lock);
		return ObjRef::AddRef();
	}
	inline virtual int DecRef()
	{
		m_lock.Lock();
		int ref = ObjRef::Release();
		if (ref == 0)
		{
			m_lock.Unlock();
			delete this;
		}
		else
		{
			m_lock.Unlock();
		}
		return ref;
	}
	virtual bool ToBytes(XlangRuntime* rt, XObj* pContext, X::XLangStream& stream);
	virtual bool FromBytes(X::XLangStream& stream);
	inline int GetVarNum()
	{
		return (int)m_Vars.size();
	}
	virtual std::string GetNameString()
	{
		return "";
	}
	inline std::unordered_map <std::string, int>& GetVarMap() 
	{ 
		return m_Vars; 
	}
	inline std::vector<std::string> GetVarNames()
	{
		std::vector<std::string> names;
		for (auto& it : m_Vars)
		{
			names.push_back(it.first);
		}
		return names;
	}
	virtual void EachVar(XlangRuntime* rt,XObj* pContext,
		std::function<void(std::string,X::Value&)> const& f)
	{
		for (auto it : m_Vars)
		{
			X::Value val;
			Get(rt, pContext,it.second, val);
			f(it.first, val);
		}
	}
	virtual std::string GetModuleName(XlangRuntime* rt);
	virtual bool isEqual(Scope* s) { return (this == s); };
	virtual ScopeWaitingStatus IsWaitForCall() 
	{ 
		return ScopeWaitingStatus::NoWaiting;
	};
	virtual Scope* GetParentScope()= 0;
	virtual int AddAndSet(XlangRuntime* rt, XObj* pContext,std::string& name, Value& v)
	{
		int idx = AddOrGet(name, false,nullptr);
		if (idx >= 0)
		{
			rt->DynSet(this, pContext, idx, v);
		}
		return idx;
	}
	virtual int AddOrGet(std::string& name, bool bGetOnly, Scope** ppRightScope=nullptr)
	{//Always append,no remove, so new item's index is size of m_Vars;
		//check extern map first,if it is extern var
		//just return -1 to make caller look up to parent scopes
		if (m_ExternVarMap.find(name)!= m_ExternVarMap.end())
		{
			return (int)ScopeVarIndex::EXTERN;
		}
		auto it = m_Vars.find(name);
		if (it != m_Vars.end())
		{
			return it->second;
		}
		else if (!bGetOnly)
		{
			int idx = (int)m_Vars.size();
			m_Vars.emplace(std::make_pair(name, idx));
			return idx;
		}
		else
		{
			return (int)ScopeVarIndex::INVALID;
		}
	}
	inline virtual bool Get(XlangRuntime* rt, XObj* pContext,
		std::string& name, X::Value& v, LValue* lValue = nullptr)
	{
		int idx = AddOrGet(name, true);
		return (idx>=0)?rt->Get(this, pContext, idx, v, lValue):false;
	}
	inline bool RuntimeSet(XlangRuntime* rt, XObj* pContext,
		int idx, X::Value& v)
	{
		assert(idx != -1);
		return rt->Set(this, pContext, idx, v);
	}
	inline virtual bool Set(XlangRuntime* rt, XObj* pContext,
		int idx, X::Value& v)
	{
		assert(idx != -1);
		return rt->Set(this, pContext, idx, v);
	}

	inline virtual bool Get(XlangRuntime* rt, XObj* pContext,
		int idx, X::Value& v, LValue* lValue = nullptr)
	{
		return rt->Get(this, pContext, idx, v, lValue);
	}
};
}
}
