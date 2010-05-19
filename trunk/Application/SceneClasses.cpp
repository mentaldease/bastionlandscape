#include "stdafx.h"
#include "../Application/Application.h"
#include "../Application/Scene.h"
#include "../Application/Sky.h"
#include "../Application/DebugTextOverlay.h"
#include "../Core/Scripting.h"
#include "../Core/Util.h"
#include "../Display/GeometryHelper.h"

namespace BastionGame
{
	bool Scene::RegisterClasses()
	{
		bool bResult = true;

		#define CHECK_RETURN(ReturnValue, Expression) if (false != ReturnValue) { ReturnValue = Expression; }
		CHECK_RETURN(bResult, RegisterClass(MakeKey(string("landscape")), boost::bind(&Scene::CreateClassLandscape, _1, _2)));
		CHECK_RETURN(bResult, RegisterClass(MakeKey(string("sphere")), boost::bind(&Scene::CreateClassShpere, _1, _2)));
		CHECK_RETURN(bResult, RegisterClass(MakeKey(string("sky")), boost::bind(&Scene::CreateClassSky, _1, _2)));
		CHECK_RETURN(bResult, RegisterClass(MakeKey(string("lines")), boost::bind(&Scene::CreateClassLineManager, _1, _2)));
		#undef CHECK_RETURN

		return bResult;
	}

	void Scene::UnregisterClasses()
	{
		s_mClasses.clear();
	}

	CoreObjectPtr Scene::CreateClassLandscape(LuaObjectRef _rTable, ScenePtr _pScene)
	{
		DisplayPtr pDisplay = Display::GetInstance();
		LandscapePtr pLandscape = new Landscape(*pDisplay);
		Landscape::OpenInfo oLOInfo;
		string strRenderStage;

		oLOInfo.m_strName = _rTable["name"].GetString();
		oLOInfo.m_uGridSize = _rTable["grid_size"].GetInteger();
		oLOInfo.m_uQuadSize = _rTable["grid_chunk_size"].GetInteger();
		oLOInfo.m_fPixelErrorMax = _rTable["pixel_error_max"].GetFloat();
		oLOInfo.m_fFloorScale = _rTable["floor_scale"].GetFloat();
		oLOInfo.m_fHeightScale = _rTable["height_scale"].GetFloat();
		Scripting::Lua::Get(_rTable, "target_stage", strRenderStage, strRenderStage);
		oLOInfo.m_uRenderStageKey = MakeKey(strRenderStage);

		bool bResult = (false != pLandscape->Create(boost::any(0)));

		if (false != bResult)
		{
			string strFormat = _rTable["vertex_format"].GetString();
			oLOInfo.m_eFormat = Landscape::StringToVertexFormat(strFormat);
			bResult = (ELandscapeVertexFormat_UNKNOWN != oLOInfo.m_eFormat);
		}
		if (false != bResult)
		{
			oLOInfo.m_strHeightmap.clear();
			oLOInfo.m_strHeightmap = _rTable["heightmap"].GetString();
			oLOInfo.m_strLayersConfig.clear();
			oLOInfo.m_strLayersConfig = _rTable["layers_config"].GetString();
			oLOInfo.m_pOctree = _pScene->GetOctree();
			bResult = pLandscape->Open(oLOInfo);
		}
		if (false != bResult)
		{
			string strMaterialName = _rTable["material"].GetString();
			Key uKey = MakeKey(strMaterialName);
			DisplayMaterialPtr pMaterial = pDisplay->GetMaterialManager()->GetMaterial(uKey);
			bResult = (NULL != pMaterial);
			pLandscape->SetMaterial(pMaterial);
		}
		if (false != bResult)
		{
			Vector3 oPos(0.0f, 0.0f, 0.0f);
			Scripting::Lua::Get(_rTable, "position", oPos, oPos);
			Matrix m4World;
			D3DXMatrixTranslation(&m4World, oPos.x, oPos.y, oPos.z);
			pLandscape->SetWorldMatrix(m4World);
			_pScene->AddLandscape(pLandscape);
		}
		if (false == bResult)
		{
			CoreObject::ReleaseDeleteReset(pLandscape);
		}

		return pLandscape;
	}

