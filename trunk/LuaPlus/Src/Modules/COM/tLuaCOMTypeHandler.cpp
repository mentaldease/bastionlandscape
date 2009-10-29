// tLuaCOMTypeHandler.cpp: implementation of the tLuaCOMTypeHandler class.
//
//////////////////////////////////////////////////////////////////////

// RCS Info
static char *rcsid = "$Id: tLuaCOMTypeHandler.cpp,v 1.30 2004/02/09 18:53:49 almendra Exp $";
static char *rcsname = "$Name:  $";


#include <ole2.h>

extern "C"
{
#include <lua.h>
#include "LuaCompat.h"
}

#include <assert.h>
#include <stdio.h>

#include "tLuaCOMTypeHandler.h"
#include "tLuaCOM.h"
#include "tLuaVector.h"
#include "tLuaCOMException.h"
#include "tCOMUtil.h"

#include "tLuaCOMEnumerator.h"

#include "tUtil.h"
#include "LuaAux.h"
#include "luabeans.h"

#include "luacom_internal.h"


#define LUA_NOOBJECT 0

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

tLuaCOMTypeHandler::tLuaCOMTypeHandler(ITypeInfo *ptypeinfo, LuaBeans *p_lbeans)
{
  lbeans = p_lbeans;
  L = lbeans->getLuaState();
  m_typeinfo = ptypeinfo;

  if(m_typeinfo)
    m_typeinfo->AddRef();
}

tLuaCOMTypeHandler::~tLuaCOMTypeHandler()
{
  COM_RELEASE(m_typeinfo);
}

void tLuaCOMTypeHandler::com2lua(VARIANTARG varg_orig)
{
  LUASTACK_SET(L);

  HRESULT hr = S_OK;

  VARIANT varg;
  VariantInit(&varg);

  // dereferences VARIANTARG (if necessary)
  hr = VariantCopyInd(&varg, &varg_orig);

  if(FAILED(hr))
    COM_ERROR(tUtil::GetErrorMessage(hr));

  // Gives a different treatment to SAFEARRAYs
  if(varg.vt & VT_ARRAY)
  {
    // treats an array of VT_UI1 as an array of char and 
    // converts it to a string
    if(varg.vt == (VT_ARRAY | VT_UI1))
      safearray2string(varg);
    else
      safearray_com2lua(varg);
  }
  else
  {


    // used in some type conversions
    VARIANTARG new_varg;
    VariantInit(&new_varg);

    try
    {
      switch (varg.vt)
      {
      case VT_EMPTY:
      case VT_NULL:      // SQL's NULL value.
        lua_pushnil(L);
        break;

      case VT_CY:
      case VT_UI1:
      case VT_UI2:
      case VT_UI4:
      case VT_INT:
      case VT_UINT:
      case VT_I1:
      case VT_I2:
      case VT_I4:
      case VT_R4:
      case VT_R8:
        {
          new_varg.vt = VT_R8;
          HRESULT hr = VariantChangeType(&new_varg, &varg, 0, VT_R8);
          CHK_COM_CODE(hr);

          lua_pushnumber(L, new_varg.dblVal);

          break;
        }

      case VT_DATE:
        {
          HRESULT hr = VariantChangeType(&new_varg, &varg, 0, VT_BSTR);
          CHK_COM_CODE(hr);

          lua_pushstring(L, (char *) tUtil::bstr2string(new_varg.bstrVal));

          break;
        }


      case VT_ERROR: 
        // assumes that a parameter has been omitted
        lua_pushnil(L);
        break;

      case VT_BOOL:
        luaCompat_pushCBool(L, varg.boolVal);
        break;

      case VT_BSTR:
        {
          const char* str = tUtil::bstr2string(varg.bstrVal);
          lua_pushstring(L, (char *) str);
        
          break;
        }

      case VT_DISPATCH:
        {
          unsigned int ninfo = 0;
          IDispatch *pdisp = varg.pdispVal;

          if(pdisp == NULL)
          {
            lua_pushnil(L);
            break;
          }

          
          tLuaCOM* lcom = NULL;

          try
          {
            lcom = tLuaCOM::CreateLuaCOM(pdisp, lbeans);
          }
          catch(class tLuaCOMException& e)
          {
            UNUSED(e);
            lua_pushnil(L);
            break;
          }

          lbeans->push(lcom);
        }
        break;

      case VT_UNKNOWN:
        {
          // first, tries to get an IDispatch. If not possible,
          // pushes pointer
          IUnknown* punk = varg.punkVal;
          IDispatch* pdisp = NULL;

          hr = punk->QueryInterface(IID_IDispatch, (void **) &pdisp);

          if(SUCCEEDED(hr))
          {
            tLuaCOM* lcom = NULL;

            try
            {
              lcom = tLuaCOM::CreateLuaCOM(pdisp, lbeans);
            }
            catch(class tLuaCOMException& e)
            {
              UNUSED(e);
              COM_RELEASE(pdisp);
              lua_pushnil(L);
              break;
            }

            COM_RELEASE(pdisp);
            lbeans->push(lcom);

            break;

          }

          IEnumVARIANT* pEV = NULL;

          hr = punk->QueryInterface(IID_IEnumVARIANT, (void **) &pEV);

          if(SUCCEEDED(hr))
          {
            tLuaCOMEnumerator* enumerator = NULL;

            try
            {
              enumerator = 
                new tLuaCOMEnumerator(L, lbeans, pEV);
            }
            catch(class tLuaCOMException& e)
            {
              UNUSED(e);

              COM_RELEASE(pEV);
              throw;
            }

            COM_RELEASE(pEV);

            enumerator->push();

            break;
          }


          // defaults to pushing and userdata for the IUnknown
          varg.punkVal->AddRef();
          pushIUnknown(varg.punkVal);

          break;
        }

      default:
        {
          static char msg[100];
        
          sprintf(msg, "COM->Lua - Type 0x%.2x not implemented.", varg.vt); 
        
          TYPECONV_ERROR(msg);
        
          break;
        }
      }
    }
    catch(class tLuaCOMException& e)
    {
      UNUSED(e);
      VariantClear(&varg);
      throw;
    }

    VariantClear(&varg);
  }

  LUASTACK_CLEAN(L, 1);
}

