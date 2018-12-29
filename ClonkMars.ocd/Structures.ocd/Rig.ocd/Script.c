#include Library_MarsStructure
#include Library_MarsResearch
#include Library_PowerSystem_Consumer
#include Library_PowerSystem_DisplayStatus
#include Library_DamageControl
#include Library_ConstructionAnimation

/* --- Properties --- */

local Name = "$Name$";
local Description = "$Description$";
local ContainBlast = 1;
local HitPoints = 25;
local Components = { Metal=3, Silicon=1 };
local Touchable = 2;

local drillhead;
local rig_light;


/* --- Engine Callbacks --- */

func Initialize()
{
	if (!rig_light)
	{
		rig_light = CreateObject(Dummy, -19, -8, NO_OWNER);
		rig_light->SetObjectLayer(rig_light);
		rig_light.Visibility = VIS_All;
		rig_light->SetLightRange(40);
	}
	rig_light->SetLightColor(RGB(255, 255, 220));
	_inherited(...);
}


func Destruction()
{
	if (drillhead)
	{
		drillhead->RemoveObject();
	}
	if (rig_light)
	{
		rig_light->RemoveObject();
	}
}

/* --- Controls --- */


public func ControlDown(object clonk)
{
	SetMoveDirection(COMD_Down, clonk->GetController());
	return true;
}

public func ControlUp(object clonk)
{
	SetMoveDirection(COMD_Up, clonk->GetController());
	return true;
}

public func ControlStop(object clonk, int control)
{	
	if ((control == CON_Up && GetYDir() <= 0)
    ||  (control == CON_Down && GetYDir() >= 0))
    {
	    if (DrillHeadCheck())
		{
			drillhead->Halt(true);
		}
		UnregisterPowerRequest();
	}
	return true;
}

/* --- Internals --- */

func SetMoveDirection(int direction, int controller)
{
	if (DrillHeadCheck())
	{
		if (HasEnoughPower())
		{
			var drill = drillhead->GetContact(-1, CNAT_Bottom) && direction == COMD_Down;
			drillhead->SetMoveDirection(direction, drill, controller);
		}
		else
		{
			RegisterPowerRequest(GetNeededPower());
		}
	}
}

func DrillHeadCheck()
{
	if (!drillhead) 
	{
		drillhead = CreateObject(Structure_OilRig_DrillHead, 10, 41, GetOwner());
		drillhead->SetRig(this);
	}
	return drillhead;
}

func GetNeededPower()
{
	return 20;
}
