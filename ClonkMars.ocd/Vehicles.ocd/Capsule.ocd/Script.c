
// #include storm thingy
#include Library_Structure // sort of - inherits the repair stuff
#include Library_DamageControl
#include Library_OxygenSupplier
//#include Library_Ownable // do not use this yet!

/* -- Properties --*/

local Name = "$Name$";
local Description = "$Description$";
local HitPoints = 20;
local ContactCalls = true;
local Touchable = 1;
local BorderBound = C4D_Border_Sides;
local Components = { Metal = 2 };
local capsule; // proplist
local MaxContentsCount = 20; // for loading?

static const CAPSULE_Precision = 100; // 1/100 px per tick

public func IsContainer() { return true; } // can carry items
public func IsVehicle() { return true; }	// not sure where this is used, but the lorry has it

/* -- Engine callbacks -- */

private func Construction()
{
	this.MeshTransformation = Trans_Translate(0, -7000, 0);
	_inherited(...);
}


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
		max_velocity_land = 200, // in CAPSULE_PRECISION: px / tick
		max_acceleration = 30,   // in CAPSULE_PRECISION: px / tick^2
		max_rotation = 1000,     // in CAPSULE_PRECISION: degrees
		damage_velocity = 210,   // in precision 100:     px / tick
		// control
		thrust_vertical = 0,     // in per mille
		thrust_horizontal = 0,
		target_rotation = 0, 	 // in CAPSULE_PRECISION: degrees
		// misc
		stacked_sounds = 0,
		is_landing = false,
	};

	SetAction("FreeFall");
	SetComDir(COMD_Down);
	SetYDir();

	return _inherited();
}


private func Hit(int xdir, int ydir)
{
	var velocity = Distance(xdir, ydir);
	var hit = velocity - capsule.damage_velocity;
	
	Log("[%d] Capsule hit, velocity = %d", FrameCounter(), velocity);

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
	var time_precaution;
	if (port)
	{
		distance_y = Abs(GetY() - port->GetY());
		port->Occupy(this);
		capsule.origin_x = port->GetX();
		time_precaution = 0;
	}
	else
	{
		var dist_left = Abs(GetY() - GetHorizon(-24));
		var dist_right = Abs(GetY() - GetHorizon(+24));
		distance_y = Min(dist_left, dist_right); // distance to ground
		capsule.origin_x += RandomX(-400, 400);
		
		// start engine earlier if the ground is steep, up to 18 frames earlier
		var angle_ground = Normalize(Angle(dist_left, 48, dist_right, 0), -180);
		time_precaution = Abs(angle_ground / 10);
		Log("Capsule will start engine %d frames earlier due to steep ground", time_precaution);
	}
	distance_y -= GetBottom(); // subtract capsule height
	distance_y -= 5; // add a small safety buffer
	distance_y *= CAPSULE_Precision;
	if (distance_y < 1) distance_y = 1;

	var acceleration_gravity = GetGravity(); // the usual gravity
	var acceleration_capsule = -capsule.max_acceleration; // accelerate upwards
	var velocity_capsule = this->GetYDir(CAPSULE_Precision);
	
	// calculate the minimum time to fall down the entire distance, with the quadratic formula, for a = 0.5*acceleration_gravity, b = velocity_capsule, c = -distance_y;
	// why? if cancelling the gravity with an upwards boost in the landing phase it will take longer
	var time_forecast = (Sqrt(velocity_capsule**2 + 2 * distance_y * acceleration_gravity) -  velocity_capsule);
	Log("Capsule landing parameters: distance_y = %d, velocity_capsule = %d, acceleration_capsule = %d, acceleration_gravity = %d, forecast %d", distance_y, velocity_capsule, acceleration_capsule, acceleration_gravity, time_forecast);
	time_forecast /= acceleration_gravity;

	// brute force determine the landing time at first - could probably be solved with an equation system and some sensible boundary conditions, but I am too lazy for that
	var time_freefall = time_forecast / 2;
	
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

			if (acceleration >= 0 || acceleration < acceleration_capsule) continue;
			Log("[%d] Capsule landing parameters: time_freefall = %d, distance_fall = %d, velocity_fall = %d, time_land = %d, distance_land = %d, velocity_land %d, acceleration = %d", FrameCounter(), time_fall, distance_fall, velocity_fall, time_land, distance_land, velocity_land, acceleration);
			optimize = false;
			time_freefall = time_fall - 1; // deduct one frame, because the thrusters react only one frame later
			acceleration_capsule = acceleration;
			//Log("==> Found optimum");
		}
	}

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
    
    // remove frames of freefall as a precaution
    time_freefall = Max(1, time_freefall - time_precaution);
    
    // schedule landing
	ScheduleCall(this, this.StartLanding, time_freefall, nil, Abs(acceleration_capsule));
	if (capsule.port)
	{
		ScheduleCall(capsule.port, capsule.port.PortActive, 50);
	}
	capsule.automatic = auto;
	return time_freefall;
}


