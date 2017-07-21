
// #include storm thingy
#include Library_Structure // sort of - inherits the repair stuff
#include Library_DamageControl
#include Library_Ownable

/* -- Properties --*/

local Name = "$Name$";
local Description = "$Description$";
local HitPoints = 20;
local ContactCalls = true;
local Touchable = 1;
local BorderBound = C4D_Border_Sides;
local capsule; // proplist

static const CAPSULE_Precision = 100; // 1/100 px per tick

/* -- Engine callbacks -- */

private func Initialize() 
{
	capsule = {
		// starting and landing settings
		port = nil,
		port_vertex = -1,
		origin_x = GetX(),
		automatic = false,
		// properties
		max_velocity = 500,      // in CAPSULE_PRECISION: px / tick
		max_velocity_land = 250, // in CAPSULE_PRECISION: px / tick
		max_acceleration = 30,   // in CAPSULE_PRECISION: px / tick^2
		max_rotation = 1000,     // in CAPSULE_PRECISION: degrees
		// control
		thrust_vertical = 0,     // in per mille
		thrust_horizontal = 0,
		target_rotation = 0, 	 // in CAPSULE_PRECISION: degrees
		// misc
		stacked_sounds = 0,
	};

	SetAction("FreeFall");
	SetComDir(COMD_Down);
	SetYDir();

	return _inherited();
}


private func Hit(int xdir, int ydir)
{
	var hit = Distance(xdir, ydir) - 210;
	
	if (hit >= 270)
	{
		DestroyBlast();
	}
	else if (hit > 0)
	{
		DoDamage(hit / 9);
	}
}

/* -- Landing system -- */

public func SetLandingDestination(object port, bool auto)
{
	if (GetGravity() < 1)
	{
		FatalError("Cannot land without gravity.");
	}
	capsule.port = port;
	
	// Determine distance to the landing position
	var distance_y;
	if (port)
	{
		distance_y = Abs(GetY(CAPSULE_Precision) - port->GetY(CAPSULE_Precision));
		port->Occupy(this);
		capsule.origin_x = port->GetX();
	}
	else
	{
		distance_y = Min(Abs(GetY() - GetHorizon(-24)), Abs(GetY() - GetHorizon(24))); // distance to ground
		distance_y -= 5; // add a small safety buffer
		distance_y *= CAPSULE_Precision;
		capsule.origin_x += RandomX(-400, 400);
	}
	if (distance_y < 1) distance_y = 1;

	var acceleration_gravity = GetGravity(); // the usual gravity
	var acceleration_capsule = -capsule.max_acceleration; // accelerate upwards
	var velocity_capsule = this->GetYDir(CAPSULE_Precision);
	
	// calculate the minimum time to fall down the entire distance, with the quadratic formula, for a = 0.5*acceleration_gravity, b = velocity_capsule, c = -distance_y;
	// why? if cancelling the gravity with an upwards boost in the landing phase it will take longer
	var time_forecast = (Sqrt(velocity_capsule**2 + 2 * distance_y * acceleration_gravity) -  velocity_capsule);
	Log("Capsule landing parameters: velocity_capsule = %d, acceleration_capsule = %d, acceleration_gravity = %d, forecast %d", velocity_capsule, acceleration_capsule, acceleration_gravity, time_forecast);
	time_forecast /= acceleration_gravity;

	// brute force determine the landing time at first - could probably be solved with an equation system and some sensible boundary conditions, but I am too lazy for that
	var time_freefall = time_forecast / 2;

	Log("Capsule: Distance to landing zone %d, time forecast %d", distance_y, time_forecast);
	
	// maximize time in freefall
	// used the following equations / conditions
	// E1) distance_y = distance_fall + distance_land
	// E2) distance_fall = velocity_capsule * time_fall + 0.5 * acceleration_gravity * time_fall^2
	// E3) distance_land = velocity_fall * time_land + 0.5 * (acceleration_gravity + acceleration) * time_land^2
	// E4) velocity_fall = velocity_capsule + acceleration_gravity * time_fall
	// E5) velocity_land = velocity_fall + (acceleration_gravity + acceleration) * time_land
	// <=> (acceleration_gravity + acceleration) * time_land = velocity_fall - velocity_land
	// E5 in E3 => E6) 2 * distance_land = time_land * (velocity_land + velocity_fall)
	// C1) 0 <= velocity_land <= capsule.max_velocity_land
	// C2) distance_land >= 0
	// C3) time_land >= 0
	var optimize = true;
	for (var time_fall = time_forecast; time_fall > 0 && optimize; --time_fall)
	{
		for (var velocity_land = 0; velocity_land <= capsule.max_velocity_land && optimize; velocity_land += 10) // minimize landing velocity
		{			
			var velocity_fall = velocity_capsule + acceleration_gravity * time_fall;
			var distance_fall = time_fall * (velocity_capsule  + velocity_fall) / 2; // E2) transformed a little
			
			var distance_land = distance_y - distance_fall;
			if (distance_land <= 0) continue;
			
			var time_land = (2 * distance_land) / (velocity_land + velocity_fall); // E6) transformed
			if (time_land <= 0) continue;

			var acceleration = 2 * (distance_land - velocity_fall * time_land) / (time_land ** 2); // E3) transformed
			acceleration -= acceleration_gravity;

			//Log("* time_freefall = %d, distance_fall = %d, velocity_fall = %d, distance_land = %d, velocity_land %d, acceleration = %d", time_freefall, distance_fall, velocity_fall, distance_land, velocity_land, acceleration);
			if (acceleration >= 0 || acceleration < acceleration_capsule) continue;
			optimize = false;
			time_freefall = time_fall;
			acceleration_capsule = acceleration;
			//Log("==> Found optimum");
		}
	}

	Log("Capsule: time_freefall = %d", time_freefall);
	/*
	if (this->~AdvancedWindCalculations())
	{
		var dir, pos=0;
		for (var i; i < time_freefall; ++i)
		{
			//FIXME: Use forecast. And fix that whole thingy
			dir += (dir - GetWind(0, 0, true))**2 * this->WindEffect() / 1000;
			pos += dir;
		}
		if (GetWind(0, 0, true) < 0)
		{
			pos *= -1;
		}
		SetPosition(GetX() - pos / 100, GetY());
		//StartThruster(); no idea why it starts the thruster right now
	}
    */
	ScheduleCall(this, this.StartLanding, time_freefall, nil, Abs(acceleration_capsule));
	if (capsule.port)
	{
		ScheduleCall(capsule.port, capsule.port.PortActive, 50);
	}
	capsule.automatic = auto;
}


