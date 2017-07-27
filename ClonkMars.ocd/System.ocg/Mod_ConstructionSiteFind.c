
global func Find_ID(id type)
{
	var find_old = _inherited(type, ...);
	
	if (type == ConstructionSite)
	{
		return Find_And(find_old, _inherited(ConstructionSiteEx));
	}
	else
	{
		return find_old;
	}
}