void tLuaCOMTypeHandler::lua2com(stkIndex luaval, VARIANTARG& varg)
{
  CHECKPARAM(luaval > 0);

  VariantClear(&varg);

  switch(lua_type(L, luaval))
  {
  case LUA_TNUMBER:
    varg.dblVal = lua_tonumber(L, luaval);
    varg.vt = VT_R8;

    break;


  case LUA_TSTRING:
    {
      const char* str = lua_tostring(L, luaval);
      long c_len = strlen(str);
      long l_len = lua_strlen(L, luaval);
      if (c_len == l_len)
      {
         varg.vt = VT_BSTR;
         varg.bstrVal = tUtil::string2bstr(str);
      }
      else
      {
        string2safearray(str, l_len, varg);
      }
    }

    break;


  case LUA_TTABLE:
    {

      // tests whether it's a LuaCOM object
      tLuaCOM* lcom = from_lua(luaval);

      if(lcom)
      {
        varg.pdispVal = lcom->GetIDispatch();
  
        varg.pdispVal->AddRef();
        varg.vt = VT_DISPATCH;
      }
      else
      {

        // in the future, we will check for custom
        // type conversions for tables with tags.
        
        safearray_lua2com(luaval, varg, VT_VARIANT);
      }

      break;
    }

    break;
  
  case LUA_TBOOLEAN:
    varg.vt = VT_BOOL;
    varg.boolVal = luaCompat_toCBool(L, luaval);
    break;


  case LUA_TUSERDATA:
    if(isIUnknown(luaval)) // is an IUnknown?
    {
      varg.vt = VT_UNKNOWN;
      varg.punkVal = (IUnknown *) luaCompat_getTypedObject(L, luaval);
      varg.punkVal->AddRef();
    }

    break;

  case LUA_TNIL:
    varg.vt = VT_EMPTY;
    break;

  case LUA_TNONE:
  default:
    TYPECONV_ERROR("No lua value to convert.");
    break;
  }
}

bool tLuaCOMTypeHandler::setRetval(const FUNCDESC * funcdesc,
                                   stkIndex luaval,
                                   VARIANTARG * pvarg_retval)
{
  VARIANT varg;

  VariantInit(&varg);

  if(funcdesc->elemdescFunc.tdesc.vt != VT_VOID)
  {
    lua2com(luaval, varg);

    initByRefParam(pvarg_retval, VT_VARIANT);
    toByRefParam(varg, pvarg_retval);
  }
  else
    return false;

  return true;
}


int tLuaCOMTypeHandler::pushOutValues(const DISPPARAMS& dispparams)
{
  unsigned int i = 0;
  int num_pushed_values = 0;
   
  // Procura valor de retorno dos parametros de saida
  for(i = 0; i < dispparams.cArgs; i++)
  {
    VARIANTARG& varg = dispparams.rgvarg[dispparams.cArgs - i - 1];

    if(varg.vt & VT_BYREF)
    {
      com2lua(varg);
      num_pushed_values++;
    }
  }

  return num_pushed_values;
}


void tLuaCOMTypeHandler::releaseVariants(DISPPARAMS *pDispParams)
{
  unsigned int i = 0;
  VARIANTARG* &vargs = pDispParams->rgvarg;

  if (vargs != NULL)
  {
    for (i = 0; i < pDispParams->cArgs; i ++)
    {
      releaseVariant(&vargs[i]);
    }

    delete [] vargs;

    vargs = NULL;
    pDispParams->cArgs = 0;
  }

}



//
// Preenche estrutura DISPPARAMS, inicializando parametros
//

