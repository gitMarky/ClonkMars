#include Library_MarsStructure
#include Library_MarsResearch
#include Library_ConstructionAnimation
#include Library_DoorControlFx
#include Library_DamageControl
#include Library_OxygenSupplier


/* -- Structure properties -- */

func ClonkCapacity() {	return 3; } // TODO: this was probably used for reproduction stuff - not used yet


/* -- Properties -- */

local Name = "$Name$";
local Description = "$Description$";
local ContainBlast = true;
local HitPoints = 50;
local Components = { Metal = 2, Plastic = 2};

func Definition(id type)
{
	type.PictureTransformation = Trans_Mul(Trans_Scale(800, 800, 800), Trans_Translate(5000, -7500, +40000));
}


/* -- Door control -- */

func SoundOpenDoor()
{
	SetAction("OpenDoor");
}


func SoundCloseDoor()
{
	SetAction("CloseDoor");
}

/* -- Callbacks -- */

func InitializeStructure()
{
	_inherited(...);
	GivePlayerBasicKnowledge(GetOwner());
}