private func StartLanding(int override_acceleration)
{
	if (!capsule.thrust_vertical)
	{
		// activate the port a second time in case that the landing started too early
		if (capsule.port)
		{
			capsule.port->PortActive();
		}
		var per_mille = 1000;
		var thrust = BoundBy(override_acceleration * per_mille / capsule.max_acceleration, 0, per_mille);
		Log("[%d] Capsule is landing: Set thrust to %d (%d); velocity_fall = %d", FrameCounter(), thrust, override_acceleration, Distance(GetXDir(CAPSULE_Precision), GetYDir(CAPSULE_Precision)));
		SetVerticalThrust(thrust);
		capsule.is_landing = true;
	}
}

/* -- Thruster control -- */

public func SetHorizontalThrust(int bo)
{
	if (capsule.thrust_horizontal != bo)
	{
		capsule.thrust_horizontal = bo;
		PlaySoundJetUpdate();
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
		capsule.thrust_vertical = bo;
		PlaySoundJetUpdate();
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
			ParticleFx();
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
	},
	
	AutomaticCapsuleControl = func (int time)
	{
		// landing control: do not skyrocket
		if (Target.capsule.is_landing)
		{
			var reduce_thrust = false;
			if (Target->GetYDir() < 0)
			{
				Target->SetYDir();
				reduce_thrust = true;
			}
			
			if (Target->GetYDir(CAPSULE_Precision) < Target.capsule.max_velocity_land / 2)
			{
				reduce_thrust = true;
			}
			
			if (reduce_thrust)
			{
				var thrust_new = 9 * Target.capsule.thrust_vertical / 10;
				
				if (thrust_new > 0)
				{
					Target->SetVerticalThrust(thrust_new);
				}
				else
				{
					Target->ResetThrust();
				}
			}
		}
	
		// everything else is available in automatic mode only:
		if (!Target.capsule.automatic) return false;
			
		// selling
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
		
		// stuck or something?
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
		
		Log("[%d] Thrust: acceleration = %d, add_ydir = %d, velocity = %d", FrameCounter(), acceleration, add_ydir, Distance(Target->GetXDir(CAPSULE_Precision), Target->GetYDir(CAPSULE_Precision)));

		Target->AddSpeed(add_xdir, add_ydir, CAPSULE_Precision);
		Target->Message("Acc: %d", Target.capsule.thrust_vertical);
	},
	
	ParticleFx = func()
	{
		var xdir = -Sin(Target->GetR(),15) +RandomX(-1,1) + Target->GetXDir();
		var ydir = +Cos(Target->GetR(),15 + Random(5)) + Target->GetYDir()/2;
		var size = 5 + Target.capsule.thrust_vertical / 100;
		var lifetime = size;
		var props =
		{
			Prototype = Particles_Thrust(size),
			Alpha = PV_Linear(128, 0),
			BlitMode = GFX_BLIT_Additive,
			R = 200, G = 200, B = 255,
			Size = PV_KeyFrames(0, 0, 0, 50, size, 1000, size / 2), // these ones shrink
		};
		for (var i = RandomX(5, 8); i; --i)
		{
			Target->CreateParticle("Thrust", -Sin(Target->GetR(), 8) + Cos(Target->GetR(), -10), Cos(Target->GetR(), 8) - Sin(Target->GetR(), +11), xdir, ydir, lifetime, props);
			Target->CreateParticle("Thrust", -Sin(Target->GetR(), 11),                           Cos(Target->GetR(), 11),                           xdir, ydir, lifetime, props);
			Target->CreateParticle("Thrust", -Sin(Target->GetR(), 8) + Cos(Target->GetR(), +11), Cos(Target->GetR(), 8) - Sin(Target->GetR(), -11), xdir, ydir, lifetime, props);
		}

		ParticleFxDust();
	},
	
	ParticleFxDust = func ()
	{
		// distance from the ground when dust starts appearing
		var dust_distance = 50;
		var burn_distance = 50;
		var max_y = Target->GetY() + dust_distance;
		var x = Target->GetX();
		var y;
		var ground_material = -1; // sky material
		
		// find out if there is ground at all
		for (y = Target->GetY() + 10; y < max_y; y += 5)
		{
			if (GBackSolid(x, y))
			{
				ground_material = GetMaterial(x, y);
				break;
			}
		}
	
		var distance = y - Target->GetY();

		// create dust?
		if (GetMaterialVal("DigFree", "Material", ground_material) != 0)
		{
			// some values depend on distance
			var size = RandomX(4, Max(6, (600 - distance) / 10));
			var alpha = Max(0, (225 - distance) * 100 / 225);
			var texture = GetTexture(x, y);
			if (texture)
			{
				var particles = Particles_DustAdvanced(texture, size, alpha);
				particles.Rotation = PV_Direction();
				
				// calculate values
				var x_offset = 30;
				var y_offset = -7;
				var x_dir_base = 50;
				var x_pos = RandomX(0, x_offset);
				var x_dir = Target->GetXDir() / 2;
				var y_dir = PV_Random(-9, 5);
				var lifetime = PV_Random(15, 36);
				
				var x_dir_neg = (-x_dir_base + x_pos);
				var x_dir_pos = (+x_dir_base - x_pos);
				
				// the nearer the dust to the center, the faster it is blown aside 
				CreateParticle("BoostDust", x - x_pos, y + y_offset, PV_Random(x_dir + x_dir_neg / 2, x_dir + x_dir_neg), y_dir,lifetime, particles);
				CreateParticle("BoostDust", x + x_pos, y + y_offset, PV_Random(x_dir + x_dir_pos / 2, x_dir + x_dir_pos), y_dir,lifetime, particles);
			}
		}
		
		// burn the landscape?
		/*
		if (distance < burn_distance)
		{
			var landscape_x, landscape_y;
			var burn = 70 - distance;
			var color = RGB(burn, burn, burn);

			for (var amount = 0; amount < 5; ++amount)
			{
				landscape_x = RandomX(-20, 20);
				if (Random(2)) landscape_x *= 2;
				landscape_x += Target->GetX();
				
				landscape_y = 0;
				for (landscape_y = 0; !GBackSolid(landscape_x, Target->GetY() + landscape_y); ++landscape_y);
				landscape_y += RandomX(-2, 2) + burn / 3;
				landscape_y += Target->GetY();
				if (GBackSolid(landscape_x, landscape_y))
				{
					SetLandscapePixel(landscape_x, landscape_y, color);
				}
			}
		}
		*/
	},
};


