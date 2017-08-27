#include Library_Structure
#include Library_DoorControlFx
#include Library_DamageControl
#include Library_OxygenSupplier


/* -- Production & research */

public func IsMarsResearch() { return true; }
public func IsBuildableByConKit() { return true; }
public func GetBasementID(){ return Structure_Basement; }
public func GetBasementWidth(){ return 95; }

private func ClonkCapacity() {	return 3; } // TODO: this was probably used for reproduction stuff - not used yet


/* -- Properties -- */

local Name = "$Name$";
local Description = "$Description$";
local ContainBlast = true;
local HitPoints = 50;
local Components = { Metal = 2, Plastic = 2};

private func Definition(id type)
{
	type.PictureTransformation = Trans_Mul(Trans_Scale(800, 800, 800), Trans_Translate(5000, -7500, +40000));
}


/* -- Door control -- */

private func SoundOpenDoor()
{
	SetAction("OpenDoor");
}


private func SoundCloseDoor()
{
	SetAction("CloseDoor");
}


/* -- Actions -- */
/*
local ActMap = {

Door = {
	Prototype = Action,
	Name = "Door",
	Procedure = DFA_NONE,
	Directions = 1,
	Length = 20,
	Delay = 1,
	X = 0,
	Y = 37,
	Wdt = 17,
	Hgt = 26,
	OffX = 18,
	OffY = 8,
	FacetBase = 1,
	Sound = "metaldoor",
},

Wait = {
	Prototype = Action,
	Name = "Wait",
	Procedure = DFA_NONE,
	Directions = 1,
	Length = 1,
	X = 0,
	Y = 0,
	Wdt = 95,
	Hgt = 34,
	OffX = 0,
	OffY = 0,
	FacetBase = 0,
},

Green = {
	Prototype = Action,
	Name = "Green",
	Procedure = DFA_NONE,
	Directions = 1,
	Length = 1,
	X = 189,
	Y = 0,
	Wdt = 95,
	Hgt = 34,
	OffX = 0,
	OffY = 0,
	FacetBase = 0,
},

};
*/