void tLuaCOMTypeHandler::fillDispParams(DISPPARAMS& rDispParams,
                                        FUNCDESC * pfuncdesc,
                                        tLuaObjList& params,
                                        int invkind)
{
  // initializes DISPPARAMS
  rDispParams.cArgs = 0;
  rDispParams.cNamedArgs = 0;
  rDispParams.rgvarg = NULL;
  rDispParams.rgdispidNamedArgs = NULL;

  // if we know that this function does not received
  // parameters, stop here
  if (pfuncdesc && pfuncdesc->cParams == 0)
    return;

  static DISPID dispidNamed = DISPID_PROPERTYPUT;

  unsigned short i          = 0;
  stkIndex val              = -1;

  // references to simplify code
  unsigned int& r_cArgs   = rDispParams.cArgs; 
  VARIANTARG* &r_rgvarg   = rDispParams.rgvarg;

  // propertyput particular case
  if(invkind == DISPATCH_PROPERTYPUT ||
     invkind == DISPATCH_PROPERTYPUTREF)
  {
    rDispParams.cNamedArgs = 1;
    rDispParams.rgdispidNamedArgs = &dispidNamed;
  }

  r_cArgs = 0; // starts empty
  long lua_args = 0;
  long max_idl_params = 0;


  // first attemp to determine the size
  // of DISPPARAMS
  unsigned int max_params = 0;

  if(pfuncdesc)
    max_params = pfuncdesc->cParams;
  else
    max_params = params.getNumParams();

  // if this function can receive a variable
  // number of arguments, then the last
  // formal parameter is a safearray of these
  // arguments and must be treated separately.
  // Following the documentation, we should pass
  // a safearray containing the remaining parameters.
  // As this didn't work with MSHTML.HTMLDocument.WriteLn,
  // we choose to do the same as VBScript: pass all
  // the remaining parameters separately.
  // The code to do this the "documented" way
  // is commented some lines of code below.
  if(pfuncdesc && pfuncdesc->cParamsOpt == -1)
  {
    assert(max_params >= 1);

    // The maximum number of parameters now is
    // bounded by the size of the parameter list,
    // if it is larger than the formal parameter
    // list
    if(params.getNumParams() > pfuncdesc->cParams)
      max_params = params.getNumParams();
    else
      max_params = pfuncdesc->cParams;

    // We ignore the last parameter (the safearray
    // of variants), as we won't use it
    max_idl_params = pfuncdesc->cParams - 1;
  }
  else
    max_idl_params = max_params;

  // creates array of VARIANTs
  r_rgvarg = new VARIANTARG[max_params]; 


  bool hasdefault     = false;
  bool byref          = false;
  bool array          = false;
  VARTYPE array_type  = VT_EMPTY;
  VARTYPE type        = VT_EMPTY;

  VARIANT var, defaultValue;

  // itera no array lprgelemdescParam procurando pegar
  // os parametros da tabela lua

  VariantInit(&defaultValue);

  try
  {
    for (i = 0; i < max_params; i++)
    {
      // default values
      byref      = true;
      hasdefault = false;
      array      = false;
      type       = VT_VARIANT;

      VariantInit(&r_rgvarg[r_cArgs]);
  
      // processing that makes sense when there is type info
      // available
      if(pfuncdesc && i < max_idl_params)
      {
        PARAMDESC paramdesc = pfuncdesc->lprgelemdescParam[i].paramdesc;
        const TYPEDESC tdesc = 
          processTYPEDESC(m_typeinfo, pfuncdesc->lprgelemdescParam[i].tdesc);

        // stores type of the expected value
        type = tdesc.vt & ~VT_BYREF;

        // tests whether an array is expected,
        // storing type of array elements if so.
        if(tdesc.vt & VT_ARRAY)
        {
          array = true;
          array_type = type & ~VT_ARRAY;
        }

        // ignores out parameters
        if(!(paramdesc.wParamFlags &  PARAMFLAG_FIN) &&
            (paramdesc.wParamFlags != PARAMFLAG_NONE))
        {
          r_cArgs++;
          continue;
        }
        else if(
                 (paramdesc.wParamFlags & PARAMFLAG_FOUT) ||
                 (
                   paramdesc.wParamFlags == PARAMFLAG_NONE && 
                   (tdesc.vt & VT_BYREF)
                 )
               )
        {         
          // assumes that it is an in/out parameter
          byref = true;
        }
        else // in param
          byref = false;

        // deals with default values (if any)
        if(paramdesc.wParamFlags & PARAMFLAG_FHASDEFAULT)
        {
          hasdefault = true;
          VariantCopy(
            &defaultValue,
            &paramdesc.pparamdescex->varDefaultValue);
        }
      }

      // gets value from lua
      val = params.getparam(lua_args);

      // Converts to VARIANT
      VariantInit(&var);

      if(val != 0 && lua_type(L, val) != LUA_TNONE && !lua_isnil(L, val)) 
      {
        if(array)
          safearray_lua2com(val, var, array_type);
        else
          lua2com(val, var);
      }
      else if(hasdefault)
      {
        VariantCopy(&var, &defaultValue);
        VariantClear(&defaultValue);
      }
      else
      {
        // assumes that a parameter is expected but has not been found

        var.vt = VT_ERROR;
        var.scode = DISP_E_PARAMNOTFOUND;
      }

      if(!byref || var.vt == VT_ERROR)
      {
        VariantCopy(&r_rgvarg[i], &var);
        VariantClear(&var);
      }
      else
      {
        initByRefParam(&r_rgvarg[i], type);
        toByRefParam(var, &r_rgvarg[i]);
      }

      r_cArgs++;
      lua_args++;
    }

    /* 
    // deals with vararg functions following 
    // vararg documentation
    if(pfuncdesc && pfuncdesc->cParamsOpt == -1)
    {
      safearray_lua2com(
        params.getparam(lua_args),
        r_rgvarg[r_cArgs],
        VT_VARIANT,
        true);

      r_cArgs++;
    }
    */
  }
  catch(class tLuaCOMException& e)
  {
    UNUSED(e);

    delete r_rgvarg;
    r_rgvarg = NULL;
    throw;
  }

  // inverts parameters order to match
  // what is expected by Automation
  if(r_cArgs > 0)
  {
    VARIANTARG temp;

    for(i = 0; i < r_cArgs/2; i++)
    {
      temp = r_rgvarg[i];
      r_rgvarg[i] = r_rgvarg[r_cArgs - i - 1]; 
      r_rgvarg[r_cArgs - i - 1] = temp;
    }
  }

  return;
}


