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
local work_position;

/* --- Engine Callbacks --- */


func Definition(type)
{
	type.PictureTransformation = Trans_Mul(Trans_Rotate(20, 0, 1, 0), Trans_Rotate(-2, 0, 0, 1), Trans_Translate(-7000,  -15000, 40000), Trans_Scale(800));
	return _inherited(type, ...);
}


func Initialize()
{
	work_position = 0;
	work_animation = PlayAnimation("smelt", 1, Anim_Const(work_position));
	return _inherited(...);
}


/* --- Production --- */

func IsProduct(id product_id)
{
	return product_id->GetCategory() & C4D_Object;
}

func ProductionTime(id product) { return _inherited(product, ...) ?? 180; }


func OnProductionStart(id product)
{
	work_timer = CreateEffect(FxWorkTimer, 1, 3);
	PlaySoundWorking(true);
	return _inherited(product, ...);
}

func OnProductionHold(id product)
{
	work_timer.paused = true;
	PlaySoundWorking(false);
	return _inherited(product, ...);
}

func OnProductionContinued(id product)
{
	work_timer.paused = false;
	PlaySoundWorking(true);
	return _inherited(product, ...);
}

func OnProductionFinish(id product)
{
	RemoveEffect(nil, this, work_timer);
	PlaySoundWorking(false);
	return _inherited(product, ...);
}

func OnProductionProgress(id product, int progress)
{
	// TODO: Do this only for metal
	var length = GetAnimationLength("smelt");
	work_position = BoundBy(progress * length / 1000, 0, length);
	SetAnimationPosition(work_animation, Anim_Const(work_position));
	return _inherited(product, ...);
}

/* --- Effects --- */

func PlaySoundCrushing()
{
	Sound("Structure::MaterialUnit::Melt_Crushing");
}

func PlaySoundWorking(bool on)
{
	var loop;
	if (on)
	{
		loop = 1;
	}
	else
	{
		loop = -1;
	}

	Sound("Structure::MaterialUnit::Melt_Working", {loop_count = loop});
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

local FxWorkTimer = new Effect
{
	Timer = func ()
	{
		if (!this.paused)
		{
			if (!Random(2))
				this.Target->Smoke(17, Random(3) - 19, Random(9) + 20);
			if (!Random(4))
				this.Target->Smoke(17, Random(3) - 15, Random(5) + 20);
		}
	},
};
