/**
	Damage control
	
	Handles the way buildings and vehicles take damage.
	
	@credits Luchs, Nachtfalter
 */
 
/* -- Callbacks -- */


public func Damage (int change, int cause, int cause_plr)
{
	/* TODO
	if (GetDamage() > MaxHitPoints())
	{
		AddEffect("MaxDamageExplosion", this, 1, 20, this, DACT);
	}
	else
	{
		var type = "BuildingAttack";
		if(GetID() == CPSL)
			type = "CapsuleAttack";
		UpdateHUD(-GetOwner() - 2, HUD_EventLog, type);
	}
	*/

	CreateFragment("Glass", PV_Random(-50, +50), PV_Random(-50, 50), Particles_Glass(), change + Random(4));
	// TODO: Fragment particles do not exist CreateFragment("Fragment1", PV_Random(-50, +50), PV_Random(-50, 50), nil, GetDamage() * 5 / MaxHitPoints());
	
	// what you usually do
	return _inherited(change, cause, cause_plr);
}



// Callback from the structure: Object was destroyed
func OnNoHitPointsRemaining(int cause, int cause_plr)
{
	if (!GetEffect("FxMaxDamageExplosion", this))
	{
		CreateEffect(FxMaxDamageExplosion, 1, 20, cause_plr);
	}
	return true; // damage is handled, do not remove this object
}


/* -- Internals -- */


func DestroyBlast(int by_player)
{
	AssertObjectContext();
		
	/* TODO
	var type = "BuildingExplode";
	if(target -> GetID() == CPSL)
		type = "CapsuleExplode";
	UpdateHUD(-(target -> GetOwner()) - 2, HUD_EventLog, type);
	*/
	
	var components_metal = this->GetComponent(Metal);
	var components_plastic = this->GetComponent(Plastic);
	
	var shape = GetShapeRectangle();
	var count =  components_metal + Random(components_plastic);
	var power = Distance(shape.wdt, shape.hgt);

	this->~EjectContentsOnDestruction(FX_Call_DmgBlast, by_player);
	this->CastObjects(Ore, count / 2, power);
	this->Explode(power / 2);

	return true;
}


local FxMaxDamageExplosion = new Effect
{
	Construction = func (int cause_player)
	{
		Target->Sound("Warning_blowup", {loop_count = 1});
		this.caused_by_player = cause_player;
 	},
	
	Timer = func (int time)
	{
		if (time > 100)
		{
			Target->DestroyBlast(this.caused_by_player);
			return FX_Execute_Kill;
		}
		
		/* TODO: Fragment particles do not exist
		Target->CreateFragment("Fragment1", 0, 0, nil, 10); // TODO: random radius between 50 and 150 for this
		
		if (time > 35)
		{
			Target->CreateFragment("Fragment2", 0, 0, nil, 10);
			Target->CreateFragment("Fragment3", 0, 0, nil, 10);
		}
		*/
		/*
		for (var i = 0; i < 10; ++i)
		{
			Target->CastParticles("Fragment1", 1, RandomX(50,150), ox+Random(wdt), oy+Random(hgt), 20, 30);		
		}
		if (time > 35)
		{
			for (var i = 0; i < 10; ++i)
			{
				Target->CastParticles("Fragment2", 1, RandomX(50,150), ox+Random(wdt), oy+Random(hgt), 20, 30);
				Target->CastParticles("Fragment3", 1, RandomX(50,150), ox+Random(wdt), oy+Random(hgt), 20, 30);	
			}		
		}
		*/
		return FX_OK;
	},
};


func MaxHitPoints()
{
	return Max(1, this->~GetHitPoints());
}


func CreateParticleInRectangle(string name, proplist rect, xdir, ydir, lifetime, proplist properties, int amount)
{
	return CreateParticle(name, PV_Random(rect.x, rect.x + rect.wdt), PV_Random(rect.y, rect.y + rect.hgt), xdir, ydir, lifetime, properties, amount);
}


func CreateFragment(name, xdir, ydir, proplist properties, int amount)
{
	return CreateParticleInRectangle(name, GetShapeRectangle(), xdir, ydir, 50, properties ?? Particles_WoodChip(), amount);
}
