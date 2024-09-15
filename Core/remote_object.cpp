#include "remote_object.h"
#include "list.h"

namespace X
{
	X::Data::List* RemoteObject::FlatPack(XlangRuntime* rt, XObj* pContext,
		std::vector<std::string>& IdList, int id_offset,
		long long startIndex, long long count)
	{
		X::Data::List* pPackList = nullptr;
		X::Value valPack;
		//neet to convert IdList to xlang's Vector
		Port::vector<std::string> IdList_new((int)IdList.size());
		for (auto& id : IdList)
		{
			IdList_new.push_back(id);
		}
		m_proxyLock.Lock();
		bool bOK = m_proxy?m_proxy->FlatPack(m_remote_Parent_Obj_id,
			m_remote_Obj_id, IdList_new, id_offset,startIndex, count, valPack):false;
		m_proxyLock.Unlock();
		if (bOK && valPack.IsObject())
		{
			pPackList = dynamic_cast<X::Data::List*>(valPack.GetObj());
			if (pPackList)
			{
				pPackList->IncRef();
			}
		}
		return pPackList;
	}
	X::Value RemoteObject::UpdateItemValue(XlangRuntime* rt, XObj* pContext,
		std::vector<std::string>& IdList, int id_offset,
		std::string itemName, X::Value& val)
	{
		//neet to convert IdList to xlang's Vector
		Port::vector<std::string> IdList_new((int)IdList.size());
		for (auto& id : IdList)
		{
			IdList_new.push_back(id);
		}
		X::Value retValue;
		m_proxyLock.Lock();
		if (m_proxy)
		{
			retValue = m_proxy->UpdateItemValue(m_remote_Parent_Obj_id,
				m_remote_Obj_id, IdList_new, id_offset, itemName, val);
		}
		m_proxyLock.Unlock();
		return retValue;
	}
}