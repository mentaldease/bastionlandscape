#ifndef __FILENATIVE_H__
#define __FILENATIVE_H__

#include "../Core/File.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class FileNative : public File
	{
	public:
		struct CreateInfo
		{
			string			m_strPath;
			FS::EOpenMode	m_eOpenMode;
		};
		typedef CreateInfo* CreateInfoPtr;

	public:
		FileNative();
		virtual ~FileNative();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		virtual int Size();
		virtual int Tell();
		virtual int Seek(const int& _sOffset, const ESeekMode& _eMode);

		virtual unsigned int Read(VoidPtr _pBuffer, const int& _sBytes);
		virtual unsigned int Write(const VoidPtr _pBuffer, const int& _sBytes);

	protected:
		FILE*	m_pFile;

	private:
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class FSNative : public FS
	{
	public:
		FSNative();
		virtual ~FSNative();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		virtual FilePtr OpenFile(const string& _strPath, const EOpenMode& _eMode);
		virtual void CloseFile(FilePtr _pFile);

	protected:
	private:
	};
}

#endif // __FILENATIVE_H__