private func StartLanding(int override_acceleration) // TODO: the override is not used yet
{
	if (!capsule.thrust_vertical)
	{
		// activate the port a second time in case that the landing started too early
		if (capsule.port)
		{
			capsule.port->PortActive();
		}
		var per_mille = 1000;
		SetVerticalThrust(BoundBy(override_acceleration * per_mille / capsule.max_acceleration, 0, per_mille));
	}
}

/* -- Thruster control -- */

public func SetHorizontalThrust(int bo)
{
	if (capsule.thrust_horizontal != bo)
	{
		if (bo && !capsule.thrust_horizontal)
		{
			StackCapsuleSound(1);
		}
		else if (!bo)
		{
			StackCapsuleSound(-1);
		}
		capsule.thrust_horizontal = bo;
		
		if (bo == -1)    SetAction("LeftBoostTurnUp");
		else if (bo == 1) SetAction("RightBoostTurnUp");
		else if (WildcardMatch(GetAction(), "RightBoost*")) SetAction("RightBoostTurnDown");
		else SetAction("LeftBoostTurnDown");
	}
	if (bo)
	{
		StartThruster();
	}
}


public func SetVerticalThrust(int bo)
{
	if (capsule.thrust_vertical != bo)
	{
		StackCapsuleSound(bo - capsule.thrust_vertical);
		capsule.thrust_vertical = bo;
		if (bo)
		{
			StartThruster();
		}
	}
}


private func StartThruster()
{
	if (!GetEffect("FxBlowout", this))
	{
		CreateEffect(FxBlowout, 1, 1);
	}
}


private func StopThruster()
{
	RemoveEffect("FxBlowout", this);
}


private func IsThrusterOn()
{
	return GetEffect("FxBlowout", this);
}


public func IsBlowingOut() // seems to be unused, but kept for compatibility reasons for now
{
	IsThrusterOn();
}


