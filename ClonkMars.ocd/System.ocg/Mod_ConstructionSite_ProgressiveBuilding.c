/*
	Progressive Building

	Allows structures to be build continuously while adding materials, instead of instantly when all materials are there.
*/

#appendto ConstructionSite

local progressive_building;


/* -- Engine callbacks -- */

public func Construction()
{
	progressive_building = {
		progress_con = 0,	// how far have we built? 0 - 1000
		progress_max = 0,	// how far can we build? 0 - 1000
		components = CreateObject(Dummy, 0, 0, NO_OWNER),
		con_site = nil,		// the actual construction site
	};

	return _inherited(...);
}


public func Destruction()
{
	if (progressive_building.components)
	{
		progressive_building.components->SetPosition(GetX(), GetY());
		progressive_building.components->RemoveObject(true);
	}
	_inherited(...);
}


public func GetInteractionMenus(object clonk)
{
	var menus = _inherited(clonk, ...) ?? [];		
	var comp_menu =
	{
		title = "Available material", // TODO
		entries_callback = this.GetAvailableMaterialMenuEntries,
		BackgroundColor = RGB(0, 50, 0),
		Priority = 14,
	};
	PushBack(menus, comp_menu);
	return menus;
}

// Transfer to internal counter, update is carried out by Ejection()
public func Collection2(object item)
{
	if (progressive_building.components)
	{
		item->Enter(progressive_building.components);
	}
	UpdateStatus(item);
}


/* -- Internals -- */

// Gets the number of available components of a type.
private func GetAvailableComponentCount(id component)
{
	return ContentsCount(component) + progressive_building.components->ContentsCount(component);
}


private func GetComponentName(id component)
{
	return Format("%i", component);
}


private func GetAvailableComponents()
{
	if (definition == nil)
		return;

	if (full_material == true)
		return nil;

	// Check for material
	var available_material = CreateArray();
	for (var entry in GetProperties(definition.Components))
	{
		var component = GetDefinition(entry);
		var amount = progressive_building.components->ContentsCount(component);
		if (component && amount)
		{
			PushBack(available_material, {id = component, count = amount});
		}
	}
	
	return available_material;
}


public func GetAvailableMaterialMenuEntries(object clonk)
{
	var material = GetAvailableComponents();
	if (!material) return [];
	
	var entries = [];
	for (var mat in material)
	{
		var text = nil;
		if (mat.count > 1) text = Format("x%d", mat.count);
		PushBack(entries, {symbol = mat.id, text = text});
	}
	return entries;
}


private func UpdateStatus(object item)
{	
	// Update message
	ShowMissingComponents();
	
	// Update possibly open menus.
	UpdateInteractionMenus([this.GetAvailableMaterialMenuEntries, this.GetMissingMaterialMenuEntries]);
	
	// Update preview image
	if (definition) definition->~SetConstructionSiteOverlay(this, direction, stick_to, item);
	
	// Check if we're done?
	if (full_material)
	{
		// Check who built it
		var controller = GetOwner();
		if (item) controller = item->GetController();
		// Remove contents for good
		if (progressive_building.components) progressive_building.components->RemoveObject();
		// Create the thing
		StartConstructing(controller);
	}
}
