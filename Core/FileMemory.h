#ifndef __FILEMEMORY_H__
#define __FILEMEMORY_H__

#include "../Core/File.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class FSMemory;
	typedef FSMemory* FSMemoryPtr;
	typedef FSMemory& FSMemoryRef;

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class FileMemory : public File
	{
	public:
		struct CreateInfo
		{
			string			m_strPath;
			FS::EOpenMode	m_eOpenMode;
		};
		typedef CreateInfo* CreateInfoPtr;

	public:
		FileMemory();
		virtual ~FileMemory();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		virtual int Size();
		virtual int Tell();
		virtual int Seek(const int& _sOffset, const ESeekMode& _eMode);

		virtual unsigned int Read(VoidPtr _pBuffer, const int& _sBytes);
		virtual unsigned int Write(const VoidPtr _pBuffer, const int& _sBytes);

		virtual bool EndOfFile();

	protected:
		static const int s_sBlockSize = 4096;

		string			m_strPath;
		FS::EOpenMode	m_eOpenMode;
		File*			m_pFile;
		BytePtr			m_pBuffer;
		int				m_sMaxSize;
		int				m_sCurrentSize;
		int				m_sCurrentPos;
		int				m_sWriteCount;
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class FSMemory : public FS
	{
	public:
		FSMemory();
		virtual ~FSMemory();

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		virtual FilePtr OpenFile(const string& _strPath, const EOpenMode& _eMode);
		virtual void CloseFile(FilePtr _pFile);
	};
}

#endif // __FILEMEMORY_H__
