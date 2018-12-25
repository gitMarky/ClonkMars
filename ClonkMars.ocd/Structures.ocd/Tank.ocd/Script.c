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
		// The barrel can hold only one type of liquid
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
	if (OilTank_Liquid)
	{
		return liquid_name == OilTank_Liquid->GetLiquidType();
	}
	else
	{
		return IsValueInArray(["Oil", "Lava"], liquid_name); // Currently only for oil and lava, otherwise for specific liquid
	}
}

/* --- Display --- */

func UpdateOilTank()
{
	OilTank_Liquid = Contents();
}
