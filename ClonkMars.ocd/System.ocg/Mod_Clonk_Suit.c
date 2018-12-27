#appendto Clonk

local skin_mesh_helper;

public func IsWearingSpaceSuit()
{
	var suits = FindObjects(Find_ID(SpaceSuit), Find_Container(this));
	for (var suit in suits)
	{
		if (suit->~IsWorn())
		{
			return suit;
		}
	}
}


public func SetSkin(int new_skin)
{
	// Remember skin
	skin = new_skin;

	// Adventurer
	if(skin == 0)
	{
		skin_name = "Clonk";
		gender = 0;
	}

	// Steampunk
	if(skin == 1)
	{
		skin_name = "Steampunk";
		gender = 1;
	}

	// Alchemist
	if(skin == 2)
	{
		skin_name = "Alchemist";
		gender = 0;
	}

	// Farmer
	if(skin == 3)
	{
		skin_name = "Farmer";
		gender = 1;
	}

	// Does not need a backpack
	RemoveBackpack();

	// Give a space suit (without helmet)
	SetGraphics("Suit", SpaceSuit);

	// Attach a head that is matching the clonk skin
	skin_mesh_helper = skin_mesh_helper ?? CreateObject(ClonkCustomSkins);
	skin_mesh_helper->SetGraphics(Format("Head%s", skin_name));
	skin_mesh_helper.Visibility = VIS_None;
	AttachMesh(skin_mesh_helper, "skeleton_head", "skeleton_head", nil, AM_MatchSkeleton);

	// Refreshes animation after changing the base graphics,
	// because the clonk would be in a scarecrow position otherwise,
	// Go back to original action afterwards and hope
	// that noone calls SetSkin during more compex activities
	var prev_action = GetAction();
	SetAction("Jump");
	SetAction(prev_action);

	return skin;
}


public func Destruction()
{
	if (skin_mesh_helper) skin_mesh_helper->RemoveObject();

	return _inherited(...);
}