void tLuaCOMTypeHandler::pushLuaArgs(DISPPARAMS* pDispParams,
                                     const ELEMDESC* pElemDesc)
{
  unsigned int arg = 0;
  VARIANT var;
  HRESULT hr = S_OK;
  unsigned int ArgErr = 0;

  const unsigned int first_named_param = 
    pDispParams->cArgs - pDispParams->cNamedArgs;

  VariantInit(&var);

  for(arg = 0; arg < pDispParams->cArgs; arg++)
  {
    const USHORT& wParamFlags = pElemDesc[arg].paramdesc.wParamFlags;
    const TYPEDESC tdesc = processTYPEDESC(m_typeinfo, pElemDesc[arg].tdesc);

    // if parameter is not an out param, convert it to Lua
    if(wParamFlags & PARAMFLAG_FIN || wParamFlags == PARAMFLAG_NONE )
    {

      // First we have to find the right index in the
      // rgvarg array, as we must deal with positional and
      // named parameters

      int index = -1;

      if(arg < first_named_param) // still inside positional parameters
        index = pDispParams->cArgs - arg - 1; 
      else // now we are dealing with named parameters
      {
        // tries to find a named param for the current position
        for(unsigned int i = 0; i < pDispParams->cNamedArgs; i++)
        {
          if(pDispParams->rgdispidNamedArgs[i] == arg)
          {
            index = i;
            break;
          }
        }

        if(index == -1) // no corresponding named param found
        {
          lua_pushnil(L);
          continue;
        }
      }

      VARIANTARG& varg = pDispParams->rgvarg[index];    

      // we assume that empty parameters with paramflags NONE
      // are out params
      if(varg.vt == VT_EMPTY && wParamFlags == PARAMFLAG_NONE)
        continue;


      if(varg.vt == VT_ERROR || varg.vt == VT_EMPTY)
      {
        // parameter has been omitted
        lua_pushnil(L);
        continue;
      }

      // removes indirections
      hr = VariantCopyInd(&var, &varg);
      CHK_COM_CODE(hr);

      // some type checks
      if((tdesc.vt & ~(VT_BYREF)) != VT_VARIANT &&
         V_ISARRAY(&tdesc) != V_ISARRAY(&var))
        CHK_COM_CODE(DISP_E_TYPEMISMATCH);
      else if(!V_ISARRAY(&var))
      {
        // coerces value to the expected scalar type
        Coerce(var, var, (tdesc.vt & ~VT_BYREF));
      }

      com2lua(var);
      VariantClear(&var);
    }
  }
}

void tLuaCOMTypeHandler::setOutValues(FUNCDESC * pFuncDesc,
                                      DISPPARAMS * pDispParams,
                                      stkIndex outvalue
                                      )
{
  CHECKPRECOND(outvalue > 0);

  const unsigned int num_args = pDispParams->cArgs;
  const unsigned int first_named_param = pDispParams->cArgs - pDispParams->cNamedArgs;
  unsigned int arg = 0;

  VARIANT var;
  VariantInit(&var);

  // Procura valor de retorno dos parametros de saida
  for(arg = 0; arg < num_args; arg++)
  {
    // checks whether there are more return values to map
    // to out parameters
    if(lua_type(L, outvalue) == LUA_TNONE)
      break;

    // alias
    const TYPEDESC tdesc = 
      processTYPEDESC(m_typeinfo, pFuncDesc->lprgelemdescParam[arg].tdesc);

    // tests whether this parameters is an out parameter
    if(V_ISBYREF(&tdesc))
    {
      // tries to find the right position in the rgvarg array,
      // when using named args
      unsigned int index = -1;

      if(arg < first_named_param) // still inside positional parameters
        index = pDispParams->cArgs - arg - 1; 
      else // now we are dealing with named parameters
      {
        // tries to find a named param for the current position
        for(unsigned int i = 0; i < pDispParams->cNamedArgs; i++)
        {
          if(pDispParams->rgdispidNamedArgs[i] == arg)
          {
            index = i;
            break;
          }
        }

        if(index == -1) 
        {
          // no corresponding named param found, so we must skip
          // this one
          outvalue++;
          continue;
        }
      }


      // alias
      VARIANTARG& varg_orig = pDispParams->rgvarg[index];

      // does indirection in the case of VARIANTs
      if(varg_orig.vt == (VT_VARIANT | VT_BYREF))
      {
        VARIANTARG& varg = *varg_orig.pvarVal;

        if(varg.vt & VT_ARRAY)
          safearray_lua2com(outvalue, var, varg.vt & ~(VT_BYREF | VT_ARRAY));
        else
          lua2com(outvalue, var);

        if(varg.vt & VT_BYREF)
          toByRefParam(var, &varg);
        else
          VariantCopy(&varg, &var);
      }
      else
      {
        VARIANTARG& varg = varg_orig;

        if(varg.vt == VT_ERROR || varg.vt == VT_EMPTY)
          initByRefParam(&varg, tdesc.vt & ~VT_BYREF);

        if(varg.vt & VT_ARRAY)
          safearray_lua2com(outvalue, var, varg.vt & ~(VT_BYREF | VT_ARRAY));
        else
          lua2com(outvalue, var);

        toByRefParam(var, &varg);
      }

      outvalue++;
    }
  }
}

//
// Conversao de Safe Arrays para tabelas lua e vice versa
//

//  funcoes auxiliares

// funcoes auxiliares de safearray_lua2com




SAFEARRAYBOUND* tLuaCOMTypeHandler::getRightOrderedBounds(
    SAFEARRAYBOUND *bounds, 
    unsigned long num_dimensions)
{
  SAFEARRAYBOUND* new_bounds = new SAFEARRAYBOUND[num_dimensions];

  unsigned long i = 0;

  for(i = 0; i < num_dimensions; i++)
    new_bounds[i] = bounds[num_dimensions - i - 1];

  return new_bounds;
}


