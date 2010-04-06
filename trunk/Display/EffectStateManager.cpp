#include "stdafx.h"
#include "../Display/Effect.h"
#include "../Display/EffectStateManager.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	void DisplayStateManager::States::Clear()
	{
		m_mRenderStates.clear();
		m_mSamplerStates.clear();
		m_mTextures.clear();
		m_mTextureStates.clear();
	}

	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	DisplayStateManager::DisplayStateManager()
	:	ulRefCount(0),
		m_rDevice(*Display::GetInstance()->GetDevicePtr()),
		m_uStateChanges(0),
		m_uRedundantStateChanges(0)
	{

	}

	DisplayStateManager::~DisplayStateManager()
	{

	}

	HRESULT DisplayStateManager::QueryInterface(REFIID iid, LPVOID *ppv)
	{
		if (IID_ID3DXEffectStateManager == iid)
		{
			*ppv = this;
			AddRef();
			return S_OK;
		}
		return E_NOTIMPL;
	}

	ULONG DisplayStateManager::AddRef()
	{
		return ++ulRefCount;
	}

	ULONG DisplayStateManager::Release()
	{
		return --ulRefCount;
	}

	HRESULT DisplayStateManager::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix)
	{
		HRESULT hResult = m_rDevice.SetTransform(State, pMatrix);
		return hResult;
	}

	HRESULT DisplayStateManager::SetMaterial(CONST D3DMATERIAL9 *pMaterial)
	{
		HRESULT hResult = m_rDevice.SetMaterial(pMaterial);
		return hResult;
	}

	HRESULT DisplayStateManager::SetLight(DWORD Index, CONST D3DLIGHT9 *pLight)
	{
		HRESULT hResult = m_rDevice.SetLight(Index, pLight);
		return hResult;
	}

	HRESULT DisplayStateManager::LightEnable(DWORD Index, BOOL Enable)
	{
		HRESULT hResult = m_rDevice.LightEnable(Index, Enable);
		return hResult;
	}

	HRESULT DisplayStateManager::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
	{
#if EFFECT_RENDER_FLAGS
		HRESULT hResult = S_OK;
		RenderStateMap::iterator iPair = m_oGlobalStates.m_mRenderStates.find(State);
		if ((m_oGlobalStates.m_mRenderStates.end() == iPair) || (Value != iPair->second))
		{
			DWORD CurrentValue;
			hResult = m_rDevice.GetRenderState(State, &CurrentValue);
			if (SUCCEEDED(hResult))
			{
				m_oCurrentStates.m_mRenderStates[State] = CurrentValue;
				hResult = m_rDevice.SetRenderState(State, Value);
				m_oGlobalStates.m_mRenderStates[State] = Value;
			}
		}
		else
		{
			++m_uRedundantStateChanges;
		}
#else // EFFECT_RENDER_FLAGS
		HRESULT hResult = m_rDevice.SetRenderState(State, Value);
#endif // EFFECT_RENDER_FLAGS
		++m_uStateChanges;
		return hResult;
	}

	HRESULT DisplayStateManager::SetTexture(DWORD Stage, LPDIRECT3DBASETEXTURE9 pTexture)
	{
#if EFFECT_RENDER_FLAGS
		HRESULT hResult = S_OK;
		TextureStageMap::iterator iPair = m_oGlobalStates.m_mTextures.find(Stage);
		if ((m_oGlobalStates.m_mTextures.end() == iPair) || (pTexture != iPair->second))
		{
			LPDIRECT3DBASETEXTURE9 pCurrentTexture;
			hResult = m_rDevice.GetTexture(Stage, &pCurrentTexture);
			if (SUCCEEDED(hResult))
			{
				m_oCurrentStates.m_mTextures[Stage] = pCurrentTexture;
				HRESULT hResult = m_rDevice.SetTexture(Stage, pTexture);
				m_oGlobalStates.m_mTextures[Stage] = pTexture;
			}
		}
		else
		{
			++m_uRedundantStateChanges;
		}
#else // EFFECT_RENDER_FLAGS
		HRESULT hResult = m_rDevice.SetTexture(Stage, pTexture);
#endif // EFFECT_RENDER_FLAGS
		return hResult;
	}

	HRESULT DisplayStateManager::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
	{
#if EFFECT_RENDER_FLAGS
		HRESULT hResult = S_OK;
		TextureStateMapRef rStates = m_oGlobalStates.m_mTextureStates[Stage];
		TextureStateMap::iterator iPair = rStates.find(Type);
		if ((rStates.end() == iPair) || (Value != iPair->second))
		{
			DWORD CurrentValue;
			hResult = m_rDevice.GetTextureStageState(Stage, Type, &CurrentValue);
			if (SUCCEEDED(hResult))
			{
				m_oCurrentStates.m_mTextureStates[Stage][Type] = CurrentValue;
				hResult = m_rDevice.SetTextureStageState(Stage, Type, Value);
				rStates[Type] = Value;
			}
		}
#else // EFFECT_RENDER_FLAGS
		HRESULT hResult = m_rDevice.SetTextureStageState(Stage, Type, Value);
#endif // EFFECT_RENDER_FLAGS
		return hResult;
	}

	HRESULT DisplayStateManager::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
	{
#if EFFECT_RENDER_FLAGS
		HRESULT hResult = S_OK;
		SingleSamplerStateMapRef rStates = m_oGlobalStates.m_mSamplerStates[Sampler];
		SingleSamplerStateMap::iterator iPair = rStates.find(Type);
		if ((rStates.end() == iPair) || (Value != iPair->second))
		{
			DWORD CurrentValue;
			hResult = m_rDevice.GetSamplerState(Sampler, Type, &CurrentValue);
			if (SUCCEEDED(hResult))
			{
				m_oCurrentStates.m_mSamplerStates[Sampler][Type] = CurrentValue;
				hResult = m_rDevice.SetSamplerState(Sampler, Type, Value);
				rStates[Type] = Value;
			}
		}
#else // EFFECT_RENDER_FLAGS
		HRESULT hResult = m_rDevice.SetSamplerState(Sampler, Type, Value);
#endif // EFFECT_RENDER_FLAGS
		return hResult;
	}

	HRESULT DisplayStateManager::SetNPatchMode(FLOAT NumSegments)
	{
		HRESULT hResult = m_rDevice.SetNPatchMode(NumSegments);
		return hResult;
	}

	HRESULT DisplayStateManager::SetFVF(DWORD FVF)
	{
		HRESULT hResult = m_rDevice.SetFVF(FVF);
		return hResult;
	}

	HRESULT DisplayStateManager::SetVertexShader(LPDIRECT3DVERTEXSHADER9 pShader)
	{
		HRESULT hResult = m_rDevice.SetVertexShader(pShader);
		return hResult;
	}

	HRESULT DisplayStateManager::SetVertexShaderConstantF(UINT RegisterIndex, CONST FLOAT *pConstantData, UINT RegisterCount)
	{
		HRESULT hResult = m_rDevice.SetVertexShaderConstantF(RegisterIndex, pConstantData, RegisterCount);
		return hResult;
	}

	HRESULT DisplayStateManager::SetVertexShaderConstantI(UINT RegisterIndex, CONST INT *pConstantData, UINT RegisterCount)
	{
		HRESULT hResult = m_rDevice.SetVertexShaderConstantI(RegisterIndex, pConstantData, RegisterCount);
		return hResult;
	}

	HRESULT DisplayStateManager::SetVertexShaderConstantB(UINT RegisterIndex, CONST BOOL *pConstantData, UINT RegisterCount)
	{
		HRESULT hResult = m_rDevice.SetVertexShaderConstantB(RegisterIndex, pConstantData, RegisterCount);
		return hResult;
	}

	HRESULT DisplayStateManager::SetPixelShader(LPDIRECT3DPIXELSHADER9 pShader)
	{
		HRESULT hResult = m_rDevice.SetPixelShader(pShader);
		return hResult;
	}

	HRESULT DisplayStateManager::SetPixelShaderConstantF(UINT RegisterIndex, CONST FLOAT *pConstantData, UINT RegisterCount)
	{
		HRESULT hResult = m_rDevice.SetPixelShaderConstantF(RegisterIndex, pConstantData, RegisterCount);
		return hResult;
	}

	HRESULT DisplayStateManager::SetPixelShaderConstantI(UINT RegisterIndex, CONST INT *pConstantData, UINT RegisterCount)
	{
		HRESULT hResult = m_rDevice.SetPixelShaderConstantI(RegisterIndex, pConstantData, RegisterCount);
		return hResult;
	}

	HRESULT DisplayStateManager::SetPixelShaderConstantB(UINT RegisterIndex, CONST BOOL *pConstantData, UINT RegisterCount)
	{
		HRESULT hResult = m_rDevice.SetPixelShaderConstantB(RegisterIndex, pConstantData, RegisterCount);
		return hResult;
	}

	void DisplayStateManager::Reset()
	{
		m_oGlobalStates.Clear();
		m_oCurrentStates.Clear();
		m_uStateChanges = 0;
		m_uRedundantStateChanges = 0;
	}

	void DisplayStateManager::BeginPass(const UInt _uPass)
	{
#if EFFECT_RENDER_FLAGS
		m_oCurrentStates.Clear();
#endif // EFFECT_RENDER_FLAGS
	}

	void DisplayStateManager::EndPass()
	{
#if EFFECT_RENDER_FLAGS
		{
			RenderStateMap::iterator iPair = m_oCurrentStates.m_mRenderStates.begin();
			RenderStateMap::iterator iEnd = m_oCurrentStates.m_mRenderStates.end();
			while (iEnd != iPair)
			{
				m_oGlobalStates.m_mRenderStates[iPair->first] = iPair->second;
				m_rDevice.SetRenderState(iPair->first, iPair->second);
				++iPair;
			}
		}
		{
			MultiSamplerStateMap::iterator iPair = m_oCurrentStates.m_mSamplerStates.begin();
			MultiSamplerStateMap::iterator iEnd = m_oCurrentStates.m_mSamplerStates.end();
			while (iEnd != iPair)
			{
				DWORD Sampler = iPair->first;
				SingleSamplerStateMapRef rStates = iPair->second;
				SingleSamplerStateMap::iterator iPair2 = rStates.begin();
				SingleSamplerStateMap::iterator iEnd2 = rStates.end();
				while (iEnd2 != iPair2)
				{
					m_oGlobalStates.m_mSamplerStates[Sampler][iPair2->first] = iPair2->second;
					m_rDevice.SetSamplerState(Sampler, iPair2->first, iPair2->second);
					++iPair2;
				}
				++iPair;
			}
		}
		{
			TextureStageMap::iterator iPair = m_oCurrentStates.m_mTextures.begin();
			TextureStageMap::iterator iEnd = m_oCurrentStates.m_mTextures.end();
			while (iEnd != iPair)
			{
				m_oGlobalStates.m_mTextures[iPair->first] = iPair->second;
				m_rDevice.SetTexture(iPair->first, iPair->second);
				++iPair;
			}
		}
		{
			TextureStageStateMap::iterator iPair = m_oCurrentStates.m_mTextureStates.begin();
			TextureStageStateMap::iterator iEnd = m_oCurrentStates.m_mTextureStates.end();
			while (iEnd != iPair)
			{
				DWORD Stage = iPair->first;
				TextureStateMapRef rStates = iPair->second;
				TextureStateMap::iterator iPair2 = rStates.begin();
				TextureStateMap::iterator iEnd2 = rStates.end();
				while (iEnd2 != iPair2)
				{
					m_oGlobalStates.m_mTextureStates[Stage][iPair2->first] = iPair2->second;
					m_rDevice.SetTextureStageState(Stage, iPair2->first, iPair2->second);
					++iPair2;
				}
				++iPair;
			}
		}
#endif // EFFECT_RENDER_FLAGS
	}
}