	CoreObjectPtr Scene::CreateClassShpere(LuaObjectRef _rTable, ScenePtr _pScene)
	{
		DisplayGeometrySpherePtr pResult = new DisplayGeometrySphere(*_pScene->GetOctree());
		const float fSize = 100.f;
		DisplayGeometrySphere::CreateInfo oGSCInfo;
		Scripting::Lua::Get(_rTable, "bottom_hemisphere", true, oGSCInfo.m_bBottomHemisphere);
		Scripting::Lua::Get(_rTable, "top_hemisphere", true, oGSCInfo.m_bTopHemisphere);
		Scripting::Lua::Get(_rTable, "view_from_inside", false, oGSCInfo.m_bViewFromInside);
		Scripting::Lua::Get(_rTable, "position", Vector3(0.0f, 0.0f, 0.0f), oGSCInfo.m_f3Pos);
		Scripting::Lua::Get(_rTable, "rotation", Vector3(0.0f, 0.0f, 0.0f), oGSCInfo.m_f3Rot);
		Scripting::Lua::Get(_rTable, "radius", Vector3(fSize, fSize, fSize), oGSCInfo.m_f3Radius);
		Scripting::Lua::Get(_rTable, "horiz_slices", UInt(10), oGSCInfo.m_uHorizSlices);
		Scripting::Lua::Get(_rTable, "vert_slices", UInt(10), oGSCInfo.m_uVertSlices);
		Scripting::Lua::Get(_rTable, "color", Vector4(26.0f / 255.0f, 103.0f / 255.0f, 149.0f / 255.0f, 1.0f), oGSCInfo.m_f4Color);

		bool bResult = pResult->Create(boost::any(&oGSCInfo));
		while (false != bResult)
		{
			DisplayPtr pDisplay = Display::GetInstance();
			string strMaterialName;
			Scripting::Lua::Get(_rTable, "material", strMaterialName, strMaterialName);
			const Key uMaterialKey = MakeKey(strMaterialName);
			DisplayMaterialPtr pMaterial = pDisplay->GetMaterialManager()->GetMaterial(uMaterialKey);
			bResult = (NULL != pMaterial);
			if (false == bResult)
			{
				break;
			}

			string strRenderStage;
			bResult = Scripting::Lua::Get(_rTable, "target_stage", strRenderStage, strRenderStage);
			if (false == bResult)
			{
				break;
			}

			pResult->SetMaterial(pMaterial);
			pResult->SetRenderStage(MakeKey(strRenderStage));

			break;
		}

		if (false == bResult)
		{
			CoreObject::ReleaseDeleteReset(pResult);
		}

		return pResult;
	}

	CoreObjectPtr Scene::CreateClassSky(LuaObjectRef _rTable, ScenePtr _pScene)
	{
		SkyPtr pResult = new Sky(*_pScene);
		bool bResult = pResult->Create(boost::any(_rTable));

		if (false == bResult)
		{
			CoreObject::ReleaseDeleteReset(pResult);
		}

		return pResult;
	}

	CoreObjectPtr Scene::CreateClassLineManager(LuaObjectRef _rTable, ScenePtr _pScene)
	{
		DisplayGeometryLineManagerPtr pResult = new DisplayGeometryLineManager;
		DisplayGeometryLineManager::CreateInfo oGLMCInfo; 
		Scripting::Lua::Get(_rTable, "position", Vector3(0.0f, 0.0f, 0.0f), oGLMCInfo.m_f3Pos);
		Scripting::Lua::Get(_rTable, "rotation", Vector3(0.0f, 0.0f, 0.0f), oGLMCInfo.m_f3Rot);
		bool bResult = pResult->Create(boost::any(&oGLMCInfo));

		while (false != bResult)
		{
			DisplayPtr pDisplay = Display::GetInstance();
			string strMaterialName;
			Scripting::Lua::Get(_rTable, "material", strMaterialName, strMaterialName);
			const Key uMaterialKey = MakeKey(strMaterialName);
			DisplayMaterialPtr pMaterial = pDisplay->GetMaterialManager()->GetMaterial(uMaterialKey);
			bResult = (NULL != pMaterial);
			if (false == bResult)
			{
				break;
			}

			string strRenderStage;
			bResult = Scripting::Lua::Get(_rTable, "target_stage", strRenderStage, strRenderStage);
			if (false == bResult)
			{
				break;
			}

			pResult->SetMaterial(pMaterial);
			pResult->SetRenderStage(MakeKey(strRenderStage));

			break;
		}

		if (false == bResult)
		{
			CoreObject::ReleaseDeleteReset(pResult);
		}

		return pResult;
	}
}
