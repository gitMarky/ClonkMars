#appendto Clonk


// Handle rolling when hitting the landscape
public func HandleRollOnHit(int iXSpeed, int iYSpeed)
{
	if (this->IsWalking() && iYSpeed > 450)
	{
		// roll :D
		var x_movement = ComDir2XY(GetComDir())[0];
		var looking_right = GetDir() == DIR_Right;
		if ((x_movement > 0 && looking_right) || (x_movement < 0 && !looking_right))
		{
			this->DoRoll(true); // the code is entirely the same, except that DoRoll is called with a parameter. Might add to the original code
		}
		else // Force kneel-down when hitting the ground at high velocity.
			this->DoKneel(true);
	}
}


private func DoRoll(bool forced)
{
	// the suit slows you down
	if (this->IsWearingSpaceSuit())
	{
		// kneel, if you are forced to roll by falling
		if (forced)
		{
			this->DoKneel(true);
		}
		// do nothing if you initiated rolling while running
	}
	else
	{
		// roll normally without a suit
		_inherited();
	}
}
