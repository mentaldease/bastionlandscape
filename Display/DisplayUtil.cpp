#include "stdafx.h"
#include "../Display/Display.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	unsigned int Display::GetFormatBitsPerPixel(const D3DFORMAT& _eFormat)
	{
		switch (_eFormat)
		{
			//case D3DFMT_UNKNOWN:
			case D3DFMT_R8G8B8:
			{
				return 24;
			}
			case D3DFMT_A8R8G8B8:
			case D3DFMT_X8R8G8B8:
			case D3DFMT_A2B10G10R10:
			case D3DFMT_A8B8G8R8:
			case D3DFMT_X8B8G8R8:
			case D3DFMT_G16R16:
			case D3DFMT_A2R10G10B10:
			case D3DFMT_X8L8V8U8:
			case D3DFMT_Q8W8V8U8:
			case D3DFMT_V16U16:
			case D3DFMT_A2W10V10U10:
			case D3DFMT_D32:
			case D3DFMT_D24S8:
			case D3DFMT_D24X8:
			case D3DFMT_D24X4S4:
			case D3DFMT_D32F_LOCKABLE:
			case D3DFMT_D24FS8:
		#if !defined(D3D_DISABLE_9EX)
			case D3DFMT_D32_LOCKABLE:
		#endif // !D3D_DISABLE_9EX
			case D3DFMT_INDEX32:
			case D3DFMT_G16R16F:
			case D3DFMT_R32F:
			{
				return 32;
			}
			case D3DFMT_R5G6B5:
			case D3DFMT_X1R5G5B5:
			case D3DFMT_A1R5G5B5:
			case D3DFMT_A4R4G4B4:
			case D3DFMT_A8R3G3B2:
			case D3DFMT_X4R4G4B4:
			case D3DFMT_A8L8:
			case D3DFMT_V8U8:
			case D3DFMT_L6V5U5:
			case D3DFMT_R8G8_B8G8:
			case D3DFMT_G8R8_G8B8:
			case D3DFMT_D16_LOCKABLE:
			case D3DFMT_D15S1:
			case D3DFMT_D16:
			case D3DFMT_L16:
			case D3DFMT_INDEX16:
			case D3DFMT_R16F:
			case D3DFMT_CxV8U8:
			{
				return 16;
			}
			case D3DFMT_R3G3B2:
			case D3DFMT_A8:
			case D3DFMT_A8P8:
			case D3DFMT_P8:
			case D3DFMT_L8:
			case D3DFMT_A4L4:
		#if !defined(D3D_DISABLE_9EX)
			case D3DFMT_S8_LOCKABLE:
		#endif // !D3D_DISABLE_9EX
			{
				return 8;
			}
			case D3DFMT_A16B16G16R16:
			case D3DFMT_G32R32F:
			case D3DFMT_Q16W16V16U16:
			case D3DFMT_A16B16G16R16F:
			{
				return 64;
			}
			case D3DFMT_A32B32G32R32F:
			{
				return 128;
			}
		#if !defined(D3D_DISABLE_9EX)
			case D3DFMT_A1:
			{
				return 1;
			}
		#endif // !D3D_DISABLE_9EX
			case D3DFMT_UYVY:
			case D3DFMT_YUY2:
			case D3DFMT_DXT1:
			case D3DFMT_DXT2:
			case D3DFMT_DXT3:
			case D3DFMT_DXT4:
			case D3DFMT_DXT5:
			case D3DFMT_VERTEXDATA:
			case D3DFMT_MULTI2_ARGB8:
		#if !defined(D3D_DISABLE_9EX)
			case D3DFMT_BINARYBUFFER:
		#endif // !D3D_DISABLE_9EX
			{
				return 0;
			}
		}
		return 0;
	}

	bool Display::IsPowerOf2(const unsigned int& _uValue, UIntPtr _pPowerLevel)
	{
		unsigned int uTemp = _uValue;
		unsigned int uBitsCount = 0;
		bool bResult = false;

		if (NULL == _pPowerLevel)
		{
			while (1 != uTemp)
			{
				uBitsCount = (0x1 == (0x1 & uTemp)) ? (uBitsCount + 1) : uBitsCount;
				uTemp >>= 1;
			}
			uBitsCount = (0x1 == (0x1 & uTemp)) ? (uBitsCount + 1) : uBitsCount;
		}
		else
		{
			(*_pPowerLevel) = 0;
			while (1 != uTemp)
			{
				uBitsCount = (0x1 == (0x1 & uTemp)) ? (uBitsCount + 1) : uBitsCount;
				uTemp >>= 1;
				++(*_pPowerLevel);
			}
			uBitsCount = (0x1 == (0x1 & uTemp)) ? (uBitsCount + 1) : uBitsCount;
		}

		bResult = (1 == uBitsCount); // is it a power of 2 number ??

		return bResult;
	}

	D3DFORMAT Display::StringToDisplayFormat(const string& _strFormatName, const D3DFORMAT& _uDefaultFormat)
	{
		if (false != s_mDisplayFormat.empty())
		{
			InitDisplayFormatMap();
		}
		Key uFormatNameKey = MakeKey(_strFormatName);
		if (s_mDisplayFormat.end() == s_mDisplayFormat.find(uFormatNameKey))
		{
			return _uDefaultFormat;
		}
		return s_mDisplayFormat[uFormatNameKey];
	}

	D3DFORMAT Display::KeyToDisplayFormat(const Key& _uFormatNameKey, const D3DFORMAT& _uDefaultFormat)
	{
		if (false != s_mDisplayFormat.empty())
		{
			InitDisplayFormatMap();
		}
		if (s_mDisplayFormat.end() == s_mDisplayFormat.find(_uFormatNameKey))
		{
			return _uDefaultFormat;
		}
		return s_mDisplayFormat[_uFormatNameKey];
	}

	void Display::InitDisplayFormatMap()
	{
		#define AddToDisplayFormatMap(Format) s_mDisplayFormat[MakeKey(string(#Format))] = Format;

		AddToDisplayFormatMap(D3DFMT_UNKNOWN);
		AddToDisplayFormatMap(D3DFMT_R8G8B8);
		AddToDisplayFormatMap(D3DFMT_A8R8G8B8);
		AddToDisplayFormatMap(D3DFMT_X8R8G8B8);
		AddToDisplayFormatMap(D3DFMT_A2B10G10R10);
		AddToDisplayFormatMap(D3DFMT_A8B8G8R8);
		AddToDisplayFormatMap(D3DFMT_X8B8G8R8);
		AddToDisplayFormatMap(D3DFMT_G16R16);
		AddToDisplayFormatMap(D3DFMT_A2R10G10B10);
		AddToDisplayFormatMap(D3DFMT_X8L8V8U8);
		AddToDisplayFormatMap(D3DFMT_Q8W8V8U8);
		AddToDisplayFormatMap(D3DFMT_V16U16);
		AddToDisplayFormatMap(D3DFMT_A2W10V10U10);
		AddToDisplayFormatMap(D3DFMT_D32);
		AddToDisplayFormatMap(D3DFMT_D24S8);
		AddToDisplayFormatMap(D3DFMT_D24X8);
		AddToDisplayFormatMap(D3DFMT_D24X4S4);
		AddToDisplayFormatMap(D3DFMT_D32F_LOCKABLE);
		AddToDisplayFormatMap(D3DFMT_D24FS8);
#if !defined(D3D_DISABLE_9EX)
		AddToDisplayFormatMap(D3DFMT_D32_LOCKABLE);
#endif // !D3D_DISABLE_9EX
		AddToDisplayFormatMap(D3DFMT_INDEX32);
		AddToDisplayFormatMap(D3DFMT_G16R16F);
		AddToDisplayFormatMap(D3DFMT_R32F);
		AddToDisplayFormatMap(D3DFMT_R5G6B5);
		AddToDisplayFormatMap(D3DFMT_X1R5G5B5);
		AddToDisplayFormatMap(D3DFMT_A1R5G5B5);
		AddToDisplayFormatMap(D3DFMT_A4R4G4B4);
		AddToDisplayFormatMap(D3DFMT_A8R3G3B2);
		AddToDisplayFormatMap(D3DFMT_X4R4G4B4);
		AddToDisplayFormatMap(D3DFMT_A8L8);
		AddToDisplayFormatMap(D3DFMT_V8U8);
		AddToDisplayFormatMap(D3DFMT_L6V5U5);
		AddToDisplayFormatMap(D3DFMT_R8G8_B8G8);
		AddToDisplayFormatMap(D3DFMT_G8R8_G8B8);
		AddToDisplayFormatMap(D3DFMT_D16_LOCKABLE);
		AddToDisplayFormatMap(D3DFMT_D15S1);
		AddToDisplayFormatMap(D3DFMT_D16);
		AddToDisplayFormatMap(D3DFMT_L16);
		AddToDisplayFormatMap(D3DFMT_INDEX16);
		AddToDisplayFormatMap(D3DFMT_R16F);
		AddToDisplayFormatMap(D3DFMT_CxV8U8);
		AddToDisplayFormatMap(D3DFMT_R3G3B2);
		AddToDisplayFormatMap(D3DFMT_A8);
		AddToDisplayFormatMap(D3DFMT_A8P8);
		AddToDisplayFormatMap(D3DFMT_P8);
		AddToDisplayFormatMap(D3DFMT_L8);
		AddToDisplayFormatMap(D3DFMT_A4L4);
#if !defined(D3D_DISABLE_9EX)
		AddToDisplayFormatMap(D3DFMT_S8_LOCKABLE);
#endif // !D3D_DISABLE_9EX
		AddToDisplayFormatMap(D3DFMT_A16B16G16R16);
		AddToDisplayFormatMap(D3DFMT_G32R32F);
		AddToDisplayFormatMap(D3DFMT_Q16W16V16U16);
		AddToDisplayFormatMap(D3DFMT_A16B16G16R16F);
		AddToDisplayFormatMap(D3DFMT_A32B32G32R32F);
#if !defined(D3D_DISABLE_9EX)
		AddToDisplayFormatMap(D3DFMT_A1);
#endif // !D3D_DISABLE_9EX
		AddToDisplayFormatMap(D3DFMT_UYVY);
		AddToDisplayFormatMap(D3DFMT_YUY2);
		AddToDisplayFormatMap(D3DFMT_DXT1);
		AddToDisplayFormatMap(D3DFMT_DXT2);
		AddToDisplayFormatMap(D3DFMT_DXT3);
		AddToDisplayFormatMap(D3DFMT_DXT4);
		AddToDisplayFormatMap(D3DFMT_DXT5);
		AddToDisplayFormatMap(D3DFMT_VERTEXDATA);
		AddToDisplayFormatMap(D3DFMT_MULTI2_ARGB8);
#if !defined(D3D_DISABLE_9EX)
		AddToDisplayFormatMap(D3DFMT_BINARYBUFFER);
#endif // !defined(D3D_DISABLE_9EX)

		#undef AddToDisplayFormatMap
	}
}
