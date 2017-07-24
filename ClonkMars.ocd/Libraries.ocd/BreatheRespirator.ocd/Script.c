/**
	Library_BreatheRespirator

	Allows the clonk to breathe through a respirator, if he has one.

	We don't want no perpetuum mobile - the respirator just adds to the Clonk's
	breath, so do not implement "CanBreathe" here.
*/


public func Construction()
{
	_inherited(...);
}


public func TakeBreath(int max_supply)
{
	// take as much as you can from the elsewhere first, because hopefully it is free for you
	var take_breath = _inherited(max_supply, ...);

	// get the rest from the respirator
	var rest = Max(0, max_supply - take_breath);
	var respirator = this->~GetRespirator();
	if (respirator)
	{
		take_breath += Min(respirator->GetOxygen(), rest);
	}
	
	return BoundBy(take_breath, 0, max_supply);
}


// Override, so that we count the respirator, too
public func GetBreath()
{
	var breath = _inherited(...);
	var respirator = this->~GetRespirator();
	if (respirator)
	{
		breath += respirator->GetOxygen();
	}
	return breath;
}


// Override, so that we count the respirator, too
public func GetMaxBreath()
{
	var breath = _inherited(...);
	if (this && GetType(this) == C4V_C4Object) // look for respirators only in object context
	{
		var respirator = this->~GetRespirator();
		if (respirator)
		{
			breath += respirator->GetMaxOxygen();
		}
	}
	return breath;
}


// Override, so that we use the respirator, too
public func DoBreath(int change)
{
	var current = GetBreath();
	// take as much as you can from the elsewhere first, because hopefully it is free for you
	var do_breath = _inherited(change);

	// get the rest from the respirator
	var rest = change - do_breath;
	var respirator = this->~GetRespirator();
	if (respirator)
	{
		respirator->DoOxygen(rest);
	}

	// return the change
	return GetBreath() - current;
}
