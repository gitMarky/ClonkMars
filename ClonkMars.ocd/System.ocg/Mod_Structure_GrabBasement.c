#appendto Library_Structure


/* -- Engine callbacks -- */

public func Initialize()
{
	_inherited(...);

	// perfect positioning!
	if (GetBasement())
	{
		var y = GetBasement()->GetY() + GetBasement()->GetTop() - this->GetBottom();
		SetPosition(GetX(), y);
	}
}

/* -- Internals -- */

public func DoConstructionEffects(object construction_site)
{
	var do_fx = _inherited(construction_site, ...);

	// grab existing basement from a construction site
	var basement = construction_site->~GetBasement();
	if (basement)
	{
		construction_site->~SetBasement(nil);	// remove basement from the site
		basement->~SetParent(this);				// add basement to myself
	}

	return do_fx;
}