local FxBlowout = new Effect
{
	Timer = func (int time)
	{
		NormalizeRotation();
		if (Target.capsule.thrust_vertical)
		{
			// Effects
			/* TODO
			var clr = RGBa(200,200,255);
			for (var i = RandomX(5, 8); i; --i)
			{
				CreateParticle("Thrust", -Sin(GetR(),8)+Cos(GetR(),-10), Cos(GetR(),8)-Sin(GetR(),11),  -Sin(GetR(),15)+RandomX(-1,1)+GetXDir(), Cos(GetR(),15+Random(10))+GetYDir()/2, 45, clr, this);
				CreateParticle("Thrust", -Sin(GetR(),11),                Cos(GetR(),11),                -Sin(GetR(),15)+RandomX(-1,1)+GetXDir(), Cos(GetR(),15+Random(10))+GetYDir()/2, 45, clr, this);
				CreateParticle("Thrust", -Sin(GetR(),8)+Cos(GetR(),11),  Cos(GetR(),8)-Sin(GetR(),-11), -Sin(GetR(),15)+RandomX(-1,1)+GetXDir(), Cos(GetR(),15+Random(10))+GetYDir()/2, 45, clr, this);
			}

			EffectDust();
			if (capsule.thrust_vertical == 2) EffectDust();
			*/

			ApplyThrust();

			if (AutomaticCapsuleControl(time))
			{
				Target->RemoveObject();
				return FX_Execute_Kill;
			}
		}
		var velocity_min_x = 100;
		var acceleration_x = 1;
		var precision_xdir = 70;
		var precision_rdir = 100;
		if (Target.capsule.thrust_horizontal == 1)
		{
			Target->SetXDir(Max(Target->GetXDir(precision_xdir) - acceleration_x, -velocity_min_x), precision_xdir);		
			Target->SetRDir(Target->GetRDir(precision_rdir) - 1, 100);
		}
		else if (Target.capsule.thrust_horizontal == -1)
		{
			Target->SetXDir(Min(Target->GetXDir(precision_xdir) + acceleration_x, +velocity_min_x), precision_xdir);
			Target->SetRDir(Target->GetRDir(precision_rdir) + 1, precision_rdir);
		}
		else if (Target.capsule.thrust_vertical == 0 && !Target.capsule.automatic) // This means that both are 0 and the capsule is hand controlled
		{ 
			return FX_Execute_Kill;
		}
		
		if (Target.capsule.automatic && Target->~AdvancedWindCalculations())
		{
			var nbo;

			Message("%v %v %v", this, Target->GetXDir(), Target->GetX() - Target.capsule.origin_x, GetWind());
			
			if (     Target->GetXDir() >= -17 && Target->GetX() > Target.capsule.origin_x) nbo = 1;
			else if (Target->GetXDir() <=  17 && Target->GetX() < Target.capsule.origin_x) nbo = -1;

			if (Abs(Target->GetR()) > 8)
			{
				if (Target->GetRDir() > 0) Target->SetRDir(Target->GetRDir(100)-1, 100);
				else Target->SetRDir(Target->GetRDir(100)+1, 100);
				nbo = 0;
			}

			if (nbo != Target.capsule.thrust_horizontal) Target->SetHorizontalThrust(nbo);
		}
	},
	
	NormalizeRotation = func ()
	{
		var angle = BoundBy(Target.capsule.target_rotation ?? 0, -Target.capsule.max_rotation, +Target.capsule.max_rotation) - Target->GetR() * CAPSULE_Precision;
		angle = Normalize(angle, -180 * CAPSULE_Precision, CAPSULE_Precision);

		var precision = CAPSULE_Precision;
		if (Target.capsule.thrust_vertical) precision /= 2; // rotate faster if thruster is on

		var angular_acceleration = 1;

		if (angle > 1 && Target->GetRDir() < 1)
		{
			Target->SetRDir(Target->GetRDir(precision) + angular_acceleration, precision);
		}
		else if (angle < -1 && Target->GetRDir() > -1)
		{
			Target->SetRDir(Target->GetRDir(precision) - angular_acceleration, precision);
		}
		else if (angle == 0)
		{
			Target->SetRDir();
		}
		Log("%d", Target->GetR() * CAPSULE_Precision);
	},
	
	AutomaticCapsuleControl = func (int time)
	{
		if (!Target.capsule.automatic) return false;
	
		// Selling
		if (Target->GetY() <= -20 && Target->GetYDir() < 0)
		{
			for (var item in FindObjects(Find_Container(Target)))
			{
				if (item) item->Sell(Target->GetOwner()); // TODO: needs a vendor object
			}
			if (Target.capsule.port)
			{
				Target.capsule.port->PortWait();
			}
			return true;
		}
		
		// Stuck or something?
		if (time > (LandscapeHeight() * 4 / 3 + 300) && !Random(10))
		{
			Target->DoDamage(1); 
		}
		return false;
	},

	ApplyThrust = func()
	{
		var per_mille = 1000;
		var acceleration = Target.capsule.thrust_vertical * Target.capsule.max_acceleration / per_mille;
		var angle = Target->GetR()*CAPSULE_Precision;

		var add_xdir = +Sin(angle, acceleration, CAPSULE_Precision);
		var add_ydir = -Cos(angle, acceleration, CAPSULE_Precision);

		Target->AddSpeed(add_xdir, add_ydir, CAPSULE_Precision);
		Target->Message("Acc: %d", Target.capsule.thrust_vertical);
	},
};



