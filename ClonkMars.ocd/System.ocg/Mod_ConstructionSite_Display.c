#appendto ConstructionSite


public func Set(id type, int dir, object stick)
{
	_inherited(type, dir, stick);
	
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


public func Destruction()
{
	var basement = GetBasement();
	if (basement)
	{
		basement->RemoveObject();
	}
	_inherited(...);
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
