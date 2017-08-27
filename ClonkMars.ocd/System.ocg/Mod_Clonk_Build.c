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
	if (GetAction() == "Build")
	{
		SetAction("Walk");
	}
}


local FxBuilding = new Effect{
	Timer = func ()
	{
		if (Target->GetActionTarget())
		{
			// Custom call, instead of DoCon
			Target->GetActionTarget()->~DoConstructionProgress(1, Target);
			// Action
			Target->PlayAnimation("KneelDown", CLONK_ANIM_SLOT_Arms, Anim_Linear(0, 0, Target->GetAnimationLength("KneelDown"), this.KneelDuration, ANIM_Remove), Anim_Linear(0, 0, 1000, 30, ANIM_Remove));
			// Particle effects
			// TODO
			// Sound effects
			// TODO
		}
		else
		{
			Target->StopBuilding();
		}
	},
	
	KneelDuration = 30,
};