void tLuaCOMTypeHandler::put_in_array(SAFEARRAY* safearray,
                         VARIANT var,
                         long* indices,
                         VARTYPE vt
                         )
{
  HRESULT hr = S_OK;
  VARIANT var_value;

  // converts to the right type
  Coerce(var, var, vt);

  // does a copy to avoid problems
  VariantInit(&var_value);
  VariantCopy(&var_value, &var);

  switch(vt)
  {
  case VT_I2:
    hr = SafeArrayPutElement(safearray, indices, &var_value.iVal);
    break;

  case VT_I4:
    hr = SafeArrayPutElement(safearray, indices, &var_value.lVal);
    break;
    
  case VT_R4:
    hr = SafeArrayPutElement(safearray, indices, &var_value.fltVal);
    break;

  case VT_R8:
    hr = SafeArrayPutElement(safearray, indices, &var_value.dblVal);
    break;

  case VT_CY:
    hr = SafeArrayPutElement(safearray, indices, &var_value.cyVal);
    break;

  case VT_DATE:
    hr = SafeArrayPutElement(safearray, indices, &var_value.date);
    break;

  case VT_BSTR:
    // BSTR already a pointer
    hr = SafeArrayPutElement(safearray, indices, var_value.bstrVal);
    break;

  case VT_DISPATCH:
    // IDispatch already is a pointer (no indirection need)
    hr = SafeArrayPutElement(safearray, indices, var_value.pdispVal);
    VariantClear(&var_value);
    break;

  case VT_ERROR:
    hr = SafeArrayPutElement(safearray, indices, &var_value.scode);
    break;

  case VT_BOOL:
    hr = SafeArrayPutElement(safearray, indices, &var_value.boolVal);
    break;

  case VT_VARIANT:
    hr = SafeArrayPutElement(safearray, indices, &var_value);
    break;

  case VT_UNKNOWN:
    // see IDispatch
    hr = SafeArrayPutElement(safearray, indices, var_value.punkVal);
    VariantClear(&var_value);
    break;

  case VT_DECIMAL:
    hr = SafeArrayPutElement(safearray, indices, &var_value.decVal);
    break;

  case VT_UI1:
    hr = SafeArrayPutElement(safearray, indices, &var_value.bVal);
    break;

  case VT_INT:
    hr = SafeArrayPutElement(safearray, indices, &var_value.intVal);
    break;

  case VT_ARRAY:
    TYPECONV_ERROR("SAFEARRAY of SAFEARRAYS not allowed");
    break;

  case VT_I1:
  case VT_UI2:
  case VT_UI4:
  case VT_UINT:
  default:
    TYPECONV_ERROR("Type not compatible with automation.");
    break;
  }
 
  CHK_COM_CODE(hr);
}

stkIndex tLuaCOMTypeHandler::get_from_array(SAFEARRAY* safearray,
                                         long *indices,
                                         const VARTYPE& vt
                                         )
{
  VARIANTARG varg;
  void *pv = NULL;

 
  HRESULT hr = S_OK;

  if(vt == VT_VARIANT)
  {
    pv = &varg;
  }
  else
  {
    VariantInit(&varg);
    varg.vt = vt;

    // e' uma union, tanto faz de quem pego o ponteiro
    pv = (void *) &varg.dblVal; 
  }

  hr = SafeArrayGetElement(safearray, indices, pv);

  if(FAILED(hr))
    LUACOM_EXCEPTION(INTERNAL_ERROR);

  com2lua(varg);

  return lua_gettop(L);
}



void tLuaCOMTypeHandler::inc_indices(long *indices, 
                        SAFEARRAYBOUND *bounds,
                        unsigned long dimensions
                        )
{
  unsigned long j = 0;

  indices[0]++;
  j = 0;

  while(
    (indices[j] >= (long) bounds[j].cElements) &&
    (j < (dimensions - 1))
    )
  {
    indices[j] = 0;
    indices[j+1]++;

    j++;
  }
}


//
// Cuida da conversao de tabelas para safe arrays
//

void tLuaCOMTypeHandler::safearray_lua2com(stkIndex luaval,
                                           VARIANTARG& varg,
                                           VARTYPE vt,
                                           bool from_stack)
{
  LUASTACK_SET(L);

  HRESULT hr = S_OK;

  tLuaVector luavector(lbeans);

  long stack_bottom = lua_gettop(L);

  if(!from_stack)
  {
    // when trying to convert a string to a safearray
    // uses a specific method
    if(lua_type(L, luaval) == LUA_TSTRING)
    {
      string2safearray(
        lua_tostring(L, luaval),
        lua_strlen(L, luaval),
        varg
        );
      return;
    }

    // creates a luavector based on the table passed
    luavector.InitVectorFromTable(luaval);
  }
  else // trying to create an array from the stack
  {
    luavector.InitVectorFromStack(luaval);    
  }

  long stack_top = lua_gettop(L);

  // Cria variaveis
  unsigned long i = 0;
  const unsigned long dimensions = luavector.get_Dimensions();
  SAFEARRAYBOUND *bounds = new SAFEARRAYBOUND[dimensions];
  SAFEARRAY *safearray = NULL;
  VARIANTARG var_value;

  VariantInit(&var_value);


  // inicializa dimensoes
  for(i = 0; i < dimensions; i++)
  {
    bounds[i].lLbound = 0;
    bounds[i].cElements = luavector.get_Nth_Dimension(dimensions - i);
  }


  // cria array
  safearray = SafeArrayCreate(vt, dimensions, bounds);
  
  long *indices = NULL;
  
  try
  {
    CHECK(safearray, INTERNAL_ERROR);

    // Inicializa indices
    indices = new long[dimensions];

    for(i = 0; i < dimensions; i++)
      indices[i] = 0;

    // copia elementos um por um
    while(indices[dimensions - 1] < (long) bounds[dimensions - 1].cElements)
    {
      // obtem valor
      luaval = luavector.getindex(indices, dimensions);

      //converte
      lua2com(luaval, var_value);

      // coloca no array
      put_in_array(safearray, var_value, indices, vt);

      // libera
      VariantClear(&var_value);

      // incrementa indices
      inc_indices(indices, bounds, dimensions);
    }
  }
  catch(class tLuaCOMException&)
  {
    delete bounds;
    delete indices;
    SafeArrayDestroy(safearray);

    while(stack_top-- > stack_bottom)
    {
      lua_remove(L, stack_bottom);
    }

    throw;
  }
  

  // preenche variantarg
  varg.vt = vt | VT_ARRAY;
  varg.parray = safearray;


  // libera memoria
  delete bounds;
  delete indices;

  while(stack_top-- > stack_bottom)
  {
    lua_remove(L, stack_bottom);
  }

  LUASTACK_CLEAN(L, 0);

  return;
}

