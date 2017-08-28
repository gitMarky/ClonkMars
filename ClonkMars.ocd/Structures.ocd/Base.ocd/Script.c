#include Library_MarsStructure
#include Library_MarsResearch
#include Library_DoorControlFx
#include Library_DamageControl
#include Library_OxygenSupplier


/* -- Structure properties -- */

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

/* -- Callbacks -- */

private func InitializeStructure()
{
	_inherited(...);
	GivePlayerBasicKnowledge(GetOwner());
}
