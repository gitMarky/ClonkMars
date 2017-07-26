/**
	Library_BreatheRespirator

	Allows the clonk to breathe through a supplier, if he has one.

	We don't want no perpetuum mobile - the supplier just adds to the Clonk's
	breath, so do not implement "CanBreathe" here.
*/


public func Construction()
{
	_inherited(...);
}


public func CanBreathe()
{
	if (_inherited(...))
	{
		return true;
	}
	else
	{
		return !!this->~GetOxygenSupplier();
	}
}


public func TakeBreath(int max_supply)
{
	// take as much as you can from the elsewhere first, because hopefully it is free for you
	var take_breath = _inherited(max_supply, ...);

	// get the rest from the supplier
	var rest = Max(0, max_supply - take_breath);
	var supplier = this->~GetOxygenSupplier();
	if (supplier)
	{
		var take_oxygen = Abs(supplier->DoOxygen(-rest));
		take_breath += take_oxygen;
	}
	
	return BoundBy(take_breath, 0, max_supply);
}
