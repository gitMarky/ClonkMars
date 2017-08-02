/**
	Temperature
	
	Adds a grid of temperature points to the map that tell the tempeature at
	a specific position.
 */


/* -- Properties -- */

local MinTemperature = -27315;
local MaxTemperature = 100000;

/* -- Globals -- */

global func GetTemperatureAt(int x, int y, int prec)
{
	if (GetType(this) == C4V_C4Object)
	{
		x += GetX();
		y += GetY();
	}
	
	var control = Temperature->GetTemperatureControl();
	AssertNotNil(control);
	
	return control->Point(x, y)->GetTemp(prec);
}

global func SetTemperatureAt(int x, int y, int amount)
{
	if (GetType(this) == C4V_C4Object)
	{
		x += GetX();
		y += GetY();
	}
	
	var control = Temperature->GetTemperatureControl();
	AssertNotNil(control);
	
	control->Point(x, y)->SetTemp(amount);
}

/* -- Public interface -- */

public func CreateGrid(int sample_distance)
{
	if (this != Temperature)
	{
		FatalError(Format("Called from context of %v, but can be called only from %v", this, Temperature));
	}

	var control = GetTemperatureControl(true);
	if (control)
	{
		control->CreateGrid(sample_distance);
	}
	return control;
}


public func GetMaterialTemperature(int material)
{
	return GetTemperature();
}


public func GetMaterialChangeSpeed(int material)
{
	var density = GetMaterialVal("Density", "Material", material);
	if (density > 0)
	{
		// liquid?
		if (density < 30)
		{
			return 200;
		}
		// solid?
		else
		{
			return 500;
		}
	}
	// sky
	return 900;
}


/* -- Internals -- */

private func GetTemperatureControl(bool create_if_necessary)
{
	var control = GetEffect("FxTemperatureControl");
	if (!control && create_if_necessary)
	{
		control = Global->CreateEffect(FxTemperatureControl, 100, 20);
	}
	return control;
}


static const TemperaturePoint = new Global
{
	Temp = 0,	// base temperature, in precision 100 (= 0.01 degrees)
	Speed = 1000,		// speed adjustment
	X = 0,				// position, in global coordinates, precision 1
	Y = 0,				// position, in global coordinates, precision 1
	
	SetPosition = func (int x, int y)
	{
		this.X = x;
		this.Y = y;
		return this;
	},

	SetTemp = func (int amount)
	{
		if (amount)
		{
			this.Temp = BoundBy(amount, Temperature.MinTemperature, Temperature.MaxTemperature);
		}
		else
		{			
			var material = GetMat(this.X, this.Y);
			var temperature = Temperature->GetMaterialTemperature(material);
			if (temperature) SetTemp(temperature);
		}
		return this;
	},

	GetTemp = func(int prec)
	{
		return this.Temp * (prec ?? 1) / 100;
	},

	ChangeTemp = func (int amount)
	{
		SetTemp(this.Temp + amount * this.Speed  / 1000);
	},
	
	SetChangeSpeed = func (int amount)
	{
		if (amount)
		{
			this.Speed = BoundBy(amount, 0, 1000);
		}
		else
		{
			var material = GetMat(this.X, this.Y);
			var speed = Temperature->GetMaterialChangeSpeed(material);
			if (speed) SetChangeSpeed(speed);
		}
		return this;
	},
	
	GetMat = func (int x, int y)
	{
		if (x < 0)
		{
			x *= -1;
		}
		if (x > LandscapeWidth())
		{
			x = 2 * LandscapeWidth() - x;
		}
		if (y < 0)
		{
			y *= -1;
		}
		if (y > LandscapeHeight())
		{
			y = 2 * LandscapeHeight() - y;
		}
		return GetMaterial(x, y);
	},
};


local FxTemperatureControl = new Effect
{
	Construction = func ()
	{
		this.grid_distance = 10;
		this.grid = [];
		this.debug = true;
	},
	
	Timer = func ()
	{
		Log("Update temperature");
		for (var column in this.grid)
		for (var point in column)
		{
			var temp = BoundBy(point->GetTemp(), -60, 128);
			if (this.debug)
			{
				var hue = 128 - temp;
				CreateParticle("Magic", point.X, point.Y, 0, 0, this.Interval, { Prototype = Particles_Colored(Particles_Trajectory(), HSL2RGB(RGB(hue, 255, 128))), Size = this.grid_distance * 2, Alpha = 50});
			}
		}
	},
	
	CreateGrid = func(int sample_distance)
	{
		Log("Creating temperature grid");
		this.grid_distance = Max(1, sample_distance ?? 10);
		var amount_x = 2 + LandscapeWidth() / this.grid_distance;
		var amount_y = 2 + LandscapeHeight() / this.grid_distance;

		for (var x = 0; x < amount_x; ++x)
		{
			this.grid[x] = [];
			for (var y = 0; y < amount_y; ++y)
			{
				this.grid[x][y] = new TemperaturePoint {};
				
				var global_x = (x - 1) * this.grid_distance;
				var global_y = (y - 1) * this.grid_distance;

				this.grid[x][y]->SetPosition(global_x, global_y)
				               ->SetTemp()
				               ->SetChangeSpeed();
			}
		}
	},
	
	Point = func (int x, int y)
	{
		x = BoundBy(x, -this.grid_distance, LandscapeWidth() + this.grid_distance);
		y = BoundBy(y, -this.grid_distance, LandscapeHeight() + this.grid_distance);
		var index_x = 1 + x / this.grid_distance;
		var index_y = 1 + y / this.grid_distance;
		
		return this.grid[index_x][index_y];
	}
};
