scenetest00 =
{
	name = "SceneTest00",
	render_targets =
	{
		--[[
		gbuffers =
		{
			count = 4,
			formats =
			{
				"D3DFMT_A8R8G8B8",
				"D3DFMT_R32F",
				"D3DFMT_A8R8G8B8",
				"D3DFMT_A8R8G8B8",
			},
		},
		]]
		additionals =
		{
			{
				name = "rt_reflection1",
				format = "D3DFMT_A8R8G8B8",
			},
			{
				name = "rt_reflection2",
				format = "D3DFMT_A8R8G8B8",
			},
			{
				name = "rt_reflection3",
				format = "D3DFMT_A8R8G8B8",
			},
			{
				name = "rt_reflection4",
				format = "D3DFMT_A8R8G8B8",
			},
			{
				name = "ui",
				format = "D3DFMT_A8R8G8B8",
			},
		},
	},
	materials =
	{
		"data/materials/MaterialLib00.lua",
	},
	cameras =
	{
		"data/scenes/CameraLib00.lua",
	},
	water_config = "data/scenes/WaterSceneTest00.lua",
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
			material = "terrain04",
			grid_size = 16,
			grid_chunk_size = 16,
			position = { 0.0, 0.0, 0.0, },
			pixel_error_max = 2.5,
			floor_scale = 10.0,
			height_scale = 1.0,
			heightmap = "data/landscapes/land02.tga",
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
	renderpasses =
	{
		{
			name = "scene",
			camera = "scenecamera00",
			normalprocesses =
			{
				{
					name = "reflection",
					viewport = "default",
					clear = true,
					render_targets =
					{
						{
							type = "tex2d",
							name = "rt_reflection1",
							index = 0,
						},
					},
				},
				{
					name = "reflection2",
					viewport = "default",
					clear = true,
					render_targets =
					{
						{
							type = "tex2d",
							name = "rt_reflection2",
							index = 0,
						},
					},
				},
				{
					name = "base",
					viewport = "default",
					clear = true,
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
					material = "waterpost02",
					immediate_write = true,
				},
				-- {
					-- name = "monochrome",
					-- material = "monochromepost00",
				-- },
				-- {
					-- name = "inverse",
					-- material = "inversepost00",
				-- },
				-- {
					-- name = "basicblur",
					-- material = "basicblurpost00",
				-- },
				-- {
					-- name = "dofcombine",
					-- material = "dofcombinepost00",
				-- },
			},
		},
		{
			name = "ui",
			camera = "uicamera00",
			normalprocesses =
			{
				{
					name = "ui",
					viewport = "default",
					clear = false,
				},
			},
		},
	},
}
