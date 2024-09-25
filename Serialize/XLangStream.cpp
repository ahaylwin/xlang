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

#include "XLangStream.h"
#include <stdexcept>
#include "object.h"
#include "str.h"
#include "list.h"
#include "dict.h"
#include "bin.h"
#include "function.h"
#include "remote_object.h"
#include "moduleobject.h"
#include "struct.h"
#include "deferred_object.h"
#include "funclist.h"
#include "remote_client_object.h"

namespace X 
{
	XLangStream::XLangStream()
	{
		m_scope_space = new XScopeSpace();
		m_bOwnScopeSpace = true;
		m_streamKey = 0;
	}
	XLangStream::~XLangStream()
	{
		if (m_pProvider)
		{
			m_pProvider->SetPos(GetPos());
		}
		m_size = 0;
		curPos = { 0,0 };
		if (m_bOwnScopeSpace)
		{
			delete m_scope_space;
		}
	}
	bool XLangStream::FullCopyTo(char* buf, STREAM_SIZE bufSize)
	{
		if (bufSize < Size())
		{
			return false;
		}
		int blkNum = BlockNum();
		if (blkNum == 0)
		{//empty
			return true;
		}
		for (int i = 0; i < blkNum; i++)
		{
			blockInfo& blk = GetBlockInfo(i);
			memcpy(buf, blk.buf, blk.data_size);
			buf += blk.data_size;
		}
		return true;
	}
	bool XLangStream::CopyTo(char* buf, STREAM_SIZE size)
	{
		blockIndex bi = curPos;
		STREAM_SIZE leftSize = size;
		char* pOutputData = buf;
		int curBlockIndex = bi.blockIndex;
		STREAM_SIZE blockOffset = bi.offset;
		while (leftSize > 0)
		{
			if (curBlockIndex >= BlockNum())
			{
				return false;
			}
			blockInfo& curBlock = GetBlockInfo(curBlockIndex);

			STREAM_SIZE restSizeInBlock = curBlock.data_size - blockOffset;
			STREAM_SIZE copySize = leftSize < restSizeInBlock ? leftSize : restSizeInBlock;
			if (buf != nullptr)
			{
				memcpy(pOutputData, curBlock.buf + blockOffset, copySize);
			}
			pOutputData += copySize;
			blockOffset += copySize;
			leftSize -= copySize;
			if (leftSize > 0)//need next block
			{
				if (!MoveToNextBlock())
				{
					return false;
				}
				blockOffset = 0;
				curBlockIndex++;
			}
		}
		curPos.blockIndex = curBlockIndex;
		curPos.offset = blockOffset;

		return true;
	}

	bool XLangStream::appendchar(char c)
	{
		int blkNum = BlockNum();
		if (curPos.blockIndex >= blkNum)
		{
			if (!NewBlock())
			{
				return false;
			}
		}
		blockInfo& curBlock = GetBlockInfo(curPos.blockIndex);
		if (curPos.offset == curBlock.block_size)
		{
			curPos.blockIndex++;
			if (curPos.blockIndex >= blkNum)
			{
				if (!NewBlock())
				{
					return false;
				}
			}
			curPos.offset = 0;
		}
		*(curBlock.buf + curPos.offset) = c;
		curPos.offset++;
		if (!m_InOverrideMode)
		{
			curBlock.data_size++;
			m_size++;
		}
		return true;
	}
	bool XLangStream::fetchchar(char& c)
	{
		int blkNum = BlockNum();
		if (curPos.blockIndex >= blkNum)
		{
			return false;
		}
		blockInfo& curBlock = GetBlockInfo(curPos.blockIndex);
		if (curPos.offset == curBlock.block_size)
		{
			MoveToNextBlock();
			curPos.blockIndex++;
			if (curPos.blockIndex >= blkNum)
			{
				return false;
			}
			curPos.offset = 0;
		}
		c = *(curBlock.buf + curPos.offset);
		curPos.offset++;
		return true;
	}
	bool XLangStream::fetchstring(std::string& str)
	{
		char ch = 0;
		bool bRet = true;
		while (bRet)
		{
			bRet = fetchchar(ch);
			if (ch == 0)
			{
				break;
			}
			if (bRet)
			{
				str += ch;
			}
		}
		return bRet;
	}
	bool XLangStream::append(char* data, STREAM_SIZE size)
	{
		blockIndex bi = curPos;
		STREAM_SIZE leftSize = size;
		char* pInputData = data;
		int curBlockIndex = bi.blockIndex;
		STREAM_SIZE blockOffset = bi.offset;

		while (leftSize > 0)
		{
			if (curBlockIndex >= BlockNum())
			{
				if (!NewBlock())
				{
					return false;
				}
			}
			blockInfo& curBlock = GetBlockInfo(curBlockIndex);
			STREAM_SIZE restSizeInBlock = curBlock.block_size - blockOffset;
			STREAM_SIZE copySize = leftSize < restSizeInBlock ? leftSize : restSizeInBlock;
			memcpy(curBlock.buf + blockOffset, pInputData, copySize);
			if (!m_InOverrideMode)
			{
				curBlock.data_size += copySize;
			}
			pInputData += copySize;
			blockOffset += copySize;
			leftSize -= copySize;
			if (leftSize > 0)//need next block
			{
				blockOffset = 0;
				curBlockIndex++;
			}
		}
		curPos.blockIndex = curBlockIndex;
		curPos.offset = blockOffset;
		if (!m_InOverrideMode)
		{
			m_size += size;
		}
		return true;
	}

