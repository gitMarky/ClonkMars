/*--- Plastic ---*/

public func Hit()
{
	Sound("Hits::Materials::Wood::WoodHit?");
	return true;
}

local Collectible = 1;
local Name = "$Name$";
local Description = "$Description$";
local BlastIncinerate = 5;
local ContactIncinerate = 1;
local Plane = 470;
local Components = {Oil = 300};
