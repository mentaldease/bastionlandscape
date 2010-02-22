bastion_config =
{
	graphics =
	{
		fullscreen = false,
		-- width = 640,
		-- height = 480,
		width = 1280,
		height = 720,
		color_format = "D3DFMT_A8R8G8B8",
		depth_format = "D3DFMT_D16",
		depth_near = 1.0,
		depth_far = 10000.0,
		gbuffer_count = 4,
		gbuffers_format =
		{
			"D3DFMT_A8R8G8B8",
			"D3DFMT_R32F",
			"D3DFMT_A8R8G8B8",
			"D3DFMT_A8R8G8B8"
		},
		viewports =
		{
			{
				name = "default",
				x = 0.0,
				y = 0.0,
				width = 1.0,
				height = 1.0
			},
			{
				name = "quarter_top_left",
				x = 0.0,
				y = 0.0,
				width = 0.5,
				height = 0.5,
			},
			{
				name = "quarter_top_right",
				x = 0.5,
				y = 0.0,
				width = 0.5,
				height = 0.5
			},
			{
				name = "quarter_bottom_left",
				x = 0.0,
				y = 0.5,
				width = 0.5,
				height = 0.5
			},
			{
				name = "quarter_bottom_right",
				x = 0.5,
				y = 0.5,
				width = 0.5,
				height = 0.5,
			}
		}
	}
}
