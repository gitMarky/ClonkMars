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
		finished = false,	// if done
		full_material = false, // mirrors full_material. Necessary, because GetMissingComponents() sets full_material to true at one point, and other functions return values depend on that value. This caused a bug where one material was enough for the building to think that it needs no more material
	};
	
	// determine max component count
	return _inherited(...);
}


public func Destruction()
{
	// Remove saved components
	if (progressive_building.components)
	{
		progressive_building.components->SetPosition(GetX(), GetY());
		progressive_building.components->RemoveObject(!progressive_building.full_material);
	}
	// Delete basement and con site if unfinished
	if (!progressive_building.finished)
	{
		if (progressive_building.con_site)
		{
			progressive_building.con_site->RemoveObject();
		}
		var basement = GetBasement();
		if (basement)
		{
			basement->RemoveObject();
		}
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
	
	if (clonk->GetAction() == "Build")
	{
		text = "$TxtConstructStop$";
	}
	else if (CanContinueConstructing(clonk))
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
	if (clonk && IsInteractable(clonk))
	{
		if (clonk->GetAction() == "Build")
		{
			clonk->~StopBuilding();
		}
		else
		{
			TakeConstructionMaterials(clonk);
			Sound("Structures::Build?");
		}
	}
	return true;
}

/* -- Construction logic -- */

func StartConstructing(int by_player)
{
	if (!definition) return;

	is_constructing = true;

	// Find all objects on the bottom of the area that are not stuck
	var lying_around = GetObjectsLyingAround();

	// Create the site?
	progressive_building.con_site = CreateConstructionSite();
	InitializeConstructionSite(progressive_building.con_site);
	
	// Clean up stuck objects
	EnsureObjectsLyingAround(lying_around);
}


// Override is necessary so that clonk starts constructing immediately after adding materials
func TakeConstructionMaterials(object from_clonk)
{
	_inherited(from_clonk, ...);
	
	if (CanContinueConstructing(from_clonk))
	{
		ContinueConstructing(from_clonk);
	}
}


func CanContinueConstructing(object clonk)
{
	return progressive_building.progress_con < progressive_building.progress_max;
}


func ContinueConstructing(object clonk)
{
	if (!definition) return;
	
	clonk->SetAction("Build", this);
}


func SetConstructionSiteOverlayDefault(id type, int dir, object stick, int w, int h)
{
	// Basic layer: invisibile
	SetClrModulation(RGBa(255, 255, 255, 0));
	// Picture preview for interaction and menu signs
	SetGraphics(nil, definition, 1, GFXOV_MODE_Picture);
	// Construction site sign on top
	SetGraphics(nil, Icon_ConstructionSite, 2, GFXOV_MODE_ExtraGraphics);
	SetObjDrawTransform(1000, 0, (-1 + dir * 2) * 500 * w + (1 - dir * 3) * 1000 * GetID()->GetDefWidth(), 0, 1000, 0, 2);
}


func DoConstructionProgress(int change, object builder)
{
	// Change the progress
	progressive_building.progress_con = BoundBy(progressive_building.progress_con + change, 0, progressive_building.progress_max);
	UpdateConstructionProgress();
	
	// Cancel building
	if (!CanContinueConstructing(builder))
	{
		builder->~StopBuilding();
	}
	
	// Finish site?
	if (progressive_building.progress_con >= 1000)
	{
		FinishConstructing(progressive_building.con_site);
	}
}


func UpdateMaximumProgress()
{
	var amount = GetAvailableComponentCount();
	var max = progressive_building.component_count;

	progressive_building.progress_max = BoundBy(amount * 1000 / Max(1, max), 0, 1000);
}


func UpdateConstructionProgress()
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


func InitializeConstructionSite(object construction)
{
	construction->SetObjectLayer(progressive_building.con_site);
	construction->SetCon(100);
	construction->MovePosition(0, -1);
	this.Plane = construction.Plane + 1;
	
	if (construction->~GetBasementID())
	{
		construction.lib_structure = construction.lib_structure ?? {};
		construction.EditorActions = construction.EditorActions ?? {};
		construction->AddBasement();
		construction->GetBasement().Plane = this.Plane + 1;
	}

	construction->~OnStartConstructing();
}

func FinishConstructing(object construction)
{
	construction->SetObjectLayer(nil);
	construction->~OnFinishConstructing();
	progressive_building.finished = true;
	RemoveObject();
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
	UpdateConstructionProgress();
}


// Gets the number of available components of a type.
func GetAvailableComponentCount(id component)
{
	if (progressive_building.full_material)
	{
		return progressive_building.component_count;
	}
	else
	{
		return progressive_building.components->ContentsCount(component);
	}
}


func GetComponentName(id component)
{
	return Format("%i", component);
}


func GetAvailableComponents()
{
	var available_material = [];
	if (definition == nil)
		return available_material;

	// Check for material
	for (var entry in GetProperties(definition.Components))
	{
		var component = GetDefinition(entry);
		var amount;
		if (progressive_building.full_material)
		{
			amount = definition.Components[entry];
		}
		else
		{
			amount = progressive_building.components->ContentsCount(component);
		}
		if (component && amount)
		{
			PushBack(available_material, {id = component, count = amount});
		}
	}
	
	return available_material;
}


func GetAvailableMaterialMenuEntries(object clonk)
{
	var entries = [];
	PushBack(entries, GetProgressMenuEntry());

	var material = GetAvailableComponents();
	if (material)
	{
		for (var mat in material)
		{
			var text = nil;
			if (mat.count > 1) text = Format("x%d", mat.count);
			PushBack(entries, {symbol = mat.id, text = text});
		}
	}

	return entries;
}


func UpdateStatus(object item)
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


func GetProgressMenuEntry()
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


func GetBasement()
{
	if (progressive_building && progressive_building.con_site)
	{
		return progressive_building.con_site->~GetBasement();
	}
}


func UpdateFullMaterial()
{
	progressive_building.full_material = full_material;
}


func GetMissingComponents()
{
	var return_value = _inherited(...);
	UpdateFullMaterial();
	return return_value;
}


public func ForceConstruct()
{
	_inherited(...);
	UpdateFullMaterial();
}

