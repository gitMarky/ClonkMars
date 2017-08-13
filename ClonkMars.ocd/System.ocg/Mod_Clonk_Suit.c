#appendto Clonk


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
	
	//Adventurer
	if(skin == 0)
	{
		skin_name = "Clonk";
		gender = 0;
	}

	//Steampunk
	if(skin == 1)
	{
		skin_name = "Steampunk";
		gender = 1;
	}

	//Alchemist
	if(skin == 2)
	{
		skin_name = "Alchemist";
		gender = 0;
	}
	
	//Farmer
	if(skin == 3)
	{
		skin_name = "Farmer";
		gender = 1;
	}

	RemoveBackpack(); //add a backpack
	
	SetGraphics("Suit", SpaceSuit);

	//refreshes animation (whatever that means?)
	// Go back to original action afterwards and hope
	// that noone calls SetSkin during more compex activities
	var prev_action = GetAction();
	SetAction("Jump");
	SetAction(prev_action);

	return skin;
}
