/* --- Properties --- */

local ActMap = {
	Move = {
		Prototype = Action,
		Name = "Move",
		Procedure = DFA_FLOAT,
		Speed = 100,
		Accel = 2,
		Decel = 5,
		Length = 1,
		Delay = 5,
		FacetBase = 1,
		NextAction = "Move",
		StartCall = "Processing",
	},
	
	Stop = {
		Prototype = Action,
		Name = "Stop",
		Procedure = DFA_FLOAT,
		Length = 1,
		Delay = 5,
		FacetBase = 1,
		NextAction = "Stop",
		StartCall = "Processing",
	},

	Drill = {
		Prototype = Action,
		Name = "Drill",
		Procedure = DFA_FLOAT,
		Speed = 100,
		Accel = 2,
		Decel = 10,
		Length = 1,
		Delay = 1,
		FacetBase = 1,
		NextAction = "Drill",
		StartCall = "Drilling",
		Sound = "Structures::Elevator::Drilling",
		DigFree = 1
	},
};

local BorderBound = C4D_Border_Top | C4D_Border_Bottom;
local ContactCalls = true;
local MaxDrillDensity = 80; // Density of granite, anything else cannot be drilled;

local derrick;

/* --- Engine Callbacks --- */

func Initialize()
{
	this.ActMap = { Prototype = this.ActMap };
	this.ActMap.Drill = { Prototype = this.ActMap.Drill };
	SetLightRange(20);
}

func ContactBottom()
{
	Halt();
}

/* --- Movement --- */


func SetMoveDirection(int dir, bool drill)
{
	// No change?
	if (dir == COMD_Up && (GetYDir() < 0)) return;
	if (dir == COMD_Down && (GetYDir() > 0)) return;

	if (dir == COMD_Stop)
	{
		return Halt();
	}

	var action = "Move";
	if (drill)
	{
		action = "Drill";
	}

	SetAction(action);
	SetComDir(dir);
	if (derrick)
	{
		derrick->~StartEngine();
	}
}

func Halt(bool user_requested)
{
	if (derrick)
	{
		derrick->~StopEngine();
	}

	// Clear speed.
	SetAction("Stop");
	SetYDir();
	SetComDir(COMD_Stop);
}

/* --- Internals --- */

func SetRig(object rig)
{
	derrick = rig;
}

func Processing()
{
	if (derrick
	&&  derrick->~AcceptTransfer()
	&&  derrick->~HasEnoughPower())
	{
		// Materialtransfer
		//for (var i = 0; i < 3; i++)	
		//	ObjectInsertMaterial(ExtractLiquid(0, 0), pPumpTarget);
		// Blubbern
		//if (!Random(5)) Bubble(0, 0);
	}
}


func Drilling()
{
	var drill_material = GetMaterial(0, 3);
	var density = GetMaterialVal("Density", "Material", drill_material);	
	
	if ((Material("Vehicle") == drill_material)
	||  (Material("Everrock") == drill_material))
	{
		Sound("Objects::Pickaxe::ClangHard?");
		Sparks();
		return;
	}

	// Go fast in empty material
	var density_solid = 50;
	if (density < density_solid)
	{
		SetDrillSpeed(150);
		return;
	}
	else
	{
		var range = density - density_solid;
		var range_max = MaxDrillDensity - density_solid;
		SetDrillSpeed(BoundBy(100 - range * 90 / range_max, 10, 100));
	}
	// Reset to default speed?
	if (GetMaterialVal("DigFree", "Material", drill_material)
	||  GetMaterialVal("Instable", "Material", drill_material))
	{
		DigFreeRect(GetX() - 1, GetY(), 3, 4, true);
	}
	else // Go slow in hard material
	{
		ClearFreeRect(GetX() - 1, GetY(), 3, 4);
		if (!Random(5))
		{
			Sound("Objects::Pickaxe::Clang?");
			Sparks();
		}
	}
}

func SetDrillSpeed(int speed)
{
	this.ActMap.Drill.Speed = speed ?? 100;
}

func Sparks()
{
	var spark = Particles_Glimmer();
	spark.B = 255;
	spark.R = PV_Random(0, 128, 2);
	spark.OnCollision = PC_Bounce();
	CreateParticle("StarSpark", 0, 0, PV_Random(-30, 30), PV_Random(-30, 30), PV_Random(10, 50), spark, 1);
}
