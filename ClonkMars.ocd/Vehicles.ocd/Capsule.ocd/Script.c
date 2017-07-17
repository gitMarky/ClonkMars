
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

/* -- Engine callbacks -- */

private func Initialize() 
{
	capsule = {
		port = nil,
		port_vertex = -1,
		origin_x = GetX(),
		max_speed  = 2500, //iPrecision = 500
		land_speed = 250,
		acceleration = 6,
		automatic = false,
		thrust_vertical = 0,
		thrust_horizontal = 0,
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
		distance_y = Abs(GetY() - port->GetY());
		port->Occupy(this);
		capsule.origin_x = port->GetX();
	}
	else
	{
		distance_y = Min(Abs(GetY() - GetHorizon(-24)), Abs(GetY() - GetHorizon(24))); // distance to ground
		distance_y -= 150; // add a safety buffer
		capsule.origin_x += RandomX(-400, 400);
	}
	if (distance_y < 1) distance_y = 1;
	
	Log("Capsule: Distance to landing zone %d", distance_y);
	
	
	var time_interval = Max(1, distance_y / (5 * GetGravity()));
	var time_freefall = 0;
	var magic_number = 1;
	var acceleration_capsule = capsule.acceleration / magic_number;
	var acceleration_gravity = GetGravity() / magic_number;
	var land_velocity = capsule.land_speed / magic_number;
	
	Log("Capsule: time_interval = %d", time_interval);
	
	while (true)
	{
		time_freefall += time_interval;
		var i = time_freefall;
		var distance = 0;
		var velocity = 0;
		while (i--)
		{
			velocity += acceleration_gravity;
			distance += velocity;
		}
		while (velocity > land_velocity)
		{
			velocity -= acceleration_capsule;
			distance += velocity;
		}
		if (distance / magic_number > distance_y)
		{
			if (time_interval == 1)
			{
				break;
			}
			else
			{
				time_freefall -= 2 * time_interval;
				time_interval = Max(1, time_interval / 2);
			}
		}
	}
	
	Log("Capsule: time_freefall = %d", time_freefall);
	
	/* TODO
	if (AdvancedWindCalculations())
	{
		var dir,pos=0;
		for (var i;i<t;++i)
		{
			//FIXME: Use forecast. And fix that whole thingy
			dir += (dir - GetWind(0, 0, true))**2 * WindEffect() / 1000;
			pos += dir;
		}
		if (GetWind(0, 0, true) < 0)
		{
			pos *= -1;
		}
		SetPosition(GetX() - pos / 100, GetY());
		AddEffect("FxBlowout", this, 1, 1,this);
	}
	*/
	ScheduleCall(this, this.StartLanding, time_freefall);
	if (capsule.port)
	{
		ScheduleCall(capsule.port, capsule.port.PortActive, 50);
	}
	capsule.automatic = auto;
}


private func StartLanding()
{
	if (!capsule.thrust_vertical)
	{
		// activate the port a second time in case that the landing started too early
		if (capsule.port)
		{
			capsule.port->PortActive();
		}
		SetVerticalThrust(1);
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
			
			// Counter rotation
			if (Target->GetR() < -1 && Target->GetRDir() < 1)
			{
				Target->SetRDir(Target->GetRDir(50)+1, 50);
			}
			else if (Target->GetR() > 1 && Target->GetRDir() > -1)
			{
				Target->SetRDir(Target->GetRDir(50)-1, 50);
			}
			
			var prec = 500;
			
			// Acceleration
			var accspeed = Min(Target.capsule.max_speed - Cos(Target->GetR()-Angle(0,0,Target->GetXDir(prec),Target->GetYDir(prec)),Distance(Target->GetYDir(prec), Target->GetXDir(prec))), Target.capsule.acceleration+GetGravity());
			if (Target.capsule.thrust_vertical != 1 || Target->GetYDir(prec) > Target.capsule.land_speed)
			{
				Target->SetXDir(Target->GetXDir(prec)+Sin(Target->GetR(),accspeed), prec);
				Target->SetYDir(Target->GetYDir(prec)-Cos(Target->GetR(),accspeed), prec);
			}
			
			// Selling
			if (Target.capsule.automatic && Target->GetY() <= -20 && Target->GetYDir() < 0)
			{
				for (var item in FindObjects(Find_Container(Target)))
				{
					if (item) item->Sell(Target->GetOwner());
				}
				if (Target.capsule.port)
				{
					Target.capsule.port->PortWait();
				}
				Target->RemoveObject();
				return FX_Execute_Kill;
			}
			
			// Capsule does not start correctly?
			if (Target.capsule.automatic && (time > (LandscapeHeight()* 4 / 3 + 300)) && !Random(10))
			{
				Target->DoDamage(1); 
			}
		}
		var acc = 1;
		if (Target.capsule.automatic) acc = 5; // That's cheating
		if (Target.capsule.thrust_horizontal == 1)
		{
			Target->SetXDir(Max(Target->GetXDir(70)-acc,-100), 70);		
			Target->SetRDir(Target->GetRDir(100)-1, 100);
		}
		else if (Target.capsule.thrust_horizontal == -1)
		{
			Target->SetXDir(Min(Target->GetXDir(70)+acc,+100),70);
			Target->SetRDir(Target->GetRDir(100)+1, 100);
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
};


/* -- Misc -- */

// Proper Sound() stacking emulator
private func StackCapsuleSound(int change)
{
	capsule.stacked_sounds = Max(0, capsule.stacked_sounds + change);
	if (capsule.stacked_sounds < 1) Sound("Jetbelt", 0, 0, 0, -1);
	if (capsule.stacked_sounds > 0) Sound("Jetbelt", 0, 0, 0, +1);
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
