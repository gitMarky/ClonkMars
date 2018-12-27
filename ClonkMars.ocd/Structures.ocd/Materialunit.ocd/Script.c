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

local smelter_light;

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

func Destruction()
{
	RemoveSmelterLight();
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
	if (product->~IsBucketMaterial())
	{
		PlaySoundCrushing();
	}
	PlaySoundWorking(product, true);
	return _inherited(product, ...);
}

func OnProductionHold(id product)
{
	work_timer.paused = true;
	PlaySoundWorking(product, false);
	return _inherited(product, ...);
}

func OnProductionContinued(id product)
{
	work_timer.paused = false;
	PlaySoundWorking(product, true);
	return _inherited(product, ...);
}

func OnProductionFinish(id product)
{
	RemoveEffect(nil, this, work_timer);
	PlaySoundWorking(product, false);
	return _inherited(product, ...);
}

func OnProductionProgress(id product, int progress)
{
	if (product->~IsFoundryProduct())
	{
		var length = GetAnimationLength("smelt");
		work_position = BoundBy(progress * length / 1000, 0, length);

		if (Inside(progress, 210, 570))
		{
			var exit_x = 12 * GetCalcDir();
			var exit_y = 9;
			CreateParticle("Fire", PV_Random(exit_x - 2, exit_x + 2), exit_y, PV_Random(-1, 1), 0, PV_Random(18, 20),
			{
				Prototype = Particles_Fire(),
				ForceY = 5,
				Size = PV_Linear(PV_Random(2, 4), PV_Random(4, 6)),
			}, 5);
		}

		var material = "cm_smelter_melt";
		if (progress >= 865)
		{
			material = "cm_smelter_melt";
			RemoveSmelterLight();
		}
		else if (progress >= 800)
		{
			material = "cm_smelter_melt_cooling2";
		}
		else if (progress >= 700)
		{
			material = "cm_smelter_melt_cooling1";
			if (smelter_light)
			{
				smelter_light->SetLightColor(RGB(150, 55, 25));
			}
		}
		else if (progress >= 570)
		{
			material = "cm_smelter_melt_hot2";
		}
		else if (progress >= 400)
		{
			material = "cm_smelter_melt_hot1";
		}
		else if (progress >= 210)
		{
			AddSmelterLight();
		}

		SetMeshMaterial(material, 6);
	}
	else
	{
		work_position = 0;
	}
	SetAnimationPosition(work_animation, Anim_Const(work_position));
	return _inherited(product, ...);
}

/* --- Effects --- */

func PlaySoundCrushing()
{
	Sound("Structure::MaterialUnit::Melt_Crushing");
}

func PlaySoundWorking(id product, bool on)
{
	if (product->~IsBucketMaterial())
	{
		return;
	}

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

func AddSmelterLight()
{
	if (!smelter_light)
	{
		smelter_light = CreateObject(Dummy, 0, 0, NO_OWNER);
		smelter_light->SetObjectLayer(smelter_light);
		smelter_light.Visibility = VIS_All;
		smelter_light->SetLightRange(40);
	}
	smelter_light->SetLightColor(RGB(255, 96, 0));
}

func RemoveSmelterLight()
{
	if (smelter_light)
	{
		smelter_light->RemoveObject();
	}
}
