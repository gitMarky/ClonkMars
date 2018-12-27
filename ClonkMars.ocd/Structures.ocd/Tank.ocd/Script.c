#include Library_MarsStructure
#include Library_MarsResearch
#include Library_ConstructionAnimation
#include Library_Tank
#include Library_ResourceSelection


/* --- Properties --- */

local Name = "$Name$";
local Description = "$Description$";
local ContainBlast = 1;
local HitPoints = 20;
local Components = { Metal=3 };


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
	}
	if (GetLength(GetResourceSelection()) == 0)
	{
		// No restrictions, accept any liquid that is accepted in the selection
		return ShowResourceSelectionMenuEntry(GetLiquidDef(liquid_name));
	}
	else
	{
		// Restrictions from the selection
		// If the contained liquid is prohibited, then you may not add new liquid of that type
		return IsInResourceSelection(GetLiquidDef(liquid_name));
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

public func GetInteractionMenus(object clonk)
{
	var menus = _inherited(clonk, ...) ?? [];	
	var materials_menu =
	{
		title = "$TankMaterials$",
		entries_callback = this.GetResourceSelectionMenuEntries,
		callback = "OnTankMaterialsClick",
		callback_hover = "OnTankMaterialsHover",
		callback_target = this,
		BackgroundColor = RGB(0, 50, 50),
		Priority = 25,
	};
	PushBack(menus, materials_menu);
	var control_menu =
	{
		title = "$TankControl$",
		entries_callback = this.GetValveControlMenuEntries,
		callback = "OnValveControlClick",
		callback_hover = "OnValveControlHover",
		callback_target = this,
		BackgroundColor = RGB(0, 50, 50),
		Priority = 30,
	};
	PushBack(menus, control_menu);
	return menus;
}


func GetValveControlMenuEntries(object clonk)
{
	// switch on and off
	var symbol, text, action;
	if (GetEffect(FxDisperseLiquid.Name, this))
	{
		text = "$MsgCloseTank$";
		symbol = Icon_Enter;
		action = PUMP_Menu_Action_Switch_Off;
	}
	else
	{
		text = "$MsgOpenTank$";
		symbol = Icon_Exit;
		action = PUMP_Menu_Action_Switch_On;
	}

	return [{symbol = symbol, extra_data = action,
		custom =
		{
			Right = "100%", Bottom = "2em",
			BackgroundColor = {Std = 0, OnHover = 0x50ff0000},
			Priority = 1,
			text = {Left = "2em", Text = text},
			image = {Right = "2em", Symbol = symbol}
		}}];
}


func OnTankMaterialsHover(id resource, string action, desc_menu_target, menu_id)
{
	var selection_description, allowed;
	var contained_resource = Contents();
	if (GetLength(GetResourceSelection()) == 0)
	{
		selection_description = "$MsgAllMaterialsAllowed$";
		allowed = true;
	}
	else
	{
		var text;
		for (var selected in GetResourceSelection())
		{
			if (text)
			{
				text = Format("%s, %s", text, selected->GetName());
			}
			else
			{
				text = selected->GetName();
			}
		}
		selection_description = Format("$MsgMaterialsRestricted$", text);
		allowed = IsInResourceSelection(resource);
	}
	if (FindContents(resource) || contained_resource)
	{
		var is_contained_resource = contained_resource->GetID() == resource;
		if (is_contained_resource && !allowed)
		{
			selection_description = Format("%s %s", selection_description, Format("$MsgRemoveMaterialOnly$", resource->GetName()));
		}
		if (allowed && !is_contained_resource)
		{
			selection_description = Format("%s %s", selection_description, Format("$MsgInsertMaterialBlocked$", contained_resource->GetID()->GetName(), resource->GetName()));
		}
	}

	var action_description;
	if (action == RESOURCE_SELECT_Menu_Action_Resource_Enable)
	{
		action_description = Format("$MsgRestrictMaterial$", resource->GetName());
	}
	else
	{
		action_description = Format("$MsgAllowAllMaterials$");
	}

	GuiUpdateText(Format("%s||%s", selection_description, action_description), menu_id, 1, desc_menu_target);
}

func OnTankMaterialsClick(id resource, string action, object clonk)
{
	if (action == RESOURCE_SELECT_Menu_Action_Resource_Enable)
	{
		SetResourceSelection([resource]);
	}
	else
	{
		SetResourceSelection([]);
	}
	UpdateInteractionMenus(this.GetResourceSelectionMenuEntries);
}

func OnValveControlClick()
{
	var fx = GetEffect(FxDisperseLiquid.Name, this);
	if (fx)
	{
		fx->Remove();
	}
	else
	{
		CreateEffect(FxDisperseLiquid, 100, 2);
	}
	UpdateInteractionMenus(this.GetValveControlMenuEntries);
}

func OnValveControlHover()
{
}

/* --- Display --- */

func IsResourceSelectionParent(id child_resource, id parent_resource)
{
	return child_resource->~GetParentLiquidType() == parent_resource;
}

func ShowResourceSelectionMenuEntry(id resource)
{
	return resource == Oil
	    || resource == Lava;
}

func GetResourceSelectionIcon(id resource, bool enabled)
{
	var has_item = FindContents(resource);
	var has_other_item = !!Contents() && !has_item;
	if (GetLength(GetResourceSelection()) == 0) // No restrictions?
	{
		enabled = true;
	}

	var icon = {Symbol = Icon_Ok};
	if (enabled)
	{
		if (has_other_item) // Show that other items block adding new material
		{
			icon.GraphicsName = "Red";
		}
	}
	else if (has_item) // Show that contained resources can be extracted
	{
		icon.GraphicsName = "Red";
	}
	else // Show that the item is not allowed
	{
		icon.Symbol = Icon_Cancel;
	}
	return icon;
}

func UpdateOilTank()
{
}