void tLuaCOMTypeHandler::string2safearray(const char* str, long len, VARIANTARG& varg)
{
  HRESULT hr = S_OK;
  
  // cria array
  SAFEARRAY *safearray = SafeArrayCreateVector(VT_UI1, 0, len);
  CHECK(safearray, INTERNAL_ERROR);

  void * buffer = NULL;
  hr = SafeArrayAccessData(safearray,&buffer);
  if(FAILED(hr))
    LUACOM_EXCEPTION(INTERNAL_ERROR);
  if (buffer != NULL)
     memcpy(buffer,str,len);
  SafeArrayUnaccessData(safearray);

  // preenche variantarg
  varg.vt = VT_UI1 | VT_ARRAY;
  varg.parray = safearray;
}

void tLuaCOMTypeHandler::safearray2string(VARIANTARG & varg)
{
  CHECKPRECOND(varg.vt & (VT_ARRAY | VT_UI1));
  CHECKPRECOND(varg.parray->cDims == 1);

  HRESULT hr = S_OK;
  void * buffer = NULL;

  long size = varg.parray->rgsabound[0].cElements;

  hr = SafeArrayAccessData(varg.parray, &buffer);
  CHK_COM_CODE(hr);

  lua_pushlstring(L, (char*) buffer, size);

  SafeArrayUnaccessData(varg.parray);
}


long * tLuaCOMTypeHandler::dimensionsFromBounds(SAFEARRAYBOUND* bounds,
                                                long num_bounds
                                                )
{
  int i = 0;
  long *dimensions = new long[num_bounds];

  for(i = 0; i < num_bounds; i++)
  {
    dimensions[i] =
      bounds[num_bounds - i - 1].cElements; 
  }

  return dimensions;
}



void tLuaCOMTypeHandler::safearray_com2lua(VARIANTARG & varg)
{

  CHECK(varg.vt & VT_ARRAY, PARAMETER_OUT_OF_RANGE);

  bool succeeded          = false;
  long *indices           = NULL;
  SAFEARRAYBOUND* bounds  = NULL;
  
  try
  {
    SAFEARRAY* safearray = varg.parray;

    // pega dimensoes
    const int num_dimensions = SafeArrayGetDim(safearray);

    bounds = getRightOrderedBounds
      (
      safearray->rgsabound,
      num_dimensions
      );
  
    
      // cria objeto LuaVector
    tLuaVector luavector(lbeans);

    {
      long *dimensions = dimensionsFromBounds(bounds, num_dimensions);

      try
      {
        luavector.InitVectorFromDimensions(dimensions, num_dimensions);
      }
      catch(class tLuaCOMException&)
      {
        delete dimensions;
        throw;
      }

      delete dimensions;
    }

    // initializes indices
    indices = new long[num_dimensions];

    int i = 0;
    for(i = 0; i < num_dimensions; i++)
      indices[i] = bounds[i].lLbound;

    // gets array data type
    VARTYPE vt = varg.vt & ~VT_ARRAY;

    // holds index to lua objects
    stkIndex luaval = 0;

    // saves current stack position
    stkIndex stacktop = lua_gettop(L);

    // allocates enough stack room
    luaCompat_needStack(L, luavector.size()*2);

    // copia elementos um por um
    while(indices[num_dimensions-1] < 
      (long) 
       (bounds[num_dimensions-1].cElements
        + bounds[num_dimensions-1].lLbound
       )
    )
    {
      // pega do array
      luaval = get_from_array(safearray, indices, vt);

      // seta no luavector
      luavector.setindex(luaval, indices, num_dimensions, bounds);

      // incrementa indices
      inc_indices(indices, bounds, num_dimensions);
    }

    // tries to create lua table on the top of stack
    succeeded = luavector.CreateTable();

    // remove temporary objects
    stkIndex clean_until = lua_gettop(L);

    if(succeeded)
      clean_until--; // doesn't clean created table!

    while(clean_until > stacktop)
    {
      lua_remove(L, clean_until);
      clean_until--;
    }
  }
  catch(class tLuaCOMException&)
  {
    delete bounds;
    delete indices;
    throw;
  }

  delete bounds;
  delete indices;

  return;
}


tLuaCOM * tLuaCOMTypeHandler::from_lua(int index)
{
  return (tLuaCOM *) lbeans->from_lua(index);
}

