#include Library_MarsStructure
#include Library_MarsResearch
#include Library_ConstructionAnimation
#include Library_PowerSystem_Producer

/* -- Properties -- */

local Name = "$Name$";
local Description = "$Description$";
local ContainBlast = 1;
local Components = { Metal=1, Plastic=1 };


/* -- Engine callbacks -- */

private func Construction()
{
	_inherited(...);
	
	if (GetOwner() == NO_OWNER)
	{
		SetColor(RGB(150, 111, 139));
	}
}


private func InitializeStructure()
{
	_inherited(...);
	GetPowerSystem()->RegisterPowerProducer(this);
	AddTimer(this.SolarPanelProducePower, 35);
}

/* -- Power production -- */

private func BaseEnergy()
{
/* TODO
	if (upgraded)
		return 25;
	else
*/
		return 15;
}

private func SolarPanelProducePower()
{
	var rot_h = -20;
	var rot_v = +45;
	if (Time->IsNight())
	{
		SetPowerProduction(0);
	}
	else
	{
		// Calculate power production
		var night_brightness = 15;
		var current_brightness = Max(0, GetAmbientBrightness() - night_brightness);
		var max_brightness = Max(1, 100 - night_brightness);
		var energy = current_brightness * BaseEnergy() / max_brightness;
		SetPowerProduction(energy);
		
		// Adjust rotation
		var time = FindObject(Find_ID(Time));
		if (time)
		{
			var day_length = time.time_set.sunset_end - time.time_set.sunrise_start;
			var day_progress = BoundBy(time.time - time.time_set.sunrise_start, 0, day_length);
			
			var phase = day_progress * 180 / day_length;
			rot_h = -90 + phase;		// range from -80 to 80
			rot_v = +90 - Sin(phase, 50);	// range from +90 to +40
		}
	}
	SetDishRotation(rot_h, rot_v);
}


private func SetDishRotation(int horizontal, int vertical, int time)
{
	horizontal = BoundBy(Normalize(horizontal, -180), -90, 90);
	vertical = BoundBy(55 - vertical, 0, 90);
	
	var transform;
	if (horizontal == 0 && vertical == 0)
	{
		transform = Trans_Identity();
	}
	else
	{
		transform = Trans_Mul(Trans_Rotate(horizontal, 0, 1, 0), Trans_Rotate(vertical, 1, 0, 0));
	}
	TransformBone("sat", transform, 1, Anim_Linear(0, 0, 1000, time ?? 35, ANIM_Remove));
}

