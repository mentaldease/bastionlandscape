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
	hierarchy =
	{
		{
			class = "landscape",
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
			target_pass = "scene",
		},
		{
			class = "sphere",
			name = "sky",
			material = "sky",
			size = { 2000.0, 2000.0, 2000.0 },
			bottom_hemisphere = true,
			top_hemisphere = true,
			view_from_inside = true,
			position = { 0.0, 0.0, 0.0, },
			rotation = { 0.0, 0.0, 0.0, },
			horiz_slices = 10,
			vert_slices = 100,
			color = { 26.0 / 255.0, 103.0 / 255.0, 149.0 / 255.0, 1.0 },
			target_pass = "scene",
		},
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
			camera = "uicameraortho00",
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
