#ifndef __EFFECTSTATEMANAGER_H__
#define __EFFECTSTATEMANAGER_H__

#include "../Display/Display.h"

namespace ElixirEngine
{
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	// DisplayStateManager is NOT a CoreObject derived class because of conflicting Release method
	class DisplayStateManager : public ID3DXEffectStateManager
	{
	public:
		DisplayStateManager();
		virtual ~DisplayStateManager();

		HRESULT __stdcall QueryInterface(REFIID iid, LPVOID *ppv);
		ULONG __stdcall AddRef();
		ULONG __stdcall Release();

		HRESULT __stdcall SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix);
		HRESULT __stdcall SetMaterial(CONST D3DMATERIAL9 *pMaterial);
		HRESULT __stdcall SetLight(DWORD Index, CONST D3DLIGHT9 *pLight);
		HRESULT __stdcall LightEnable(DWORD Index, BOOL Enable);
		HRESULT __stdcall SetRenderState(D3DRENDERSTATETYPE State, DWORD Value);
		HRESULT __stdcall SetTexture(DWORD Stage, LPDIRECT3DBASETEXTURE9 pTexture);
		HRESULT __stdcall SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
		HRESULT __stdcall SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);
		HRESULT __stdcall SetNPatchMode(FLOAT NumSegments);
		HRESULT __stdcall SetFVF(DWORD FVF);
		HRESULT __stdcall SetVertexShader(LPDIRECT3DVERTEXSHADER9 pShader);
		HRESULT __stdcall SetVertexShaderConstantF(UINT RegisterIndex, CONST FLOAT *pConstantData, UINT RegisterCount);
		HRESULT __stdcall SetVertexShaderConstantI(UINT RegisterIndex, CONST INT *pConstantData, UINT RegisterCount);
		HRESULT __stdcall SetVertexShaderConstantB(UINT RegisterIndex, CONST BOOL *pConstantData, UINT RegisterCount);
		HRESULT __stdcall SetPixelShader(LPDIRECT3DPIXELSHADER9 pShader);
		HRESULT __stdcall SetPixelShaderConstantF(UINT RegisterIndex, CONST FLOAT *pConstantData, UINT RegisterCount);
		HRESULT __stdcall SetPixelShaderConstantI(UINT RegisterIndex, CONST INT *pConstantData, UINT RegisterCount);
		HRESULT __stdcall SetPixelShaderConstantB(UINT RegisterIndex, CONST BOOL *pConstantData, UINT RegisterCount);

		void Reset();
		void BeginPass(const UInt _uPass);
		void EndPass();
		void EndPass(UIntRef _uChangeCount, UIntRef _uDuplicateCount);

	protected:
		typedef map<D3DRENDERSTATETYPE, DWORD> RenderStateMap;
		typedef map<D3DSAMPLERSTATETYPE, DWORD> SingleSamplerStateMap;
		typedef SingleSamplerStateMap& SingleSamplerStateMapRef;
		typedef map<DWORD, SingleSamplerStateMap> MultiSamplerStateMap;
		typedef map<DWORD, LPDIRECT3DBASETEXTURE9> TextureStageMap;
		typedef map<D3DTEXTURESTAGESTATETYPE, DWORD> TextureStateMap;
		typedef TextureStateMap& TextureStateMapRef;
		typedef map<DWORD, TextureStateMap> TextureStageStateMap;

		struct States;
		typedef States* StatesPtr;
		typedef States& StatesRef;
		struct States
		{
			States(DeviceRef _rDevice);

			void Clear();

			HRESULT SetRenderState(StatesRef _rSave, D3DRENDERSTATETYPE State, DWORD Value);
			HRESULT SetTexture(StatesRef _rSave, DWORD Stage, LPDIRECT3DBASETEXTURE9 pTexture);
			HRESULT SetTextureStageState(StatesRef _rSave, DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
			HRESULT SetSamplerState(StatesRef _rSave, DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);

			void RestoreRenderState(StatesRef _rSave);
			void RestoreTexture(StatesRef _rSave);
			void RestoreTextureStageState(StatesRef _rSave);
			void RestoreSamplerState(StatesRef _rSave);

			RenderStateMap			m_mRenderStates;
			MultiSamplerStateMap	m_mSamplerStates;
			TextureStageMap			m_mTextures;
			TextureStageStateMap	m_mTextureStates;
			DeviceRef				m_rDevice;
			UInt					m_uChangeCount;
			UInt					m_uDuplicateCount;
		};

	protected:
		States		m_oGlobalStates;
		States		m_oCurrentStates;
		ULONG		ulRefCount;
		DeviceRef	m_rDevice;
	};
}

#endif // __EFFECTSTATEMANAGER_H__
