materiallib00 =
{
	{
		name = "water00",
		effect = "data/effects/ocean.fx",
		technique = "Main",
		params =
		{
			{
				semantic = "ENVIRONMENTTEX",
				name = "CloudyHillsCubemap2",
				value = "data/textures/CloudyHillsCubemap2.dds",
			},
			{
				semantic = "NORMALTEX",
				name = "waves2",
				value = "data/textures/waves2.dds",
			}
		},
	},
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
		name = "terrain00",
		effect = "data/effects/terrain.fx",
		technique = "RenderScene",
		params =
		{
			{
				semantic = "DIFFUSETEX",
				name = "diffuse00",
				value = "data/landscapes/land01.tga",
			},
		},
	},
	{
		name = "terrain02",
		effect = "data/effects/terrain3.fx",
		technique = "RenderScene",
	},
	{
		name = "terrain03",
		effect = "data/effects/terrain4.fx",
		technique = "RenderScene",
	},
	{
		name = "waterpost00",
		effect = "data/effects/WaterPost.fx",
		technique = "RenderScene",
		params =
		{
			{
				semantic = "TEX2D00",
				name = "waterpost00_foamMap",
				value = "data/textures/waterpost00_foamMap.jpg",
			},
			{
				semantic = "TEX2D01",
				name = "waterpost00_normalMap",
				value = "data/textures/waterpost00_normalMap.dds",
			},
		},
	},
	{
		name = "waterpost01",
		effect = "data/effects/WaterPost2.fx",
		technique = "RenderScene",
		params =
		{
			{
				semantic = "TEX2D00",
				name = "waterpost00_foamMap",
				value = "data/textures/waterpost00_foamMap.jpg",
			},
			{
				semantic = "TEX2D01",
				name = "waterpost00_normalMap",
				value = "data/textures/waterpost00_normalMap.dds",
			},
		},
	},
}
