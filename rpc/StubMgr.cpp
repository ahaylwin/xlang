#include "StubMgr.h"
#include "Stub.h"

namespace X
{
    void RemotingManager::Register(unsigned int callID,
        RemotingCallBase* pHandler,
        std::string funcName,
        std::vector<std::string> inputTypes,
        std::string retType)
    {
        RemoteFuncInfo* pFuncInfo = nullptr;
        mLockMapFuncs.Lock();
        auto it = mMapFuncs.find(callID);
        if (it != mMapFuncs.end())
        {
            pFuncInfo = it->second;
        }
        else
        {
            pFuncInfo = new RemoteFuncInfo();
            mMapFuncs.emplace(std::make_pair(callID, pFuncInfo));
        }
        pFuncInfo->callID = callID;
        pFuncInfo->pHandler = pHandler;
        pFuncInfo->funcName = funcName;
        pFuncInfo->inputTypes = inputTypes;
        pFuncInfo->retType = retType;
        mLockMapFuncs.Unlock();
    }

    void RemotingManager::Unregister(unsigned int callID)
    {
        mLockMapFuncs.Lock();
        auto it = mMapFuncs.find(callID);
        if (it != mMapFuncs.end())
        {
            delete it->second;
            mMapFuncs.erase(it);
        }
        mLockMapFuncs.Unlock();
    }
    void RemotingManager::CreateStub(unsigned long long key)
    {
        XLangStub* pStub = NULL;
        mStubLock.Lock();
        for (auto s : mStubs)
        {
            if (s->GetKey() == key)
            {
                pStub = s;
                break;
            }
        }
        mStubLock.Unlock();
        if (pStub == NULL)
        {
            pStub = new XLangStub();
            mStubLock.Lock();
            unsigned long long sid = ++mLastSessionID;
            pStub->SetSessionId(sid);
            mStubs.push_back(pStub);
            mStubLock.Unlock();
        }
        if (pStub->Create(key))
        {
            pStub->Start();
        }
    }
    bool RemotingManager::IsStubExist(unsigned long long sessionId)
    {
        bool bExist = false;
        mStubLock.Lock();
        for (auto it : mStubs)
        {
            if (it->GetSessionId() == sessionId)
            {
                bExist = true;
                break;
            }
        }
        mStubLock.Unlock();
        return bExist;
    }
    void RemotingManager::CloseStub(void* pStub)
    {
        XLangStub* pHostStub = (XLangStub*)pStub;
        if (pHostStub == nullptr)
        {
            return;
        }
        pHostStub->Quit();
        pHostStub->Stop();
        mStubLock.Lock();
        for (auto it = mStubs.begin(); it != mStubs.end(); it++)
        {
            if (*it == pHostStub)
            {
                mStubs.erase(it);
                break;
            }
        }
        mStubLock.Unlock();
        delete pHostStub;
    }
    void CallWorker::run()
    {
        while (m_bRun)
        {
            m_pWait->Wait(-1);
            m_bIdle = false;
            m_func();
            m_bIdle = true;
        }
    }
    CallWorker::CallWorker()
    {
        m_pWait = new XWait();
    }
    CallWorker::~CallWorker()
    {
        delete m_pWait;
    }
}