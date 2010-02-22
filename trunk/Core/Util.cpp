#include "stdafx.h"
#include <stdarg.h>
#include "../Core/Util.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	string strtoupper(const string& _strInput)
	{
		string strOutput = _strInput;

		struct ToUpperFunction
		{
			ToUpperFunction() {}
			char operator() (const char _cValue)
			{
				if (('a' <= _cValue) && ('z' >= _cValue))
				{
					return (_cValue - ('a' - 'A'));
				}
				return _cValue;
			}
		};
		transform(_strInput.begin(), _strInput.end(), strOutput.begin(), ToUpperFunction());

		return strOutput;
	}

	string strtolower(const string& _strInput)
	{
		string strOutput = _strInput;

		struct ToLowerFunction
		{
			ToLowerFunction() {}
			char operator() (const char _cValue)
			{
				if (('A' <= _cValue) && ('Z' >= _cValue))
				{
					return (('a' - 'A') + _cValue);
				}
				return _cValue;
			}
		};
		transform(_strInput.begin(), _strInput.end(), strOutput.begin(), ToLowerFunction());

		return strOutput;
	}
	
	void vsoutput(const char* pFormat, ...)
	{
#if _UNICODE
		const size_t uBufferSize = 1024;
		char szbuffer[uBufferSize];
		va_list vaArgs;
		const char* pbuffer = &szbuffer[0];

		va_start(vaArgs, pFormat);
		vsnprintf_s(szbuffer, uBufferSize, _TRUNCATE, pFormat, vaArgs);
		va_end(vaArgs);

		wchar_t wszbuffer[uBufferSize];
		mbsrtowcs(wszbuffer, &pbuffer, strlen(szbuffer) + 1, NULL);

		OutputDebugString(wszbuffer);
#else // _UNICODE
#endif // _UNICODE
	}
}
