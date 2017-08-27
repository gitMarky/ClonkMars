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
	Timer = func (int time)
	{
		if (Target->GetActionTarget())
		{
			// Custom call, instead of DoCon
			Target->GetActionTarget()->~DoConstructionProgress(1, Target);
			// Action
			Target->PlayAnimation("KneelDown", CLONK_ANIM_SLOT_Arms, Anim_Linear(0, 0, Target->GetAnimationLength("KneelDown"), this.KneelDuration, ANIM_Remove), Anim_Linear(0, 0, 1000, 30, ANIM_Remove));
			// Particle effects
			Target->WeldingFX(Target->GetCalcDir() * 6, 8);
			// Sound effects
			if (time > KneelDuration)
			{
				Target->Sound("Clonk_Build", {loop_count = 1});
			}
		}
		else
		{
			Target->StopBuilding();
		}
	},
	
	Destruction = func()
	{
		if (Target) Target->Sound("Clonk_Build", {loop_count = -1});
	},
	
	KneelDuration = 30,
};


private func WeldingFX(int x, int y)
{
	WeldingSpark(x, y,  5, RGB(0, 100, 255));
	WeldingSpark(x, y, 7, RGB(0, 80, 225));
	WeldingSpark(x, y, 10, RGB(0, 60, 200));
}


private func WeldingSpark(int x, int y, int radius, int rgb)
{
	CreateParticle("MagicSpark", PV_Random(x-2, x+2), PV_Random(y-2, y+2), PV_Random(-radius, +radius), PV_Random(-radius, 0), PV_Random(30, 45), Particles_Colored(Particles_Glimmer(), rgb), 3);
	
	if (!Random(30))
	{
		CreateParticle("Magic", x, y, 0, 0, PV_Random(10, 20), { Prototype = Particles_Colored(Particles_MuzzleFlash(), rgb), Size = PV_Random(15, 25), Alpha = PV_Linear(PV_Random(120, 60), 0)}, 1);
	}
}
