
/* -- Engine callbacks -- */

public func Initialize()
{
	_inherited(...);
    Sky_Mars();
    Scenario_Gravity();
}


public func InitializePlayer(int player)
{
	CreateStartingEquipment(player);
	CreateStartingCapsule(player);
	GivePlayerBasicKnowledge(player);
}


/* -- Parameters & useful functions -- */

private func Scenario_Gravity()
{
    SetGravity(8); // 37% of default value 20, rounded up
}


private func CreateStartingCapsule(int player)
{
	var capsule = CreateSupplyCapsule(player);
	capsule->SetLandingDestination();
	for(var material in FindObjects(Find_Owner(player), Find_Or(Find_Category(C4D_Object), Find_OCF(OCF_CrewMember))))
	{
		material->Enter(capsule);
	}
}


private func CreateSupplyCapsule(int player)
{
	return CreateObject(Capsule, RandomX(100, LandscapeWidth() - 100), 70, player);
}


private func CreateStartingEquipment(int player)
{
	CreateObjects({ConKit = 2, Metal = 2, Plastic = 2}, nil, nil, player);
}
