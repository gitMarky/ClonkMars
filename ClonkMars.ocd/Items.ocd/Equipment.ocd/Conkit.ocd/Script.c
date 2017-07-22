#include Library_Constructor
// TODO #include L_RC // Recycling

/* -- Engine callbacks -- */

private func Hit()
{
	StonyObjectHit();
}


/* -- Constructor library -- */

public func CanBuild(id construction_plan)
{
	return construction_plan && construction_plan->~IsBuildableByConKit();
}


public func CreateConstructionSite(object clonk, id structure_id, int x, int y, bool blocked, int dir, object stick_to)
{
	var return_value = _inherited(clonk, structure_id, x, y, blocked, dir, stick_to);
	if (return_value) // remove if used!
	{
		ScheduleCall(this, this.RemoveObject, 1);
	}
	return return_value;
}


/* -- Production & research -- */

public func IsMarsResearch() { return true; }


/* -- Properties -- */

local Name = "$Name$";
local Description = "$Description$";
local Collectible = true;
local Components = {Metal = 1, Plastic = 1};