	STREAM_SIZE XLangStream::CalcSize(blockIndex pos)
	{
		if (BlockNum() <= pos.blockIndex)
		{
			return -1;
		}
		STREAM_SIZE size = 0;
		for (int i = 0; i < pos.blockIndex; i++)
		{
			blockInfo& curBlock = GetBlockInfo(i);
			size += curBlock.data_size;
		}
		//last block use offset,not datasize
		size += pos.offset;
		return size;
	}

	STREAM_SIZE XLangStream::CalcSize()
	{
		auto blkNum = BlockNum();
		STREAM_SIZE size = 0;
		for (int i = 0; i < blkNum; i++)
		{
			blockInfo& curBlock = GetBlockInfo(i);
			size += curBlock.data_size;
		}
		return size;
	}
	void XLangStream::Refresh()
	{
		if (m_pProvider)
		{
			m_pProvider->Refresh();
		}
	}

	int XLangStream::BlockNum()
	{
		return m_pProvider ? m_pProvider->BlockNum() : 0;
	}

	blockInfo& XLangStream::GetBlockInfo(int index)
	{
		if (m_pProvider)
		{
			return m_pProvider->GetBlockInfo(index);
		}
		else
		{
			static blockInfo blk = { 0 };
			return blk;
		}
	}

	bool XLangStream::NewBlock()
	{
		return m_pProvider ? m_pProvider->NewBlock() : false;
	}

