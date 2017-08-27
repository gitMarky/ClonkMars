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
		component_count = 0,
		components = CreateObject(Dummy, 0, 0, NO_OWNER),
		con_site = nil,		// the actual construction site
	};
	
	// determine max component count
	return _inherited(...);
}


public func Destruction()
{
	if (progressive_building.components)
	{
		progressive_building.components->SetPosition(GetX(), GetY());
		progressive_building.components->RemoveObject(!full_material);
	}
	var basement = GetBasement();
	if (basement)
	{
		basement->RemoveObject();
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

/* -- Interaction -- */


// Players can put materials into the construction site
func IsInteractable(object clonk)
{
	if (clonk) return !Hostile(GetOwner(), clonk->GetOwner());
	return true;
}


// Adapt appearance in the interaction bar.
public func GetInteractionMetaInfo(object clonk)
{
	var text;
	
	if (CanContinueConstructing())
	{
		text = "$TxtConstruct$";
	}
	else
	{
		text = "$TxtMissingMaterial$";
	}

	return { Description = text, IconName = nil, IconID = Hammer };
}


// Called on player interaction.
public func Interact(object clonk)
{
	if (IsInteractable(clonk))
	{
		if (clonk)
		{
			TakeConstructionMaterials(clonk);
			Sound("Structures::Build?");
		}
	}
	return true;
}

/* -- Construction logic -- */

private func StartConstructing(int by_player)
{
	if (!definition) return;

	is_constructing = true;

	// Find all objects on the bottom of the area that are not stuck
	var lying_around = GetObjectsLyingAround();

	// Create the site?
	progressive_building.con_site = CreateConstructionSite();
	ConSiteInit();
	
	// Clean up stuck objects
	EnsureObjectsLyingAround(lying_around);
}


// Override is necessary so that clonk starts constructing immediately after adding materials
private func TakeConstructionMaterials(object from_clonk)
{
	_inherited(from_clonk, ...);
	
	if (CanContinueConstructing())
	{
		ContinueConstructing(from_clonk);
	}
}


private func CanContinueConstructing()
{
	return progressive_building.progress_con < progressive_building.progress_max;
}


private func ContinueConstructing(object clonk)
{
	if (!definition) return;
	
	clonk->SetAction("Build", this);
}


private func SetConstructionSiteOverlay(id type, int dir, object stick, int w, int h)
{
	// Basic layer: invisibile
	SetClrModulation(RGBa(255, 255, 255, 0));
	// Picture preview for interaction and menu signs
	SetGraphics(nil, definition, 1, GFXOV_MODE_Picture);
	// Construction site sign on top
	SetGraphics(nil, Icon_ConstructionSite, 2, GFXOV_MODE_ExtraGraphics);
	SetObjDrawTransform(1000, 0, (-1 + dir * 2) * 500 * w + (1 - dir * 3) * 1000 * GetID()->GetDefWidth(), 0, 1000, 0, 2);
}


private func DoConstructionProgress(int change, object builder)
{
	// Change the progress
	progressive_building.progress_con = BoundBy(progressive_building.progress_con + change, 0, progressive_building.progress_max);
	UpdateCurrentProgress();
	
	// Cancel building
	if (!CanContinueConstructing())
	{
		builder->~StopBuilding();
	}
}


private func UpdateMaximumProgress()
{
	var amount = GetAvailableComponentCount();
	var max = progressive_building.component_count;

	progressive_building.progress_max = amount * 1000 / Max(1, max);
}


private func UpdateCurrentProgress()
{
	// Update construction site display
	if (progressive_building.con_site)
	{
		if (progressive_building.con_site->GetAnimationLength("construction_progress"))
		{
			if (progressive_building.con_site->GetAction() != "construction_progress")
			{
				progressive_building.con_site->SetAction("construction_progress");
			}
			progressive_building.con_site->SetPhase(progressive_building.progress_con);
		}
		else
		{
			var yoff = progressive_building.con_site->GetBottom() * (1000 - progressive_building.progress_con);
			progressive_building.con_site->SetObjDrawTransform(1000, 0, 0, 0, progressive_building.progress_con, yoff);
		}
	}
	
	// Update possibly open menus.
	UpdateInteractionMenus([this.GetAvailableMaterialMenuEntries]);
}


private func ConSiteInit()
{
	progressive_building.con_site->SetObjectLayer(progressive_building.con_site);
	progressive_building.con_site->SetCon(100);
	progressive_building.con_site->MovePosition(0, -1);
	this.Plane = progressive_building.con_site.Plane + 1;
	
	if (progressive_building.con_site->~GetBasementID())
	{
		progressive_building.con_site.lib_structure = progressive_building.con_site.lib_structure ?? {};
		progressive_building.con_site.EditorActions = progressive_building.con_site.EditorActions ?? {};
		progressive_building.con_site->AddBasement();
	}
}

private func ConSiteFinish()
{
	progressive_building.con_site->SetObjectLayer(nil);
}


/* -- Internals -- */


public func Set(id structure, int dir, object stick)
{
	_inherited(structure, dir, stick);
	
	// Update max component amount
	var component;
	for (var i = 0; component = definition->GetComponent(nil, i); ++i)
	{
		progressive_building.component_count += definition->GetComponent(component);
	}

	// Create construction site
	StartConstructing(GetOwner());
	
	// Update the appearance
	UpdateMaximumProgress();
	UpdateCurrentProgress();
}


// Gets the number of available components of a type.
private func GetAvailableComponentCount(id component)
{
	return progressive_building.components->ContentsCount(component);
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


private func GetAvailableMaterialMenuEntries(object clonk)
{
	var material = GetAvailableComponents();
	if (!material) return [];

	var entries = [];
	PushBack(entries, GetProgressMenuEntry());
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
	// Update possible progress
	UpdateMaximumProgress();

	// Update message
	ShowMissingComponents();

	// Update possibly open menus.
	UpdateInteractionMenus([this.GetAvailableMaterialMenuEntries, this.GetMissingMaterialMenuEntries]);

	// Update preview image
	if (definition) definition->~SetConstructionSiteOverlay(this, direction, stick_to, item);
}


private func GetProgressMenuEntry()
{
	var menu = 
	{
		Bottom = "1em", Priority = 1, 
		BackgroundColor = RGBa(0, 0, 0, 100),
		max =
		{
			Right = ToPercentString(progressive_building.progress_max, 10),
			BackgroundColor = RGBa(0, 255, 255, 50),
			Priority = 1,
		},
		con =
		{
			Right = ToPercentString(progressive_building.progress_con, 10),
			BackgroundColor = RGBa(0, 255, 255, 50),
			Priority = 2,
		},
	};

	return {custom = menu};
}


private func GetBasement()
{
	if (progressive_building && progressive_building.con_site)
	{
		return progressive_building.con_site->~GetBasement();
	}
}

