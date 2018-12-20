#include Library_MarsStructure
#include Library_MarsResearch
#include Library_MarsProducer
#include Library_ConstructionAnimation
#include Library_DoorControlFx
#include Library_DamageControl
#include Library_OxygenSupplier


public func PowerNeed() { return 100; }

/* --- Properties --- */

local Name = "$Name$";
local Description = "$Description$";
local ContainBlast = true;
local HitPoints = 70;
local Components = { Metal=2, Plastic=2 };

local AnimName_DoorOpen = "door_open";
local AnimName_DoorOpened = "door_openend"; // Yes, the typo is in the animation...
local AnimName_DoorClose = "door_close";

local AnimBone_Construction = "base";

local work_animation;
local work_timer;

/* --- Engine Callbacks --- */


func Definition(def)
{
	//def.MeshTransformation = Trans_Mul(Trans_Scale(800), Trans_Translate(2000, 0, 0), Trans_Rotate(8, 0, 1, 0));
	//def.PictureTransformation = Trans_Mul(Trans_Translate(0, -25000, 50000), Trans_Scale(600));
	return _inherited(def, ...);
}


func Initialize()
{
	work_animation = PlayAnimation("smelt", 1, Anim_Const(0));
	return _inherited(...);
}


/* --- Production --- */

func IsProduct(id product_id)
{
	return product_id->GetCategory() & C4D_Object;
}

func ProductionTime(id product) { return _inherited(product, ...) ?? 400; }


func OnProductionStart(id product)
{
	work_timer = CreateEffect(FxSmeltAnimation, 1, 1);
	return _inherited(product, ...);
}

func OnProductionHold(id product)
{
	work_timer.paused = true;
	return _inherited(product, ...);
}

func OnProductionContinued(id product)
{
	work_timer.paused = false;
	return _inherited(product, ...);
}

func OnProductionFinish(id product)
{
	RemoveEffect(nil, this, work_timer);
	return _inherited(product, ...);
}

local FxSmeltAnimation = new Effect
{
	Timer = func ()
	{
		if (!this.paused)
		{
			var advance = 50;
			var length = this.Target->GetAnimationLength("smelt");
			var position = (this.Target->GetAnimationPosition(this.Target.work_animation) + advance) % length;

			this.Target->SetAnimationPosition(this.Target.work_animation, Anim_Const(position));
		}
	},
};