	bool XLangStream::MoveToNextBlock()
	{
		return m_pProvider ? m_pProvider->MoveToNextBlock() : false;
	}
	XLangStream& XLangStream::operator<<(X::Value v)
	{
		auto t = v.GetType();
		(*this) << (char)t;
		switch (t)
		{
		case X::ValueType::Invalid:
			break;
		case X::ValueType::None:
			break;
		case X::ValueType::Int64:
			(*this) << v.GetLongLong();
			break;
		case X::ValueType::Double:
			(*this) << v.GetDouble();
			break;
		case X::ValueType::Object:
		{
			X::Data::Object* pObj = dynamic_cast<X::Data::Object*>(v.GetObj());
			//save pObj as Id and with a flag to indicate it's a object embeded here or just
			//a reference to an object
			unsigned long long id = pObj->ID();
			(*this) << id;
			bool bRef = (m_scope_space->Query(id) != nullptr);
			(*this) << bRef;
			if (!bRef)
			{
				//add here to avoid recursive call with ToBytes in this object
				m_scope_space->Add(id, pObj);
				(*this) << (char)pObj->GetType();
				pObj->ToBytes(m_scope_space->m_rt, m_scope_space->m_pContext, *this);
			}
		}
		break;
		case X::ValueType::Str:
			(*this) << v.ToString();
			break;
		default:
			break;
		}
		return *this;

	}
	XLangStream& XLangStream::operator>>(X::Value& v)
	{
		char ch;
		(*this) >> ch;
		X::ValueType t = (X::ValueType)ch;
		v.SetType(t);
		switch (t)
		{
		case X::ValueType::Invalid:
			break;
		case X::ValueType::None:
			break;
		case X::ValueType::Int64:
		{
			long long l;
			(*this) >> l;
			v.SetLongLong(l);
		}
			break;
		case X::ValueType::Double:
		{
			double d;
			(*this) >> d;
			v.SetDouble(d);
		}
			break;
		case X::ValueType::Object:
		{
			X::Data::Object* pObjToRestore = nullptr;
			unsigned long long id;
			(*this) >> id;
			bool bRef;
			(*this) >> bRef;
			if (bRef)
			{
				pObjToRestore = (X::Data::Object*)m_scope_space->Query(id);
				v = X::Value(dynamic_cast<XObj*>(pObjToRestore), true);
			}
			else
			{
				bool needToCallFromBytesFunc = true;
				(*this) >> ch;
				X::ObjType objT = (X::ObjType)ch;
				switch (objT)
				{
				case X::ObjType::Str:
					pObjToRestore = dynamic_cast<X::Data::Object*>(new X::Data::Str());
					pObjToRestore->IncRef();
					break;
				case X::ObjType::Binary:
					pObjToRestore = dynamic_cast<X::Data::Object*>(new X::Data::Binary(nullptr, 0, true));
					pObjToRestore->IncRef();
					break;
				case X::ObjType::Expr:
					pObjToRestore = dynamic_cast<X::Data::Object*>(new X::Data::Expr(nullptr));
					pObjToRestore->IncRef();
					break;
				case X::ObjType::Function:
					pObjToRestore = dynamic_cast<X::Data::Object*>(new X::Data::Function());
					pObjToRestore->IncRef();
					break;
				case X::ObjType::MetaFunction:
					assert(false);
					break;
				case X::ObjType::XClassObject:
					pObjToRestore = dynamic_cast<X::Data::Object*>(new X::Data::XClassObject());
					pObjToRestore->IncRef();
					break;
				case X::ObjType::DeferredObject:
					pObjToRestore = dynamic_cast<X::Data::Object*>(new X::Data::DeferredObject());
					pObjToRestore->IncRef();
					break;
				case X::ObjType::FuncCalls:
					pObjToRestore = dynamic_cast<X::Data::Object*>(new X::Data::FuncCalls());
					pObjToRestore->IncRef();
					break;
				case X::ObjType::Package:
				{
					//for package, we have 8 bytes embededID followed by a bool flag
					//to indicate if this is a reference to a package or a embeded package
					unsigned long long embedId;
					(*this) >> embedId;
					bool bRefPackObj = false;
					(*this) >> bRefPackObj;
					if (bRefPackObj)
					{
						pObjToRestore = (X::Data::Object*)m_scope_space->Query(embedId);
						needToCallFromBytesFunc = false;
						break; //break the switch
					}

					//pair with PackageProxy::ToBytes
					std::string strPackUri;
					(*this) >> strPackUri;
					X::Value varPackCreate = g_pXHost->CreatePackageWithUri(strPackUri.c_str(),this);
					if (varPackCreate.IsObject())
					{
						pObjToRestore = dynamic_cast<X::Data::Object*>(varPackCreate.GetObj());
						m_scope_space->Add(embedId, (void*)pObjToRestore);
						//this is workaround for package in on side is a pacakge,
						//but in the other side is a changed to remote object
						if (pObjToRestore->GetType() != X::ObjType::Package)
						{
							//read out the size
							long long skip_size = 0;
							(*this) >> skip_size;
							if (skip_size)
							{
								Skip(skip_size);
							}
							//Get a changed object instead of a package,we need to skip the following bytes
							needToCallFromBytesFunc = false;
						}
						pObjToRestore->IncRef();
					}
				}
				break;
				case X::ObjType::ModuleObject:
					pObjToRestore = dynamic_cast<X::Data::Object*>(new X::AST::ModuleObject(nullptr));
					pObjToRestore->IncRef();
					break;
				case X::ObjType::Future:
					assert(false);
					break;
				case X::ObjType::List:
					pObjToRestore = dynamic_cast<X::Data::Object*>(new X::Data::List());
					pObjToRestore->IncRef();
					break;
				case X::ObjType::Dict:
					pObjToRestore = dynamic_cast<X::Data::Object*>(new X::Data::Dict());
					pObjToRestore->IncRef();
					break;
				case X::ObjType::Struct:
					pObjToRestore = dynamic_cast<X::Data::Object*>(new X::Data::XlangStruct());
					pObjToRestore->IncRef();
					break;
				case X::ObjType::TableRow:
					assert(false);
					break;
				case X::ObjType::Table:
					assert(false);
					break;
#if not defined(BARE_METAL)
				case X::ObjType::RemoteObject:
					pObjToRestore = dynamic_cast<X::Data::Object*>(new X::RemoteObject(nullptr));
					pObjToRestore->IncRef();
					break;
				case X::ObjType::RemoteClientObject:
					pObjToRestore = dynamic_cast<X::Data::Object*>(new X::RemoteClientObject(nullptr));
					pObjToRestore->IncRef();
					break;
#endif
				case X::ObjType::PyProxyObject:
					assert(false);
					break;
				default:
					break;
				}
				m_scope_space->Add(id, (void*)pObjToRestore);
				if (pObjToRestore)
				{
					if (needToCallFromBytesFunc)
					{
						pObjToRestore->FromBytes(*this);
					}
					v = X::Value(dynamic_cast<XObj*>(pObjToRestore), false);
				}
			}
		}
		break;
		case X::ValueType::Str:
		{
			std::string s;
			(*this) >> s;
			v.SetString(s);
		}
			break;
		default:
			break;
		}
		return *this;

	}
}
