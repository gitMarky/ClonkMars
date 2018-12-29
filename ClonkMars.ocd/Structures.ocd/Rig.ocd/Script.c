#include Library_MarsStructure
#include Library_MarsResearch
#include Library_PowerSystem_Consumer
#include Library_PowerSystem_DisplayStatus
#include Library_DamageControl
#include Library_ConstructionAnimation
#include Library_PipeControl

/* --- Properties --- */

local Name = "$Name$";
local Description = "$Description$";
local ContainBlast = 1;
local HitPoints = 25;
local Components = { Metal=3, Silicon=1 };
local Touchable = 2;

local drillhead;
local rig_light;

// Can attach one source pipe to the oil rig
local PipeLimit_Air = 0;
local PipeLimit_Source = 1;
local PipeLimit_Drain = 0;

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

/* --- Pipe Control --- */


public func OnPipeConnect(object pipe, string specific_pipe_state)
{
	if (PIPE_STATE_Source == specific_pipe_state)
	{
		SetSourcePipe(pipe);
		pipe->SetSourcePipe();
	}
	else
	{
		if (!GetSourcePipe())
		{
			OnPipeConnect(pipe, PIPE_STATE_Source);
		}
	}
	pipe->Report("$MsgConnectedPipe$");
}

func AcceptLiquidTransfer(string liquid_name)
{
	var target = GetConnectedObject(GetSourcePipe(), true);
	if (target)
	{
		var liquid = Library_LiquidContainer->GetLiquidDef(liquid_name) ?? liquid_name;
		return target->~AcceptsLiquid(liquid, 1);
	}
	return false;
}

func DoLiquidTransfer(string liquid_name, int amount)
{
	var target = GetConnectedObject(GetSourcePipe(), true);
	if (target && target->~IsLiquidContainerForMaterial(liquid_name))
	{
		var liquid = Library_LiquidContainer->GetLiquidDef(liquid_name) ?? liquid_name;
		target->~PutLiquid(liquid, amount);
	}
}
