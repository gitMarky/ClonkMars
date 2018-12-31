#include Pipe

/* --- Properties --- */

local Name = "$Name$";
local Description = "$Description$";
local Collectible = 1;
local Components = {Metal = 1};


/* --- Display --- */

public func GetCarryBone() { return "main"; }

public func GetCarryMode(object clonk, bool secondary, bool inactive)
{
	if (secondary || inactive)
	{
		return CARRY_None;
	}
	return CARRY_BothHands;
}

public func GetCarryTransform(object user)
{
	return Trans_Mul(Trans_Rotate(90, 0, 0, 1), Trans_Translate(0, -1000, 0), Trans_Scale(800));
}

public func GetCarryPhase()
{
	return 1000;
}

public func Definition(proplist type)
{
	type.PictureTransformation = Trans_Mul(Trans_Rotate(-20, 1, 0, 0), Trans_Rotate(135, 0, 1, 0), Trans_Translate(1500, -500, 0));
	type.MeshTransformation = Trans_Scale(800);
}
