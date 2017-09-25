local library_construction_animation;

/* -- Engine callbacks -- */

private func Construction()
{
	library_construction_animation = {};

	return _inherited(...);
}


private func Destruction()
{
	if (library_construction_animation.overlay)
	{
		library_construction_animation.overlay->RemoveObject();
	}
}


private func Initialize()
{
	// Create objects normally if placed via editor - delay the initialization here, so that it can be removed by the construction site
	CreateEffect(FxInitializeStructure, 1, 2);
	return _inherited(...);
}


/* -- Internals -- */

private func OnStartConstructing()
{
	// Remove forced initialization
	RemoveEffect(FxInitializeStructure.Name, this);
	// Initialize animations
	library_construction_animation.overlay = CreateObject(Dummy, 0, 0, NO_OWNER);
	library_construction_animation.overlay->SetGraphics("Construction", GetID());
	AttachMesh(library_construction_animation.overlay, "main", "main", nil, AM_MatchSkeleton);
}


private func OnFinishConstructing()
{
	SetAction("construction_finish");
}


private func InitializeStructure()
{
	if (library_construction_animation.overlay)
	{
		library_construction_animation.overlay->RemoveObject();
	}
}


local FxInitializeStructure = new Effect
{
	Timer = func ()
	{
		Target->InitializeStructure();
		return FX_Execute_Kill;
	}
};


/* -- Actions -- */

local ActMap = {
construction_progress = {
	Prototype = Action,
	Procedure = DFA_NONE,
	Name = "construction_progress",
	Animation = "construction_progress",
	Length = 1001,
	Delay = 0,
},

construction_finish = {
	Prototype = Action,
	Procedure = DFA_NONE,
	Name = "construction_finish",
	Animation = "construction_finish",
	Length = 40,
	Delay = 1,
	EndCall = "InitializeStructure",
	NextAction = "Idle",
},
};
