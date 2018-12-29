#include Library_MarsStructure
#include Library_MarsResearch
#include Library_PowerSystem_Consumer
#include Library_PowerSystem_DisplayStatus
#include Library_ConstructionAnimation

/* --- Properties --- */

local Name = "$Name$";
local Description = "$Description$";
local ContainBlast = 1;
local HitPoints = 25;
local Components = { Metal=3, Silicon=1 };
local Touchable = 2;

local drillhead;


/* --- Engine Callbacks --- */

func Initialize()
{
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
			drillhead->SetMoveDirection(direction, drill);
			SetPlrView(controller, drillhead);
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
		drillhead = CreateObject(Structure_OilRig_DrillHead, 13, 36, GetOwner());
		drillhead->SetRig(this);
	}
	return drillhead;
}

func GetNeededPower()
{
	return 20;
}
