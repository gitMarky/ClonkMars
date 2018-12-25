#include Library_MarsStructure
#include Library_MarsResearch
#include Library_ConstructionAnimation
#include Library_Tank


/* --- Properties --- */

local Name = "$Name$";
local Description = "$Description$";
local ContainBlast = 1;
local HitPoints = 20;
local Components = { Metal=3 };

local OilTank_Liquid = nil;
local OilTank_Type = nil;

local DispersionRate = 40;
local DispersionRadius = 40;


/* --- Engine Callbacks --- */

func Definition(type)
{
	type.PictureTransformation = Trans_Translate(0, 0, -20000);
	return _inherited(type, ...);
}


func RejectCollect(id type, object new_contents)
{
	if (RejectStack(new_contents)) return true;
	return _inherited(type, new_contents, ...);
}


func Collection2(object item)
{
	UpdateOilTank();
	return _inherited(item, ...);
}

func Ejection(object item)
{

	UpdateOilTank();
	return _inherited(item, ...);
}

func ContentsDestruction(object item)
{
	ScheduleCall(this, this.UpdateOilTank, 1);
	return _inherited(item, ...);
}

func RemoveLiquid(liquid_name, int amount, object destination)
{
	var res = _inherited(liquid_name, amount, destination, ...);
	UpdateOilTank();
	return res;
}

func PutLiquid(liquid_name, int amount, object source)
{
	var res = _inherited(liquid_name, amount, source, ...);
	UpdateOilTank();
	return res;
}

/* --- Callbacks from Stackable library --*/

func CollectFromStack(object item)
{
	// Callback from stackable object: Try grabbing partial objects from this stack, if the stack is too large
	if (item->GetStackCount() > GetLiquidAmountRemaining() && !this->RejectStack(item))
	{
		// Get one sample object and try to insert it into the barrel
		var candidate = item->TakeObject();
		candidate->Enter(this);
		
		// Put it back if it was not collected
		if (candidate && !(candidate->Contained()))
		{
			item->TryAddToStack(candidate);
		}
	}
}

func RejectStack(object item)
{
	// Callback from stackable object: When should a stack entrance be rejected, if the object was not merged into the existing stacks?
	if (Contents())
	{
		// Can hold only one type of liquid
		return true;
	}
	if (item->~IsLiquid() && this->~IsLiquidContainerForMaterial(item->~GetLiquidType()))
	{
		// The liquid is suitable, collect it!
		return false;
	}
	else
	{
		// Reject anything else
		return true;
	}
}

/* --- Liquid container properties --- */

public func GetLiquidContainerMaxFillLevel(liquid_name)
{
	if (liquid_name == Lava)
	{
		return 8750;
	}
	else if (liquid_name == Oil)
	{
		return 3500;
	}
	else
	{
		return 7000; // Possibly for water and acid in the future
	}
}


public func IsLiquidContainerForMaterial(string liquid_name)
{
	// Only accept a single liquid at the same time
	var liquids = GetLiquidContents();
	if (liquids)
	{
		for (var liquid_content in liquids)
		{
			if (GetLiquidDef(liquid_name) != GetLiquidDef(liquid_content))
				return false;
		}
		return true;
	}
	else
	{
		return IsValueInArray(["Oil", "Lava", "DuroLava"], liquid_name); // Currently only for oil and lava, otherwise for specific liquid
	}
}



/* --- Liquid Control --- */

// The liquid tank may have one drain and one source.
public func QueryConnectPipe(object pipe, bool show_message)
{
	if (GetDrainPipe() && GetSourcePipe())
	{
		if (show_message) pipe->Report("$MsgHasPipes$");
		return true;
	}
	else if (GetSourcePipe() && pipe->IsSourcePipe())
	{
		if (show_message) pipe->Report("$MsgSourcePipeProhibited$");
		return true;
	}
	else if (GetDrainPipe() && pipe->IsDrainPipe())
	{
		if (show_message) pipe->Report("$MsgDrainPipeProhibited$");
		return true;
	}
	else if (pipe->IsAirPipe())
	{
		if (show_message) pipe->Report("$MsgPipeProhibited$");
		return true;
	}
	return false;
}

// Set to source or drain pipe.
public func OnPipeConnect(object pipe, string specific_pipe_state)
{
	if (PIPE_STATE_Source == specific_pipe_state)
	{
		SetSourcePipe(pipe);
		pipe->SetSourcePipe();
	}
	else if (PIPE_STATE_Drain == specific_pipe_state)
	{
		SetDrainPipe(pipe);
		pipe->SetDrainPipe();
	}
	else
	{
		if (!GetDrainPipe())
		{
			OnPipeConnect(pipe, PIPE_STATE_Drain);
		}
		else if (!GetSourcePipe())
		{
			OnPipeConnect(pipe, PIPE_STATE_Source);
		}
	}
	pipe->Report("$MsgConnectedPipe$");
}


/*-- Interaction --*/

public func IsInteractable(object clonk)
{
	if (GetCon() < 100)
	{
		return false;
	}
	return !Hostile(GetOwner(), clonk->GetOwner());
}

public func GetInteractionMetaInfo(object clonk)
{
	if (GetEffect(FxDisperseLiquid.Name, this))
	{
		return { Description = "$MsgCloseTank$", IconName = nil, IconID = Icon_Enter, Selected = false };
	}
	else
	{
		return { Description = "$MsgOpenTank$", IconName = nil, IconID = Icon_Exit, Selected = false };
	}
}

public func Interact(object clonk)
{
	var fx = GetEffect(FxDisperseLiquid.Name, this);
	if (fx)
	{
		fx->Remove();
		return true;
	}
	else
	{
		CreateEffect(FxDisperseLiquid, 100, 2);	
		return true;
	}
}

local FxDisperseLiquid = new Effect
{
	Name = "FxDisperseLiquid",
	
	Construction = func()
	{
		this.Interval = 2;
		return FX_OK;
	},

	Timer = func()
	{
		var liquid = Target->Contents();
		if (!liquid || !liquid->~IsLiquid())
		{
			return FX_OK;
		}
		if (liquid->GetLiquidAmount() <= Target.DispersionRate)
		{
			liquid->Exit();
		}
		else
		{
			liquid->RemoveLiquid(nil, Target.DispersionRate);
			liquid = liquid->GetID()->CreateLiquid(Target.DispersionRate);
		}
		liquid->SetPosition(Target->GetX(), Target->GetY());
		liquid->Disperse(180, Target.DispersionRadius);
		// TODO: Sound.
		return FX_OK;
	}
};


/* --- Contents Control --- */

public func IsContainer() { return true; }

/* --- Display --- */

func UpdateOilTank()
{
	OilTank_Liquid = Contents();
}