/* -- Misc -- */

private func PlaySoundJetUpdate()
{
	var play;
	
	if (capsule.thrust_vertical || capsule.thrust_horizontal)
	{
		play = +1;
	}
	else
	{
		play = -1;
	}
	
	Sound("Jetbelt", {loop_count = play});
	
}


private func ContactBottom()
{ 
	if (capsule.is_landing)
	{
		Log("[%d] Capsule has landed", FrameCounter());
		capsule.is_landing = false;
		if (capsule.thrust_vertical)
		{
			if (capsule.automatic)
			{
				StopThruster();
			}
	
			ResetThrust();
	
			if (capsule.port && GetIndexOf(FindObjects(Find_AtPoint()), capsule.port) >= 0) // TODO: test port landing
			{
				capsule.port_vertex = GetVertexNum();
				AddVertex();
				SetVertex(capsule.port_vertex, 2, CNAT_NoCollision, 1);
				AddVertex();
				SetVertex(capsule.port_vertex, 0, capsule.port->GetVertex(0, 0) + capsule.port->GetX() - GetX(), 2);
				SetVertex(capsule.port_vertex, 1, capsule.port->GetVertex(0, 1) + capsule.port->GetY( )- GetY(), 2);
				SetAction("PortLand", capsule.port);
				SetActionData(256 * capsule.port_vertexd);
				//FIXME: Do that less hacky...
			}
			if (capsule.port)
			{ 
				if (ObjectCount(Find_Container(this), Find_OCF(OCF_CrewMember))) // do this only if there is a port - crew members dying because you fail to construct a base is not cool
				{
					ScheduleCall(this, this.EjectCrew, 30, nil, GetX(), GetY());
				}
				ScheduleCall(capsule.port, capsule.port.PortWait, 50);
			}
		}
	}
	return true;
}


