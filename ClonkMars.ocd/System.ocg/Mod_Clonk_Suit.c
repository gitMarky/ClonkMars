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