TYPEDESC tLuaCOMTypeHandler::processPTR(ITypeInfo* typeinfo,
                                        const TYPEDESC &tdesc)
{
  CHECKPRECOND(tdesc.vt == VT_PTR);

  TYPEDESC pointed_at;

  // continues indirection
  pointed_at.vt = tdesc.lptdesc->vt;
  pointed_at.lptdesc = tdesc.lptdesc->lptdesc;

  // removes aliases
  pointed_at = processAliases(typeinfo, pointed_at);
  
  // if the referenced type is userdefined, gets its
  // definition
  bool userdef = false;

  if(pointed_at.vt == VT_USERDEFINED)
  {
    userdef = true;
    pointed_at = processUSERDEFINED(typeinfo, pointed_at);
  }

  if(userdef == true &&
     (pointed_at.vt == VT_DISPATCH || pointed_at.vt == VT_UNKNOWN))
  {
    // does nothing, because it's a VT_USERDEFINED TYPEDESC that
    // describes an interface that inherits from IDispatch.
    // Pointers (that is, single indirection) to IDispatch 
    // are always VT_DISPATCH.
  }
  else if(pointed_at.vt == VT_PTR)
  {
    // continues indirection
    pointed_at = processPTR(typeinfo, pointed_at);

    // We arrive here if the TYPEDESC describes a
    // pointer to a pointer. This only happens
    // when we are refencing interfaces. Since
    // interfaces are always refenced as pointers,
    // it looks like a single indirection

    pointed_at.vt |= VT_BYREF; 
  }
  else if(pointed_at.vt == VT_SAFEARRAY)
  {
    pointed_at = processSAFEARRAY(typeinfo, pointed_at);
    pointed_at.vt |= VT_BYREF; 
  }
  else // other types under a VT_PTR are just BYREF
  {
    pointed_at.vt |= VT_BYREF; 
  }

  return pointed_at;
}

TYPEDESC tLuaCOMTypeHandler::processUSERDEFINED(ITypeInfo* typeinfo,
                                                const TYPEDESC &tdesc)
{
  HRESULT hr = S_OK;
  ITypeInfo *userdef = NULL;
  TYPEATTR *typeattr = NULL;
  TYPEDESC newtdesc;

  newtdesc.vt = 0;

  hr = typeinfo->GetRefTypeInfo(tdesc.hreftype, &userdef);

  if(FAILED(hr))
    TYPECONV_ERROR("Could not understand user-defined type");

  hr = userdef->GetTypeAttr(&typeattr);

  if(FAILED(hr))
  {
    userdef->Release();
    TYPECONV_ERROR("Could not understand user-defined type");
  }
  
  switch(typeattr->typekind)
  {
  case TKIND_ENUM:
    newtdesc.vt = VT_INT;
    break;

  case TKIND_DISPATCH:
    newtdesc.vt = VT_DISPATCH;
    break;

  case TKIND_ALIAS:
    // shouldn't arrive here: aliases must be removed via
    // processAliases()
    INTERNAL_ERROR();
    break;

  case TKIND_INTERFACE:
    newtdesc.vt = VT_UNKNOWN;
    break;

  case TKIND_UNION:
    TYPECONV_ERROR("Union type not supported!");
    break;

  case TKIND_COCLASS:
    newtdesc.vt = VT_UNKNOWN;
    break;

  case TKIND_RECORD:
    TYPECONV_ERROR("Record type not supported!");
    break;

  case TKIND_MODULE:
  case TKIND_MAX:
    TYPECONV_ERROR("TKIND_MODULE and TKIND_MAX not supported!");
    break;

  default:
    TYPECONV_ERROR("Unknown TYPEKIND on VT_USERDEFINED TYPEDESC");
    break;
  }

  userdef->ReleaseTypeAttr(typeattr);
  userdef->Release();

  return newtdesc;
}

//
// Clears a VARIANT, releasing first the memory allocated

void tLuaCOMTypeHandler::releaseVariant(VARIANTARG *pvarg, bool release_memory)
{
  if(pvarg->vt & VT_BYREF && pvarg->byref != NULL)
  {
    switch(pvarg->vt & ~VT_BYREF)
    {
    case VT_BSTR:
      SysFreeString(*pvarg->pbstrVal);
      break;

    case VT_DISPATCH:
      COM_RELEASE(*pvarg->ppdispVal);
      break;

    case VT_UNKNOWN:
      COM_RELEASE(*pvarg->ppunkVal);
      break;

    case VT_VARIANT:
      // a variant cannot contain another BYREF
      // so we just clear with VariantClear
      VariantClear(pvarg->pvarVal);
      break;

    }

    if(pvarg->vt & VT_ARRAY && *pvarg->pparray)
      SafeArrayDestroy(*pvarg->pparray);

    if(release_memory)
    {
      CoTaskMemFree(pvarg->byref);
      pvarg->byref = NULL;
      pvarg->vt = VT_EMPTY;
    }
  }
  else
    VariantClear(pvarg);

}


// Dereferences typedef's in type descriptions

TYPEDESC tLuaCOMTypeHandler::processAliases(ITypeInfo* typeinfo,
                                            const TYPEDESC &tdesc)
{
  // if it's not a userdefined type, does nothing
  if(tdesc.vt != VT_USERDEFINED)
    return tdesc;

  HRESULT hr = S_OK;
  ITypeInfo *userdef = NULL;
  TYPEATTR *typeattr = NULL;
  TYPEDESC newtdesc;

  newtdesc.vt = 0;

  hr = typeinfo->GetRefTypeInfo(tdesc.hreftype, &userdef);

  if(FAILED(hr))
    TYPECONV_ERROR("Could not understand user-defined type");

  hr = userdef->GetTypeAttr(&typeattr);

  if(FAILED(hr))
  {
    userdef->Release();
    TYPECONV_ERROR("Could not understand user-defined type");
  }

  if(typeattr->typekind == TKIND_ALIAS)
  {
    newtdesc = typeattr->tdescAlias;
    newtdesc = processAliases(typeinfo, newtdesc);
  }
  else
    newtdesc = tdesc;

  userdef->ReleaseTypeAttr(typeattr);
  userdef->Release();

  return newtdesc;
}

