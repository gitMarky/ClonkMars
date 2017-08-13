/**
	Space suit
	Connected to a pump it provides the user with air underwater.
	
	@author: Marky
	@credits: pluto, Clonkonaut for the diving helmet
*/

#include Library_Wearable
#include Library_Respirator

/*-- Engine Callbacks --*/

func Hit()
{
	Sound("Hits::Materials::Metal::DullMetalHit?");
}

/*-- Callbacks --*/

/*-- Usage --*/

public func ControlUse(object clonk)
{
	if (IsWorn())
	{
		if (GetOxygenInAtmosphere() > BREATHE_MinOxygenInAtmosphere)
		{
			TakeOff();
		}
		else
		{
			clonk->~PlaySoundDoubt();
			clonk->PlayerMessage(clonk->GetOwner(), "$NoAtmosphere$");
		}
	}
	else
	{
		PutOn(clonk);
	}
	clonk->DoBreath(); // force breath update
	return true;
}


public func IsRespiratorFor(object user)
{
	return user == Contained() && IsWorn();
}


/*-- Production --*/

public func IsToolProduct() { return true; }

/*-- Display --*/

public func GetWearPlace()
{
	return WEARABLE_Head;
}

public func GetWearBone()
{
	return WEARABLE_Head;
}

public func GetWearFlags()
{
	return AM_MatchSkeleton;
}


public func GetWearTransform()
{
	return Trans_Identity();
}

public func GetCarryMode(object clonk, bool secondary)
{
	if (IsWorn() || display_disabled)
		return CARRY_None;
	return CARRY_BothHands;
}

public func GetCarryPhase(object clonk)
{
	return 650;
}

public func GetCarryTransform(object clonk, bool secondary, bool no_hand, bool on_back)
{
	return Trans_Mul(Trans_Rotate(80, 0, 0, 1), Trans_Rotate(-90, 0, 1), Trans_Rotate(-45, 0, 0, 1), Trans_Translate(-1000, 4000));
}

func Definition(def)
{
	SetProperty("PictureTransformation", Trans_Mul(Trans_Rotate(45, 0, 1), Trans_Rotate(10, 0, 0, 1)),def);
}

/*-- Properties --*/

local Name = "$Name$";
local Description = "$Description$";
local Collectible = true;
local Components = {Metal = 1, Plastic = 1};
local MaxOxygen = 2430; // allows breathing 90 seconds (69 seconds of O2 + 21seconds clonk breath)
