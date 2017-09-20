#include Library_MarsStructure
#include Library_MarsResearch
#include Library_ConstructionAnimation
#include Library_PowerSystem_Producer
#include Library_PowerSystem_DisplayStatus

/* -- Properties -- */

local Name = "$Name$";
local Description = "$Description$";
local ContainBlast = 1;
local Components = { Metal=1, Plastic=1 };
local panel_angle = 0;	// made this a property, because it is way easier to debug the current angle this way


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
	SetDishRotation(0, 55);
	AddTimer(this.SolarPanelProducePower, 35);
}

/* -- Power production -- */

private func IsSteadyPowerProducer()
{
	return true;
}


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
	var rot_v = +65;
	if (Time->IsNight())
	{
		SetPowerProduction(0);
	}
	else
	{
		// Adjust rotation
		var time = FindObject(Find_ID(Time));
		if (time)
		{
			var day_start = (time.time_set.sunrise_start + time.time_set.sunrise_end) / 2;
			var day_end = (time.time_set.sunset_start + time.time_set.sunset_end) / 2;

			var day_length = day_end - day_start;
			var day_progress = BoundBy(time.time - day_start, 0, day_length);
			
			var phase = day_progress * 180 / day_length;
			rot_h = -80 + phase * 8 / 9;	// range from -80 to 80
			rot_v = +80 - Sin(phase, 40);	// range from +80 to +40
			//Log("Solar panel: day_length %d, day_progress %d, phase %d, rot h %d, rot v %d", day_length, day_progress, phase, rot_h, rot_v);
		}
		
		// Calculate light intensite
		// - First, determine the angle that points to the sky from the current position
		var dx = +Sin(rot_h, 1000);
		var dy = -Cos(rot_v, 1000);
		panel_angle = Angle(0, 0, dx, dy);
		// - Second, we sum the (simplified) 3 parts if the angle is free at the main angle +/- 15 degrees
		var angular_parts = GetAngleFree(panel_angle - 15, +1) + GetAngleFree(panel_angle, +1) + GetAngleFree(panel_angle + 15, +1);

		// Calculate power production
		var night_brightness = 15;
		var current_brightness = Max(0, GetAmbientBrightness() - night_brightness);
		var max_brightness = Max(1, 100 - night_brightness);
		var energy = current_brightness * BaseEnergy() * angular_parts / (3 * max_brightness);
		SetPowerProduction(energy);
	}
	SetDishRotation(rot_h, rot_v);
}


private func SetDishRotation(int horizontal, int vertical, int time)
{
	horizontal = BoundBy(Normalize(horizontal, -180), -90, 90);
	vertical = BoundBy(55 - vertical, -35, 55);
	
	var transform;
	if (horizontal == 0 && vertical == 0)
	{
		transform = Trans_Mul(Trans_Rotate(0, 0, 1, 0), Trans_Rotate(1, 1, 0, 0));
	}
	else
	{
		transform = Trans_Mul(Trans_Rotate(horizontal, 0, 1, 0), Trans_Rotate(vertical, 1, 0, 0));
	}
	TransformBone("sat", transform, 1, Anim_Linear(0, 0, 1000, time ?? 35, ANIM_Remove));
}