/* -- Misc -- */

// Proper Sound() stacking emulator
private func StackCapsuleSound(int change)
{
	capsule.stacked_sounds = Max(0, capsule.stacked_sounds + change);
	if (capsule.stacked_sounds < 1) Sound("Jetbelt", 0, 0, 0, -1);
	if (capsule.stacked_sounds > 0) Sound("Jetbelt", 0, 0, 0, +1);
}

/* -- User control -- */


public func HoldingEnabled() { return true; }

public func ControlUseStart(object clonk)
{
	return true;
}

public func ControlUseHolding(object clonk, int x, int y)
{
	SetThrust(x, y);
	return true;
}

public func ControlUseStop(object clonk, int x, int y)
{
	ResetThrust();
	return true;
}

public func ContainedUseStart(object clonk, int x, int y)
{
	return true;
}

public func ContainedUseHolding(object clonk, int x, int y)
{
	SetThrust(x, y);
	return true;
}

public func ContainedUseStop(object clonk, int x, int y)
{
	ResetThrust();
	return true;
}


private func SetThrust(int x, int y)
{
	var per_mille = 1000;
	var max = 300; // this many pixels will count as max acceleration
	var max_thrust_angle = 60 * CAPSULE_Precision; 
	var acceleration_per_mille = BoundBy(Distance(x, y) * per_mille / max, 1, per_mille);
	var angle = Normalize(Angle(0, 0, x, y, CAPSULE_Precision), -180 * CAPSULE_Precision, CAPSULE_Precision);

	// set the vertical thruster on, as long as you are in range +/- max_thrust_angle	
	if (Abs(angle) < max_thrust_angle)
	{	
		SetVerticalThrust(acceleration_per_mille);
	}
	else
	{
		capsule.thrust_vertical = nil;
	}
	
	// set the rotation, as long as you aim away far enough (10% of max distance)
	if (acceleration_per_mille > 100)
	{
		capsule.target_rotation = BoundBy(angle, -capsule.max_rotation, +capsule.max_rotation);	
	}
	else
	{
		capsule.target_rotation = nil;
	}
}


private func ResetThrust()
{
	capsule.thrust_vertical = nil;
	capsule.target_rotation = nil;
}


/* -- Actions -- */

