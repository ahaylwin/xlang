#pragma once
#include "singleton.h"
#include "xpackage.h"
#include "xlang.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sys/stat.h>

#include "folder.h"

namespace X
{
	class File
	{
		std::ifstream m_stream;
		std::ofstream m_wstream;
		std::string m_fileName;
		bool m_IsBinary = true;
		bool m_IsWrite = false;
	public:
		BEGIN_PACKAGE(File)
			APISET().AddFunc<1>("read", &File::read);
			APISET().AddFunc<1>("write", &File::write);
			APISET().AddFunc<0>("close", &File::close);
			APISET().AddProp("size", &File::get_size);
		END_PACKAGE
		File()
		{
		}
		File(std::string fileName, std::string mode);
		~File()
		{
			if (m_stream.is_open())
			{
				m_stream.close();
			}
			if (m_wstream.is_open())
			{
				m_wstream.close();
			}
		}
		X::Value read(long long size)
		{
			if (size <= 0)
			{
				return X::Value();
			}
			if (m_IsBinary)
			{
				char* data = new char[size];
				m_stream.read(data, size);
				return X::Value(g_pXHost->CreateBin(data, size,true),false);
			}
			else
			{
				auto* pStr = g_pXHost->CreateStr(nullptr, (int)size);
				char* data = pStr->Buffer();
				m_stream.read(data, size);
				return X::Value(pStr,false);
			}
		}
		bool write(X::Value p)
		{
			if (p.IsObject() && p.GetObj()->GetType() == X::ObjType::Binary)
			{
				XBin* pBin = dynamic_cast<XBin*>(p.GetObj());
				m_wstream.write(pBin->Data(), pBin->Size());
			}
			else
			{
				std::string str = p.ToString();
				std::streamsize  len = str.length();
				m_wstream.write(str.c_str(), len);
			}

			return true;
		}
		bool close()
		{
			if (m_stream.is_open())
			{
				m_stream.close();
			}
			if (m_wstream.is_open())
			{
				m_wstream.close();
			}
			return true;
		}
		X::Value get_size()
		{
			struct stat stat_buf;
			int rc = stat(m_fileName.c_str(), &stat_buf);
			size_t size = -1;
			if (rc == 0)
			{
				size = stat_buf.st_size;
			}
			return X::Value((long long)size);
		}
	};

	class FileSystem:
		public Singleton<FileSystem>
	{
		X::Value m_curModule;
		std::string m_curModulePath;
	public:
		void Run()
		{
#if 0
			std::string code = R"(
				xyz =10
				abc ="this is a string"
				def init(x,y):
					f = File("1.txt","w")
					f.close()
					return x+y
			)";
			APISET().GetPack()->RunCodeWithThisScope(code.c_str());
#endif
		}
		void SetModule(X::Value curModule)
		{
			X::XModule* pModule = dynamic_cast<X::XModule*>(curModule.GetObj());
			if (pModule)
			{
				auto path = pModule->GetPath();
				m_curModulePath = path;
				g_pXHost->ReleaseString(path);
			}
			m_curModule = curModule;
			//std::cout << "*****SetModule****" << std::endl;
		}
		std::string& GetModulePath()
		{
			return m_curModulePath;
		}
		void SetModulePath(std::string& path)
		{
			m_curModulePath = path;
		}
		X::Value& GetModule() 
		{ 
			return m_curModule; 
		}
		BEGIN_PACKAGE(FileSystem)
			APISET().AddClass<2, File>("File");
			APISET().AddClass<1, Folder>("Folder");
		END_PACKAGE
	};
}