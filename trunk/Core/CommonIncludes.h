#ifndef __COMMONINCLUDES_H__
#define __COMMONINCLUDES_H__

#ifndef D3D_DEBUG_INFO
#define D3D_DEBUG_INFO
#endif // D3D_DEBUG_INFO

// Windows Header Files:
#include <windows.h>
#include <windowsx.h>
#include <d3dx9.h>
#include <dinput.h>

// C RunTime Header Files
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <assert.h>

// stl
#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <iostream>
using namespace std;

// boost
#define NBOOST_MCMEM // special define to remove some modified boost files
#include <boost/bind.hpp>
#include <boost/functional/hash.hpp>
#include <boost/function.hpp>
#include <boost/format.hpp>


#endif // __COMMONINCLUDES_H__