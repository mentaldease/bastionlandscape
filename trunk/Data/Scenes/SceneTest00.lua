scenetest00 =
{
	name = "SceneTest00",
	materiallibs =
	{
		"data/materials/MaterialLib00.lua",
	},
	landscapes =
	{
		--[[
		{
			object_type = "landscape",
			name = "water",
			vertex_format = "liquid",
			material = "water00",
			grid_size = 16,			// must be power of 2
			grid_chunk_size = 16,	// must be power of 2
			position_x = { 0.0, 75.0, 0.0, },
			pixel_error_max = 2.5,
			floor_scale = 10.0,
			height_scale = 1.0,
		},
		]]
		{
			object_type = "landscape",
			name = "ground",
			vertex_format = "default",
			material = "terrain03",
			grid_size = 16,
			grid_chunk_size = 16,
			position = { 0.0, 0.0, 0.0, },
			pixel_error_max = 2.5,
			floor_scale = 10.0,
			height_scale = 1.0,
			heightmap = "data/landscapes/land01.tga",
			layers_config = "data/landscapes/layers00.lua",
		}
		--[[
		{
			object_type = "landscape",
			name = "ground2",
			vertex_format = "default",
			material = "terrain00",
			grid_size = 32,
			grid_chunk_size = 8,
			position = { 0.0, -5.0, 0.0, },
			pixel_error_max = 2.5,
			floor_scale = 10.0,
			height_scale = 1.0,
		}
		]]
	},
	normalprocesses =
	{
		{
			name = "reflection",
			--viewport = "quarter_top_left",
			viewport = "default",
			render_targets =
			{
				{
					type = "gbuffer",
					index = 3,
				},
				--{
					--type = "tex2d",
					--name = reflection,
				--},
			},
		},
		--{
			--name = "reflection2",
			--viewport = "quarter_top_right",
			--render_targets =
			--{
				--{
					--type = "gbuffer",
					--index = 3,
				--},
			--},
		--},
		{
			name = "base",
			--viewport = "quarter_top_left",
			viewport = "default",
			render_targets =
			{
				{
					type = "gbuffer",
					index = 0,
				},
				{
					type = "gbuffer",
					index = 1,
				},
				{
					type = "gbuffer",
					index = 2,
				},
			},
		},
	},
	postprocesses =
	{
		{
			name = "water",
			material = "waterpost01",
			immediate_write = false,
		},
		--[[
		{
			name = "monochrome",
			material = "monochromepost00",
			immediate_write = false,
		},
		{
			name = "inverse",
			material = "inversepost00",
			immediate_write = false,
		},
		{
			name = "basicblur",
			material = "basicblurpost00",
			immediate_write = false,
		},
		{
			name = "dofcombine",
			material = "dofcombinepost00",
			immediate_write = false,
		},
		]]
	},
	water =
	{
		{
			WaterLevel = 100.0,
			FadeSpeed = 0.15,
			NormalScale = 1.0,
			R0 = 0.5,
			MaxAmplitude = 1.0,
			SunColor = { 1.0, 1.0, 1.0, },
			ShoreHardness = 1.0,
			RefractionStrength = 0.0,
			NormalModifier = {1.0, 2.0, 4.0, 8.0, },
			Displace = 1.7,
			FoamExistence = { 0.65, 1.35, 0.5, },
			SunScale = 3.0,
			Shininess = 0.7,
			SpecularIntensity = 0.32,
			DepthColour = { 0.0078, 0.5176, 0.7, },
			BigDepthColour = { 0.0039, 0.00196, 0.145, },
			Extinction = { 7.0, 30.0, 40.0, },
			Visibility = 4.0,
			Scale = 0.005,
			RefractionScale = 0.005,
			Wind = { -0.3, 0.7, },
			Forward = { 0.0, 0.0, 0.0, },
		},
	},
}
