#ifndef __FILE_H__
#define __FILE_H__

#include "../Core/Core.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class File : public CoreObject
	{
	public:
		enum ESeekMode
		{
			ESeekMode_SET,
			ESeekMode_CUR,
			ESeekMode_END
		};

	public:
		File();
		virtual ~File();

		virtual int Size() = 0;
		virtual int Tell() = 0;
		virtual int Seek(const int& _sOffset, const ESeekMode& _eMode) = 0;

		virtual unsigned int Read(VoidPtr _pBuffer, const int& _sBytes) = 0;
		virtual unsigned int Write(const VoidPtr _pBuffer, const int& _sBytes) = 0;

		virtual bool EndOfFile() = 0;
	};

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	class FS : public CoreObject
	{
	public:
		enum EOpenMode
		{
			EOpenMode_UNKNOWN,
			EOpenMode_CREATE		= 0x00000001 << 0,
			EOpenMode_APPEND		= 0x00000001 << 1,
			EOpenMode_READ			= 0x00000001 << 2,
			EOpenMode_WRITE			= 0x00000001 << 3,
			EOpenMode_TEXT			= 0x00000001 << 4,
			EOpenMode_BINARY		= 0x00000001 << 5,
			EOpenMode_READTEXT		= EOpenMode_READ | EOpenMode_TEXT,
			EOpenMode_READBINARY	= EOpenMode_READ | EOpenMode_BINARY,
			EOpenMode_CREATETEXT	= EOpenMode_CREATE | EOpenMode_WRITE | EOpenMode_TEXT,
			EOpenMode_CREATEBINARY	= EOpenMode_CREATE | EOpenMode_WRITE | EOpenMode_BINARY,
		};

	public:
		FS();
		virtual ~FS();

		static FSPtr GetRoot();
		static void SetRoot(FSPtr _pFS);

		virtual bool Create(const boost::any& _rConfig);
		virtual void Update();
		virtual void Release();

		virtual FilePtr OpenFile(const string& _strPath, const EOpenMode& _eMode);
		virtual void CloseFile(FilePtr _pFile);

		virtual bool AddFS(const string& _strFSName, FSPtr _pFS);
		virtual void RemoveFS(const string& _strFSName, FSPtr _pFS);

		virtual FSPtr GetFS(const string& _strFSName);
		virtual FSPtr GetFSFromPath(const string& _strPath);

		static void GetPathWithoutFS(const string& _strSrcPath, string& _strDstPath);
		static void GetFileNameWithoutExt(const string& _strSrcPath, string& _strDstPath);
		static void GetFileExt(const string& _strSrcPath, string& _strDstExt);
		static void GetPathWithoutFileName(const string& _strSrcPath, string& _strDstPath, const bool bKeepEndingDirSeparator);
		static void ComposePath(string& _strDstPath, const string& _strSrcPath1, const string& _strSrcPath2);

		const static char s_FSMarkerInPath = '@';
		const static char s_WDirSeparator = '\\';
		const static char s_UDirSeparator = '/';
		const static char s_ExtSeparator = '.';

	protected:
		static FSPtr	s_pRootFS;

		FSPtrVec		m_vSubFS;
		FSPtrMap		m_mSubFS;
		OpenFileMap		m_mFiles;

	private:
	};
}

#endif // __FILE_H__