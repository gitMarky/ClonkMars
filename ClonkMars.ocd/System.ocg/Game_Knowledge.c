

// Gives the player plans at the scenario start.
global func GivePlayerInitialKnowledge(int player)
{
	GivePlayerKnowledge(player,
	[
		// basic structures for a settlement and production
		Structure_Base,
		// basic tools
		Shovel,
		// basic resources.
		Metal, Plastic
	]);
}


// Gives the player plans once he has built a base.
global func GivePlayerBasicKnowledge(int player)
{
	GivePlayerKnowledge(player,
	[
		// basic structures for a settlement and production
		Structure_Base,
		Structure_SolarPanel,
		Structure_Accumulator,
		Structure_MaterialUnit,
		Structure_OilRig,
		Structure_OilTank,
		// basic tools
		Shovel,
		// basic resources.
		Metal, Plastic
	]);
}



global func GivePlayerKnowledge(int player, array plans)
{
	for (var plan in plans)
	{
		SetPlrKnowledge(player, plan);
	}
}
