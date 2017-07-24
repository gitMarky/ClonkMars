/**
	Library_Respirator
*/

local library_respirator;


private func Construction()
{
	_inherited(...);
	library_respirator = {
		Oxygen = GetMaxOxygen(),
	};
}


public func GetOxygen()
{
	return library_respirator.Oxygen;
}


public func GetMaxOxygen()
{
	return this.MaxOxygen;
}


public func SetOxygen(int amount)
{
	library_respirator.Oxygen = BoundBy(amount, 0, this.MaxOxygen);
}


public func DoOxygen(int change)
{
	var previous = GetOxygen();
	SetOxygen(previous + change);
	return GetOxygen() - previous;
}


public func IsRespirator(){ return true; }
