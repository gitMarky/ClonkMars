/**
	Temperature

	Adds a grid of temperature points to the map that tell the tempeature at
	a specific position.
 */


/* -- Properties -- */

local MinTemperature = -27315;	// global minimum temperature, in 1e-2 degrees Celsius
local MaxTemperature = 100000;	// global maximum temperature, in 1e-2 degrees Celsius

local PlanetMinTemperature = -8000;	// local minimum temperature, in 1e-2 degrees Celsius;
local PlanetMaxTemperature = +8000;	// local maximum temperature, in 1e-2 degrees Celsius;

local DistanceSunlight = 100;	// ground heats up to this many pixels below the surface
local DistanceGround = 100;		// ground heats / cools in multiples of these many pixels

local PlanetTempChangeSolid = nil;		// planet intrinsical warming, in 1e-2 degrees Celsius; will be warmed by sunlight only if set to 0
local PlanetTempChangeLiquid = nil;		// planet intrinsical warming, in 1e-2 degrees Celsius; will be warmed by sunlight only if set to 0
local PlanetTempChangeSky = nil;		// planet intrinsical warming, in 1e-2 degrees Celsius; will be warmed by sunlight only if set to 0

local LowerBorderDistance = 100;		// temperature changes this many pixels from the lower landscape border
local LowerBorderTempChange = -150;		// temperature at the lower border changes by this amount, in 1e-2 degrees Celsius

local UpperBorderDistance = 1;			// temperature changes this many pixels from the lower landscape border
local UpperBorderTempChange = 0;		// temperature at the upper border changes by this amount, in 1e-2 degrees Celsius

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

func GetTemperatureControl(bool create_if_necessary)
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
		// set values
		this.grid_distance = 10;
		this.grid = [];
		this.debug = false;
		this.temperature = GetTemperature();

		// do some ticks beforehand, for a better result
		for (var i = 0; i < 100; ++i)
		{
			Timer();
		}
	},

	Timer = func ()
	{
		// difference
		this.temperature = GetTemperature();
	
		// iterate over all points
		for (var column in this.grid)
		for (var point in column)
		{
			// calculate the change
			var change = 0;
			change = Temperature->CalcTempChange_Planet(point, change);
			change = Temperature->CalcTempChange_Season(point, change, this.temperature);
			change = Temperature->CalcTempChange_Sun(point, change);
			change = Temperature->CalcTempChange_LowerBorder(point, change);
			change = Temperature->CalcTempChange_UpperBorder(point, change);
			point->ChangeTemp(change);
		}
	
	
		var prec = 100;
		// influence neighbors, update graphics
		var width = GetLength(this.grid);
		for (var x = 0; x < width; ++x)
		{
			var height = GetLength(this.grid[x]);
			for (var y = 0; y < height; ++y)
			{
				var point = this.grid[x][y];
				var temperature = CalcAverageTemperature(x, y, prec);
				point->ChangeTemp(temperature - point->GetTemp(prec));
			
				// display the temperature in debug mode
				var temp = BoundBy(point->GetTemp(), -60, 128);
				if (this.debug)
				{
					var hue = 128 - temp;
					CreateParticle("Magic", point.X, point.Y, 0, 0, this.Interval, { Prototype = Particles_Colored(Particles_Trajectory(), HSL2RGB(RGB(hue, 255, 128))), Size = this.grid_distance * 2, Alpha = 50});
				}
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
	},

	CalcAverageTemperature = func(int index_x, int index_y, int prec)
	{
		var center = this.grid[index_x][index_y];
		var samples = [center, center]; // center has double weight
	
		// left side
		if (index_x > 0)
		{
			PushBack(samples, this.grid[index_x-1][index_y]);
		}
		else
		{
			PushBack(samples, center);
		}
	
		// right side
		if (index_x == GetLength(this.grid) - 1)
		{
			PushBack(samples, center);
		}
		else
		{
			PushBack(samples, this.grid[index_x + 1][index_y]);
		}
	
		// top side
		if (index_y > 0)
		{
			PushBack(samples, this.grid[index_x][index_y-1]);
		}
		else
		{
			PushBack(samples, center);
		}
	
		// bottom side
		if (index_y == GetLength(this.grid[0]) - 1)
		{
			PushBack(samples, center);
		}
		else
		{
			PushBack(samples, this.grid[index_x][index_y+1]);
		}
	
		// calculation
		var average = 0;
		for (var sample in samples)
		{
			average += sample->GetTemp(prec);
		}
		average /= GetLength(samples);
		return average;
	},
};

/* -- Calculations -- */

func CalcTempChange_Planet(proplist point, int change)
{
	if (GBackSolid(point.X, point.Y))
	{
		if (PlanetTempChangeSolid) return CalcTempChange(PlanetTempChangeSolid);
	}
	else if (GBackLiquid(point.X, point.Y))
	{
		if (PlanetTempChangeLiquid) return CalcTempChange(PlanetTempChangeLiquid);
	}
	else if (GBackSky(point.X, point.Y))
	{
		if (PlanetTempChangeSky) return CalcTempChange(PlanetTempChangeSky);
	}

	return change;
}


func CalcTempChange_Sun(proplist point, int change)
{
	if (GBackSolid(point.X, point.Y))
	{
		var distance_sky = 0;
		var sunlight = 0;
		for (; distance_sky < DistanceSunlight; ++distance_sky)
		{
			if (GBackSky(point.X, point.Y - distance_sky))
			{
				var relative = DistanceSunlight - distance_sky;
				var intensity = GetAmbientBrightness() * 7 / 4;
				sunlight = relative * intensity / DistanceSunlight;
				break;
			}
		}
	
		return BoundBy(change + sunlight, PlanetMinTemperature, PlanetMaxTemperature);
	}

	return change;
}


func CalcTempChange_Season(proplist point, int change, int temperature)
{
	if (GBackSky(point.X, point.Y))
	{
		// sky temperature should be approximately the atmospheric temperature
		// this could also be solved by light intensity stuff, but it is not done
		// like that yet
		var prec = 100;
		var diff = temperature * prec - point->GetTemp(prec);
		return BoundBy(change + diff, PlanetMinTemperature, PlanetMaxTemperature);
	}

	return change;
}


func CalcTempChange_LowerBorder(proplist point, int change)
{
	if (LowerBorderDistance > 0)
	{
		var relative =  BoundBy(point.Y - LandscapeHeight() + LowerBorderDistance, 0, LowerBorderDistance);
		var diff = relative * LowerBorderTempChange / LowerBorderDistance;
		return BoundBy(change + diff, PlanetMinTemperature, PlanetMaxTemperature);
	}

	return change;
}


func CalcTempChange_UpperBorder(proplist point, int change)
{
	if (UpperBorderDistance > 0)
	{
		var relative =  BoundBy(UpperBorderDistance - point.Y, 0, UpperBorderDistance);
		var diff = relative * CalcTempChange(UpperBorderTempChange) / UpperBorderDistance;
		return BoundBy(change + diff, PlanetMinTemperature, PlanetMaxTemperature);
	}

	return change;
}


func CalcTempChange(value)
{
	if (GetType(value) == C4V_Int)
	{
		return value;
	}
	else if (GetType(value) == C4V_Function)
	{
		return Call(value);
	}
	else
	{
		FatalError(Format("Can calculate a function or integer, got %v", GetType(value)));
	}
}


func UpperBorderTempChangeByLight()
{
	return -110 + GetAmbientBrightness() / 3;
}

