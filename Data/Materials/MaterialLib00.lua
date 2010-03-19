materiallib00 =
{
	{
		name = "basicblurpost00",
		effect = "data/effects/BasicBlurPost.fx",
		technique = "RenderScene",
	},
	{
		name = "dofcombinepost00",
		effect = "data/effects/DofCombinePost.fx",
		technique = "RenderScene",
	},
	{
		name = "inversepost00",
		effect = "data/effects/InversePost.fx",
		technique = "RenderScene",
	},
	{
		name = "monochromepost00",
		effect = "data/effects/MonochromePost.fx",
		technique = "RenderScene",
	},
	{
		name = "simplepost00",
		effect = "data/effects/SimplePost.fx",
		technique = "RenderScene",
	},
	{
		name = "terrain04",
		effect = "data/effects/terrain5.fx",
		technique = "RenderScene",
	},
	{
		name = "terrain05",
		effect = "data/effects/terrainScattering.fx",
		technique = "RenderScene",
	},
	{
		name = "waterpost02",
		effect = "data/effects/WaterPost3.fx",
		technique = "RenderScene",
		params =
		{
			{
				semantic = "TEX2D00",
				name = "waterpost00_atlasMap",
				value = "data/textures/waterpost00_atlasMap.bmp",
			},
			{
				semantic = "TEX2D01",
				name = "rt_reflection1",
				value = "no_file",
			},
			{
				semantic = "TEX2D02",
				name = "rt_reflection2",
				value = "no_file",
			},
			{
				semantic = "TEX2D03",
				name = "rt_reflection3",
				value = "no_file",
			},
			{
				semantic = "TEX2D04",
				name = "rt_reflection4",
				value = "no_file",
			},
		},
	},
	{
		name = "ui",
		effect = "data/effects/UI.fx",
		technique = "RenderScene",
	},
	{
		name = "geomhelper",
		effect = "data/effects/Geometryhelper.fx",
		technique = "RenderScene",
	},
	{
		name = "geomhelper_line",
		effect = "data/effects/Geometryhelper.fx",
		technique = "RenderSceneVertexColor",
	},
	{
		name = "sky",
		effect = "data/effects/Sky.fx",
		technique = "SkyDomeScatteringPerPixel",
	},
}
