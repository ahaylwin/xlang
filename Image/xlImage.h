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
#include "singleton.h"
#include "xpackage.h"
#include "xlang.h"
namespace X {
	namespace Images {
		class Image
		{
			std::string m_url;
			Image* m_pimpl = nullptr;
		public:
			BEGIN_PACKAGE(Image)
				APISET().AddFunc<1>("to_tensor", &Image::To_Tensor, "image.to_tensor()");
				APISET().AddVarFunc("from_tensor", &Image::From_Tensor,"image.from_tensor()");
				APISET().AddFunc<0>("save", &Image::Save, "image.save()");
				END_PACKAGE
			Image()
			{

			}
			Image(std::string url);
			bool Init();
			~Image()
			{
				if (m_pimpl)
				{
					delete m_pimpl;
				}
			}
			virtual bool Save()
			{
				if (m_pimpl)
				{
					return m_pimpl->Save();
				}
				else
				{
					return false;
				}
			}
			virtual X::Value To_Tensor(int pixelFmt)
			{
				if (m_pimpl)
				{
					return m_pimpl->To_Tensor(pixelFmt);
				}
				else
				{
					return X::Value();
				}
			}
			virtual bool From_Tensor(X::XRuntime* rt, X::XObj* pContext,
				ARGS& params,
				KWARGS& kwParams,
				X::Value& retValue)
			{
				if (m_pimpl)
				{
					return m_pimpl->From_Tensor(rt,pContext,params,kwParams,retValue);
				}
				else
				{
					return false;
				}
			}
		};

	}
}