

// Gives the player plans according to basic knowledge.
global func GivePlayerBasicKnowledge(int player)
{
	var knowledge = [
		// basic structures for a settlement and production
		Structure_Base,
		// basic tools
		Shovel,
		// basic resources.
		Metal, Plastic
	];
	for (var plan in knowledge)
	{
		SetPlrKnowledge(player, plan);
	}
}
