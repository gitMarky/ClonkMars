#appendto ConstructionSite


public func Set(id type, int dir, object stick)
{
	_inherited(type, dir, stick);
	
	// dimensions
	var w = type->~GetSiteWidth(direction, stick_to) ?? type->GetDefWidth();
	//var h = type->~GetSiteHeight(direction, stick_to) ?? type->GetDefHeight();
	
	// invisible first layer
	SetClrModulation(RGBa(255, 255, 255, 0));

	// add construction site sign on top
	SetGraphics(nil, Icon_ConstructionSite, 3, GFXOV_MODE_ExtraGraphics);
	SetObjDrawTransform(1000, 0, (-1 + dir * 2) * 500 * w + (1 - dir * 3) * 1000 * GetID()->GetDefWidth(), 0, 1000, 0, 3);
	
	// add basement?
	var basement_type = type->~GetBasementID();
	if (basement_type)
	{
		// dig out the surface
		var height = Clonk->GetDefHeight();
		DigFreeRect(GetX() + GetLeft(), GetY() + GetBottom() - height, GetRight() - GetLeft(), height, true);
		
		// hack the basement in there
		this.lib_structure = this.lib_structure ?? {};
		this.EditorActions = this.EditorActions ?? {};
		this.AddBasement = Library_Structure.AddBasement;
		this.GetBasementID = type.GetBasementID;
		this.GetBasementWidth = type.GetBasementWidth;
		this->AddBasement();
	}
}


private func GetBasement()
{
	if (this.lib_structure)
	{
		return this.lib_structure.basement;
	}
}


private func SetBasement(object basement)
{
	if (this.lib_structure)
	{
		this.lib_structure.basement = basement;
	}
}