local ActMap = {

FreeFall = {
	Prototype = Action,
	Name = "FreeFall",
	Procedure = DFA_NONE,
	Delay = 10,
	FacetBase = 1,
	NextAction = "FreeFall",
},
 
PortLand = {
	Prototype = Action,
	Name = "PortLand",
	Procedure = DFA_ATTACH,
	Delay = 10,
	FacetBase = 1,
	NextAction = "PortLand",
},
 
RightBoostTurnUp = {
	Prototype = Action,
	Name = "RightBoostTurnUp",
	NextAction = "RightBoostMax",
	Prodedure = DFA_NONE,
	Delay = 2 ,
	Length = 4,
	FacetBase = 0,
	X = 0, Y = 45, Wdt = 60, Hgt = 45,
},
 
RightBoostMax = {
	Prototype = Action,
	Name = "RightBoostMax",
	NextAction = "RightBoostMax",
	Prodedure = DFA_NONE,
	Delay = 3,
	Length = 1,
	FacetBase = 0,
	X = 0, Y = 90, Wdt = 60, Hgt = 45,
},
 
RightBoostTurnDown = {
	Prototype = Action,
	Name = "RightBoostTurnDown",
	NextAction = "FreeFall",
	Prodedure = DFA_NONE,
	Delay = 2,
	Length = 4,
	FacetBase = 0,
	X = 0, Y = 45, Wdt = 60, Hgt = 45,
	Reverse=1,
},
 
LeftBoostTurnUp = {
	Prototype = Action,
	Name = "LeftBoostTurnUp",
	NextAction = "LeftBoostMax",
	Prodedure = DFA_NONE,
	Delay = 2,
	Length = 4,
	FacetBase = 0,
	X = 0, Y = 135, Wdt = 60, Hgt = 45,
},
 
LeftBoostMax = {
	Prototype = Action,
	Name = "LeftBoostMax",
	NextAction = "LeftBoostMax",
	Prodedure = DFA_NONE,
	Delay = 3,
	Length = 1,
	FacetBase = 0,
	X = 0, Y = 180, Wdt = 60, Hgt = 45,
},
 
LeftBoostTurnDown = {
	Prototype = Action,
	Name = "LeftBoostTurnDown",
	NextAction = "FreeFall",
	Prodedure = DFA_NONE,
	Delay = 2,
	Length = 4,
	X = 0, Y = 135, Wdt = 60, Hgt = 45,
	Reverse=1,
},
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Legacy

/*
public func GetTemp() { return 250; } // klimatisiert


local sat;





public func SetSat(pSat) {
	sat = pSat;
}

private func DestroyBlast() {
	if (sat) sat -> CapsuleDestroyed();
	return _inherited(...);
}


// hat einen Sauerstoffspeicher zum Landen
public func IsO2Producer() {
	return 1;
}


protected func ContainedDigDouble() {
	if (GetAction() == "FreeFall")
		StartLanding();
	return 1;
}


protected func ContactBottom() { 
	if (capsule.thrust_vertical == 1) {
		if (capsule.automatic)
		{
			StopThruster();
		}
		SetHorizontalThrust(0); SetVerticalThrust(0); 
		if (capsule.port && capsule.port==FindObject(Find_ID(PORT),Find_AtPoint())) {
			capsule.port_vertex = GetVertexNum();
			AddVertex();
			SetVertex(capsule.port_vertex, 2, CNAT_NoCollision, this, 1);
			AddVertex();
			SetVertex(capsule.port_vertex, 0, GetVertex(0, 0, capsule.port)+GetX(capsule.port)-GetX(), this, 2);
			SetVertex(capsule.port_vertex, 1, GetVertex(0, 1, capsule.port)+GetY(capsule.port)-GetY(), this, 2);
			SetAction("PortLand", capsule.port);
			SetActionData(256*capsule.port_vertex, this);
			//FIXME: Do that less hacky...
		} 
		if (ObjectCount(Find_Container(this), Find_OCF(OCF_CrewMember)))
			ScheduleCall(this, "Eject", 30, 0, GetX(), GetY());
		if (capsule.port) ScheduleCall(capsule.port, "PortWait", 50);
	}
	return 1;
}

// Ejects the contained clonks if the given position matches.
protected func Eject(int x, int y) {
	if (capsule.thrust_vertical == 2) return;
	if (GetX() != x || GetY() != y) {
		if (!GetXDir() && !GetYDir())
			// Try again in a moment.
			ScheduleCall(this, "Eject", 10, 0, GetX(), GetY());
		return;
	}
	for (var pObj in FindObjects(Find_Container(this), Find_OCF(OCF_CrewMember)))
		pObj -> Exit();
}

protected func ControlUpDouble() {
	if (capsule.port) capsule.port->PortActive();
	ScheduleCall(this, "Launch", 60);
}

private func Launch() {
	// Remove all bottom vertices to prevent the capsule from stucking.
	for (var vertex in [capsule.port_vertex, 2, 1, 0])
		RemoveVertex(vertex);
	capsule.port_vertex = -1;
	SetAction("FreeFall");
	SetActionData();
	SetVerticalThrust(2);
	capsule.automatic = 1;
}

// geklaut von Hazard
protected func EffectDust() {

	// Dust effect
	var mat,i;
	
	// maximum distance in which the shuttle appears
	var maxdistance = 150;
	
	// search for ground (yomisei: please use your sensor-function for that as soon as you finished it)
	for (i=10; i<maxdistance; i+=5) {
		if (GBackSolid(0,i)) {
			mat = GetMaterial(0,i);
			break;
		}
	}
	
	// ground in distance
	if (i<maxdistance) {
	
		// check if digable
		if (CheckDust(mat)) {
		
			// determine material color
			var rand = Random(3);
			var r = GetMaterialColor(mat,rand,0);
			var g = GetMaterialColor(mat,rand,1);
			var b = GetMaterialColor(mat,rand,2);
			
			// all values dependend on distance
			var size = RandomX(20,300-i/2);
			var alpha = Min(255,30+i);
			var pos = RandomX(0,30);
			// the nearer the dust to the center, the faster it is blown aside 
			CreateParticle("BoostDust",-pos,i,(-50+pos)+GetXDir()/2,RandomX(-9,5),size,RGBa(r,g,b,alpha));
			CreateParticle("BoostDust", pos,i,( 50-pos)+GetXDir()/2,RandomX(-9,5),size,RGBa(r,g,b,alpha));
		}
		if (i < 50) {
			var iX, iY, iM = 70 - i, iC = RGB(iM, iM, iM);
			for (var j = 0; j < 5; ++j) {
				iX = RandomX(-20, 20);
				if (!Random(2))
					iX *= 2;
				iY = 0;
				var max = LandscapeHeight() - GetY();
				while (!GBackSolid(iX, ++iY)) {
					if (iY > max)
						return;
				}
				iY += RandomX(-2, 2) + iM / 3;
				if (GBackSolid(iX, iY))
					SetLandscapePixel(iX, iY, iC);
			}
		}
	}
}


public func Flying() { return !GetContact(this, -1);}

public func WindEffect() { return 100;}

//
public func ContainedUp(pControl) {
	if (GetPlrCoreJumpAndRunControl(GetOwner(pControl))) {
		SetVerticalThrust(1);
	} else {
		SetVerticalThrust(BoundBy(capsule.thrust_vertical-1,0,2));
		SetHorizontalThrust(0);
	}
}

public func ContainedDownDouble(pControl) {return ContainedDown(pControl);}
public func ContainedDown(pControl) {
	if (GetPlrCoreJumpAndRunControl(GetOwner(pControl))) {
		SetVerticalThrust(2);
	} else {
		SetVerticalThrust(BoundBy(capsule.thrust_vertical+1,0,2));
		SetHorizontalThrust(0);
	}
	SetCommand(pControl, "None");
}

public func ContainedLeft(pControl) {
	if (GetPlrCoreJumpAndRunControl(GetOwner(pControl))) {
		SetHorizontalThrust(-1);
	} else {
		if (capsule.thrust_horizontal == 1) SetHorizontalThrust(0);
		else SetHorizontalThrust(-1);
	}
}

public func ContainedRight(pControl) {
	if (GetPlrCoreJumpAndRunControl(GetOwner(pControl))) {
		SetHorizontalThrust(1);
	} else {
		if (capsule.thrust_horizontal == -1) SetHorizontalThrust(0);
		else SetHorizontalThrust(1);
	}
}

public func ContainedUpReleased() { SetVerticalThrust(0); }
public func ContainedDownReleased() { SetVerticalThrust(0); }
public func ContainedLeftReleased() { SetHorizontalThrust(0); }
public func ContainedRightReleased() { SetHorizontalThrust(0); }
public func ContainedDig() { SetHorizontalThrust(0); SetVerticalThrust(0); }



protected func ControlDig(object pClonk)
{
  // Herausnehmen per Graben: Holen-Menü öffnen
  pClonk->SetCommand(0, "Get", this, 0,0, 0, 1);
}
*/