TYPEDESC tLuaCOMTypeHandler::processTYPEDESC(ITypeInfo* typeinfo,
                                             TYPEDESC tdesc)
{
  // removes aliases
  tdesc = processAliases(typeinfo, tdesc);

  bool done = false;

  switch(tdesc.vt)
  {
  case VT_USERDEFINED:
    tdesc = processUSERDEFINED(typeinfo, tdesc);
    break;

  case VT_PTR:
    tdesc = processPTR(typeinfo, tdesc);
    break;

  case VT_SAFEARRAY:
    tdesc = processSAFEARRAY(typeinfo, tdesc);
  }

  CHECKPOSCOND(tdesc.vt != VT_USERDEFINED && tdesc.vt != VT_PTR);

  return tdesc;
}


/*
 * IsIUnknown
 *
 *   checks whether the lua value is of tag LuaCOM_IUnknown
 */

bool tLuaCOMTypeHandler::isIUnknown(stkIndex value)
{
  lua_pushvalue(L, value);

  bool result = luaCompat_isOfType(L, MODULENAME, LCOM_IUNKNOWN_TYPENAME);

  lua_pop(L, 1);

  return result;
}


void tLuaCOMTypeHandler::pushIUnknown(IUnknown *punk)
{
  luaCompat_pushTypeByName(L, MODULENAME, LCOM_IUNKNOWN_TYPENAME);
  luaCompat_newTypedObject(L, punk);
}


void tLuaCOMTypeHandler::initByRefParam(VARIANTARG* pvarg, VARTYPE vt)
{
  CHECKPRECOND(!(vt & VT_BYREF));
  VariantClear(pvarg);

  pvarg->vt = vt | VT_BYREF;

  const long size = VariantSize(vt);

  pvarg->byref = (void *) CoTaskMemAlloc(size);

  // Initializes the allocated memory
  if(vt == VT_VARIANT)
    VariantInit(pvarg->pvarVal);
  else
    memset(pvarg->byref, 0, size);
}

void tLuaCOMTypeHandler::toByRefParam(VARIANT &var_source, VARIANTARG* pvarg_dest)
{
  CHECKPARAM(pvarg_dest);
  CHECKPRECOND(!(var_source.vt & VT_BYREF));

  // if a VARIANT ByRef is expected, just copies the value
  // to the ByRef VARIANT
  if(pvarg_dest->vt == (VT_VARIANT | VT_BYREF))
  {
    // we do a hard copy, to avoid increasing the reference count
    // objects by the use of VariantCopy
    memcpy(pvarg_dest->pvarVal, &var_source, sizeof(VARIANT));
    return;
  }

  VARTYPE vt_dest = pvarg_dest->vt & ~VT_BYREF;

  // Tries to convert the value to the type expected in varg_dest
  if(!(vt_dest & VT_ARRAY))
  {
    HRESULT hr = 
      VariantChangeType(&var_source, &var_source, 0, vt_dest);

    CHK_COM_CODE(hr);
  }

  // Clears the old contents
  releaseVariant(pvarg_dest, false);

  pvarg_dest->vt = vt_dest | VT_BYREF;

  // Does a hard copy of the memory contents,

  memcpy(pvarg_dest->byref, &var_source.byref, VariantSize(vt_dest));
}

TYPEDESC tLuaCOMTypeHandler::processSAFEARRAY(ITypeInfo* typeinfo,
                                              TYPEDESC &tdesc)
{
  CHECKPRECOND(tdesc.vt == VT_SAFEARRAY);

  TYPEDESC pointed_at;

  // continues indirection
  pointed_at = *tdesc.lptdesc;
  pointed_at = processTYPEDESC(typeinfo, pointed_at);

  pointed_at.vt |= VT_ARRAY;

  return pointed_at;
}


// Returns the memory size of data that can
// be stored in a VARIANT
long tLuaCOMTypeHandler::VariantSize(VARTYPE vt)
{
  if(vt & VT_ARRAY)
  {
    return sizeof(SAFEARRAY *);
  }

  switch(vt)
  {
  case VT_I2:
    return 2;

  case VT_I4:
    return 4;
    
  case VT_R4:
    return 4;

  case VT_R8:
    return 8;

  case VT_CY:
    return sizeof(CURRENCY);

  case VT_DATE:
    return sizeof(DATE);

  case VT_BSTR:
    return sizeof(BSTR);

  case VT_DISPATCH:
    return sizeof(IDispatch*);

  case VT_ERROR:
    return sizeof(SCODE);

  case VT_BOOL:
    return sizeof(VARIANT_BOOL);

  case VT_VARIANT:
    return sizeof(VARIANT);

  case VT_UNKNOWN:
    return sizeof(IUnknown*);

  case VT_DECIMAL:
    return 16;

  case VT_UI1:
  case VT_I1:
    return 1;

  case VT_UI2:
    return 2;

  case VT_UI4:
    return 4;

  case VT_INT:
    return sizeof(int);

  case VT_UINT:
    return sizeof(unsigned int);

  default:
    TYPECONV_ERROR("Unknown type");
  }
}

//
// Type conversion function
//
void tLuaCOMTypeHandler::Coerce(VARIANTARG &dest, VARIANTARG src, VARTYPE vt)
{
  HRESULT hr = S_OK;

  if(vt == VT_VARIANT)
  {
    // do nothing. We already have a VARIANT
    return;
  }

  hr = VariantChangeType(&dest, &src, 0, vt);
  CHK_COM_CODE(hr);

}
