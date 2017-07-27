#appendto ConstructionSite

// Players can put materials into the construction site
func IsInteractable(object clonk)
{
	if (clonk) return !Hostile(GetOwner(), clonk->GetOwner());
	return true;
}


// Adapt appearance in the interaction bar.
public func GetInteractionMetaInfo(object clonk)
{
	return { Description = "$TxtMissingMaterial$", IconName = nil, IconID = Hammer };
}

// Called on player interaction.
public func Interact(object clonk)
{
	if (!IsInteractable(clonk))
		return true;
	if (clonk) TakeConstructionMaterials(clonk);
	Sound("Structures::Build?");
	return true;
}

