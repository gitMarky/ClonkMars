#appendto Clonk


// Extend existing ActMap and Build action
local ActMap = {
	Build = {
		StartCall = "StartBuilding",
		AbortCall = "StopBuilding",
	},
};


public func StartBuilding()
{
	if(!GetEffect("FxBuilding", this))
	{
		CreateEffect(FxBuilding, 1, 2);
	}
}


public func StopBuilding()
{
	RemoveEffect("FxBuilding", this);
}


local FxBuilding = new Effect{
	Timer = func ()
	{
		if (Target->GetActionTarget())
		{
			// Custom call, instead of DoCon
			Target->GetActionTarget()->~DoConstructionProgress(1, Target);
		}
		else
		{
			Target->StopBuilding();
		}
	},
};
