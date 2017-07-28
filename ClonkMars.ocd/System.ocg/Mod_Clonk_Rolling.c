#appendto Clonk


private func DoRoll(bool is_falling)
{
	// the suit slows you down
	if (this->IsWearingSpaceSuit())
	{
		// kneel, if you are forced to roll by falling
		if (is_falling)
		{
			this->DoKneel(true);
		}
		// do nothing if you initiated rolling while running
	}
	else
	{
		// roll normally without a suit
		_inherited(is_falling);
	}
}
