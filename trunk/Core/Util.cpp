#include "stdafx.h"
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
}
