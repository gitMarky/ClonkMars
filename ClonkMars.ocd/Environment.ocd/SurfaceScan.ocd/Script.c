/**
	SurfaceScan

	Adds a grid of info points to the map that reveals FoW at tunnels and liquids.
	
	Should have been enabled/disabled by landscape scan after building a base, but
	for now it is enabled always, because the visibility seems to have no effect on
	the light display.
	That seems to come into play only with hostility.
 */
 
/* --- Engine Callbacks --- */

public func InitializePlayer(int player, int x, int y, object base, int team, id extra_data)
{
	RevealFoW(player, false);
}

public func RemovePlayer(int player, int team)
{
	RevealFoW(player, false);
}

/* --- Public Interface --- */

public func CreateGrid(int sample_distance)
{
	if (this != SurfaceScan)
	{
		FatalError(Format("Called from context of %v, but can be called only from %v", this, SurfaceScan));
	}

	var control = GetLandscapeControl(true);
	if (control)
	{
		control->CreateGrid(sample_distance);
	}
	return control;
}

public func RevealFoW(int player, bool reveal)
{
	// Invert the visibility, because the actually visibly objects have the VIS_ToggleLayer mode
	this.Visibility[player + 1] = !reveal;
}

/* --- Internals --- */

func GetLandscapeControl(bool create_if_necessary)
{
	var control = GetEffect(FxLandscapeControl.Name);
	if (!control && create_if_necessary)
	{
		control = Global->CreateEffect(FxLandscapeControl, 1);
	}
	return control;
}

local FxLandscapeControl = new Effect
{
	Name = "FxLandscapeControl",

	Construction = func ()
	{
		// Set values
		this.grid_distance = 10;
		this.grid = [];
		// Create dummy object, for visibility control
		this.control_visibility = CreateObject(SurfaceScan, 0, 0, NO_OWNER);
		this.control_visibility.Visibility = [VIS_Select];
		this.control_visibility->SetObjectLayer(this.control_visibility);
	},

	Update = func ()
	{
		var width = GetLength(this.grid);
		for (var x = 0; x < width; ++x)
		{
			var height = GetLength(this.grid[x]);
			for (var y = 0; y < height; ++y)
			{
				CheckPoint(x, y);
			}
		}
	},

	CreateGrid = func(int sample_distance)
	{
		this.grid_distance = Max(1, sample_distance ?? 10);
		var amount_x = 2 + LandscapeWidth() / this.grid_distance;
		var amount_y = 2 + LandscapeHeight() / this.grid_distance;

		for (var x = 0; x < amount_x; ++x)
		{
			this.grid[x] = [];
			for (var y = 0; y < amount_y; ++y)
			{
				CheckPoint(x, y);
			}
		}
	},
	
	CheckPoint = func (int grid_x, int grid_y)
	{
		var global_x = (grid_x - 1) * this.grid_distance;
		var global_y = (grid_y - 1) * this.grid_distance;

		var material = GetMaterial(global_x, global_y);
		var reveal = (material != -1 && GetMaterialVal("Density", "Material", material) < 50);

		var point = this.grid[grid_x][grid_y];
		if (reveal)
		{
			if (!point)
			{
				point = CreateObject(Dummy, 0, 0, NO_OWNER);
				this.grid[grid_x][grid_y] = point;

				point->SetPosition(global_x, global_y);
				point->SetObjectLayer(this.control_visibility);
				point->SetLightColor(RGB(40, 40, 40));
				point->SetLightRange(this.grid_distance, this.grid_distance);
				point.Visibility = VIS_LayerToggle;
			}
		}
		else if (point)
		{
			point->RemoveObject();
		}
	},
};

