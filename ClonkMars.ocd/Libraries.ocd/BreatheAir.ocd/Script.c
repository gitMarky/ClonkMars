/**
	Library_BreatheAir


*/

static const BREATHE_MinOxygenInAtmosphere = 100;

public func Construction()
{
	_inherited(...);
	if (this.library_breath)
	{
		this.library_breath.BreathRefreshTime = 36; // it takes 36 frames to refresh breath fully
	}
	else
	{
		FatalError("Include Library_Breath, or change the #include order so that Library_Breath comes before this");
	}
}


public func CanBreathe()
{
	if (_inherited(...))
	{
		return true;
	}
	else if (GetOxygenInAtmosphere() > BREATHE_MinOxygenInAtmosphere) // atmosphere needs 10% oxygen
	{
		var breathe_at = this->BreatheAt();
		return !GBackSemiSolid(breathe_at.X, breathe_at.Y);
	}
	else
	{
		return false;
	}
}


public func TakeBreath(int max_supply)
{
	// take as much as you can from the atmosphere first, because it is free for you
	var refresh_rate = Max(1, this->GetID()->GetMaxBreath() / this.library_breath.BreathRefreshTime);
	var take_breath = GetOxygenInAtmosphere() * refresh_rate / 1000;

	// try to aim for the maximum that you can take in, by supplementing the atmosphere value
	// with additionl sources
	var rest = Max(0, max_supply - take_breath);
	var additional = _inherited(rest, ...);

	return BoundBy(take_breath + additional, 0, max_supply);
}

