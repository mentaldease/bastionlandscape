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
			viewport = "quarter_top_left",
			render_targets =
			{
				{
					type = "gbuffer",
					index = 3,
				},
				--[[
				{
					type = "tex2d",
					name = reflection,
				},
				]]
			},
		},
		{
			name = "base",
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
			material = "waterpost00",
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
}
