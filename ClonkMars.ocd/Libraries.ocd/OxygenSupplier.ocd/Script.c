/**
	Library_OxygenSupplier
	
	Library for objects that can supply oxygen (to a respirator).
*/


static const SUPPLY_OXYGEN_Infinite = -1;

/* -- Properties -- */

local library_supply_oxygen;
local MaxOxygen = SUPPLY_OXYGEN_Infinite;


public func IsOxygenSupplier(){ return true; }

/* -- Engine callbacks -- */

private func Construction()
{
	_inherited(...);
	library_supply_oxygen = {
		Oxygen = GetMaxOxygen(),
		RefillRate = 1, // give back 1 per frame
	};
}

/* -- Functionality -- */

public func GetOxygen()
{
	return library_supply_oxygen.Oxygen;
}


public func GetMaxOxygen()
{
	return this.MaxOxygen;
}


public func SetOxygen(int amount)
{
	if (this.MaxOxygen != SUPPLY_OXYGEN_Infinite)
	{
		library_supply_oxygen.Oxygen = BoundBy(amount, 0, this.MaxOxygen);
	}
}


public func HasOxygen(int amount) // unused now
{
	if (this.MaxOxygen == SUPPLY_OXYGEN_Infinite)
	{
		return amount;
	}
	else
	{
		return Min(amount, GetOxygen());
	}
}


public func DoOxygen(int change)
{
	if (this.MaxOxygen == SUPPLY_OXYGEN_Infinite)
	{
		return change;
	}
	else
	{
		var previous = GetOxygen();
		SetOxygen(previous + change);
		return GetOxygen() - previous;
	}
}


public func GetOxygenRefillRate()
{
	return library_supply_oxygen.RefillRate;
}


public func SetOxygenRefillRate(int amount)
{
	library_supply_oxygen.RefillRate = amount;
}
