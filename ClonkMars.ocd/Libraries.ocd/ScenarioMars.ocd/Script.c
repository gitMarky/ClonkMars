
/* -- Engine callbacks -- */

public func Initialize()
{
	_inherited(...);
    Sky_Mars();
    Scenario_Gravity();
    Scenario_Temperature();
}


public func InitializePlayer(int player)
{
	CreateStartingMaterial(player);
	CreateStartingEquipment(player);
	CreateStartingCapsule(player);
	GivePlayerInitialKnowledge(player);
}


/* -- Parameters & useful functions -- */

func Scenario_Gravity()
{
    SetGravity(8); // 37% of default value 20, rounded up
}


func Scenario_Temperature()
{
	Temperature->CreateGrid(10);
}


func CreateStartingCapsule(int player)
{
	var capsule = CreateSupplyCapsule(player);
	capsule->SetLandingDestination();
	for (var material in FindObjects(Find_Owner(player), Find_NoContainer(), Find_Or(Find_Category(C4D_Object), Find_OCF(OCF_CrewMember))))
	{
		material->Enter(capsule);
	}
}


func CreateSupplyCapsule(int player)
{
	return CreateObject(Capsule, RandomX(100, LandscapeWidth() - 100), 70, player);
}


func CreateStartingMaterial(int player)
{
	CreateObjects({ConKit = 2, Metal = 2, Plastic = 2}, nil, nil, player);
}

func CreateStartingEquipment(int player)
{
	var crew_members = FindObjects(Find_Owner(player), Find_OCF(OCF_CrewMember));
	for (var i = 0; i < GetLength(crew_members); ++i)
	{
		var crew = crew_members[i];
		CreateStartingEquipmentForCrewMember(player, crew, i);
	}
}

func CreateStartingEquipmentForCrewMember(int player, object crew, int crew_index)
{
	var suit = crew->CreateContents(SpaceSuit);
	suit->PutOn(crew);
	crew->CreateContents(Shovel);
}