// Ejects the contained clonks if the given position matches.
private func EjectCrew(int x, int y)
{
	if (capsule.thrust_vertical) return false;

	var position_matches = GetX() == x && GetY() == y;
	var no_movement = GetXDir() == 0 && GetYDir() == 0;
	if (!IsFlying() && position_matches && no_movement)
	{
		for (var crew in FindObjects(Find_Container(this), Find_OCF(OCF_CrewMember)))
		{
			crew->Exit();
		}
		return true;
	}
	else
	{
		ScheduleCall(this, this.EjectCrew, 10, nil, GetX(), GetY());
		return false;
	}
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

public func ControlUseCancel(object clonk, int x, int y)
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

public func ContainedUseCancel(object clonk, int x, int y)
{
	ResetThrust();
	return true;
}


private func SetThrust(int x, int y)
{
	capsule.is_landing = false; // user control removes autopilot

	var per_mille = 1000;
	var max = 300; // this many pixels will count as max acceleration
	var max_thrust_angle = 60 * CAPSULE_Precision; 
	var acceleration_per_mille = BoundBy(Distance(x, y) * per_mille / max, 1, per_mille);
	var angle = Angle(0, 0, x, y, CAPSULE_Precision); //Normalize(Angle(0, 0, x, y, CAPSULE_Precision), -180 * CAPSULE_Precision, CAPSULE_Precision);
	var rotation = GetR() * CAPSULE_Precision;
	angle = Normalize(angle - rotation, -180 * CAPSULE_Precision, CAPSULE_Precision);

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
	PlaySoundJetUpdate();
}



/* -- Interaction -- */
// TODO - this does not work, because the interaction menu allows interactions only if you are outside of an object 
public func IsFlying() { return !GetContact(-1) || capsule.thrust_vertical || capsule.thrust_horizontal;}


public func IsInteractable(object clonk)
{
	if (Hostile(clonk->GetOwner(), GetOwner()))
		return false;

	return HasLandingInteraction(clonk) || HasLeaveInteraction(clonk);
}


public func HasLandingInteraction(object clonk)
{
	return this == clonk->Contained() && IsFlying() && !GetEffect("FxLandingCountdown", this);
}


public func HasLeaveInteraction(object clonk)
{
	return this == clonk->Contained() && !IsFlying();
}


public func GetInteractionMetaInfo(object clonk)
{
	if (HasLandingInteraction(clonk))
	{
		return { Description = "$StartLanding$", IconName = nil, IconID = Icon_Exit, Selected = false };
	}
	else if (HasLeaveInteraction(clonk))
	{
		return { Description = "$Exit$", IconName = nil, IconID = Icon_Exit, Selected = false };
	}
}


public func Interact(object clonk)
{
	if (HasLandingInteraction(clonk))
	{
		ResetThrust();
		var time_to_land = SetLandingDestination();
		RemoveEffect("FxLandingCountdown", this);
		CreateEffect(FxLandingCountdown, 1, 1, time_to_land);
		return true;
	}
	else if (HasLeaveInteraction(clonk))
	{
		EjectCrew(GetX(), GetY());
	}
	return false;
}


local FxLandingCountdown = new Effect
{
	Construction = func (int time_freefall)
	{
		this.time_freefall = time_freefall;
	},

	Timer = func (int time)
	{
		var time_remaining = this.time_freefall - time;
		if (Target.capsule.thrust_vertical || Target.capsule.thrust_horizontal || !Target->IsFlying() || time_remaining < 0) return FX_Execute_Kill;


		var frames = 36;
		var remainder = time_remaining % frames;
		var seconds = (time_remaining - remainder) / frames;

		var millis = BoundBy(remainder * 1000 / frames, 0, 999);
		Target->PlayerMessage(Target->GetOwner(), "$CountdownLanding$", seconds, remainder);
		return FX_OK;
	},
};


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



public func WindEffect() { return 100;}
*/
