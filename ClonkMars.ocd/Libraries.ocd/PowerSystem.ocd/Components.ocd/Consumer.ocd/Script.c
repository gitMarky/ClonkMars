/**
	Power Consumer
	Handles some aspects of power consuming structures, this library should be
	included by	all power consuming structures. Certain functions should be 
	overloaded and others can be used to implement the consumption of power in
	a uniform way consistent with the network, see text below.

	
	Important notes when including this library:
	 * The object including this library should return _inherited(...) in the
	   Construction and Destruction callback if overloaded.
	
	@author Zapper, Maikel, Marky
*/


// This object is a power consumer.
public func IsPowerConsumer() { return true; }


/* -- Interface -- */

/**
 * Gets the amount of power that this consumer needs.
 *
 * Preferrably, do not override this function.
 *
 * @return the needed amount, usually > 0; if the amount is 0 the
 *         object counts as treated as if it has enough power.
 */
private func GetPowerConsumption()
{
	if (lib_power_system.consumer.ignore_power_need)
	{
		return 0;
	}
	else
	{
		return lib_power_system.consumer.power_need;
	}
}


/**
 * Sets the amount of power that this consumer needs.
 *
 * Calculations for power consumption should be done
 * in a separate function in an object, then set with
 * this function.
 *
 * @par amount the amount, must be >= 0;
 */
private func SetPowerConsumption(int amount)
{
	// Safety check
	if (amount < 0)
	{
		FatalError(Format("Power consumption must be >= 0, was %d", amount));
	}
	
	// Callback to visualization
	if (HasEnoughPower())
	{
		this->~VisualizePowerChange(GetPowerConsumption(), amount, false);
	}
	
	// Update the value
	lib_power_system.consumer.power_need = amount;
}


/**
 * Consumer priority: the need of a consumer to have power.
 * This can be used by the structures to prioritize certain consumption,
 * for example letting an elevator be dominant over a pump.
 *
 * @return the priority. Typical return values are:
 *	       - Pump:      25
 *         - Workshop:  50
 *         - Elevator: 100	
 */
private func GetConsumerPriority()
{
	return lib_power_system.consumer.priority;
}


/**
 * Sets the consumer priority: the need of a consumer to have power.
 * This can be used by the structures to prioritize certain consumption,
 * for example letting an elevator be dominant over a pump.
 *
 * @par priority typical values are:
 *	       - Pump:      25
 *         - Workshop:  50
 *         - Elevator: 100	
 */
private func SetConsumerPriority(int priority)
{
	if (priority < 0)
	{
		FatalError(Format("Consumer priority must be >= 0, was %d", priority));
	}

	lib_power_system.consumer.priority = priority;
}


/* -- Callbacks -- */

/**
 * Find out whether this consumer has enough power.
 * 
 * The power system asks this before issuing the callbacks
 * OnEnoughPower() and OnNotEnoughPower().
 */
public func HasEnoughPower()
{
	return lib_power_system.consumer.has_enough_power;
}


/**
 * Callback by the power network. Overload this function to start the consumers
 * functionality, since enough power is available. return inherited(amount, ...)
 * to remove the no-power symbol.
 * 
 * It is not allowed to (un)register a power request in this callback.
 */
private func OnEnoughPower(int amount)
{
	// Remove the no-power symbol.
	RemoveStatusSymbol(Library_PowerConsumer);
	
	// Callback to visualization
	this->~VisualizePowerChange(0, GetPowerConsumption(), false);
	
	// Update the status
	lib_power_system.consumer.has_enough_power = true;
	
	// Let the parent class handle things
	_inherited(amount, ...);
}


/**
 * Callback by the power network. Overload this function to stop the consumers
 * functionality, since not enough power is available. return inherited(amount, ...)
 * to add the no-power symbol.
 * 
 * It is not allowed to (un)register a power request in this callback.
 */
private func OnNotEnoughPower(int amount)
{
	// Show the no-power symbol.
	ShowStatusSymbol(Library_PowerConsumer);
	
	// Callback to visualization
	this->~VisualizePowerChange(GetPowerConsumption(), 0, true);

	// Update the status
	lib_power_system.consumer.has_enough_power = false;
	
	// Let the parent class handle things
	_inherited(amount, ...);
}


/* -- Backwards compatibility -- */

/**
 * Call this function in the power consuming structure to indicate to the network
 * a request for power of the specified amount.
 */
private func RegisterPowerRequest(int amount)
{
	SetPowerConsumption(amount);
	GetPowerSystem()->RegisterPowerConsumer(this);
}


/**
 * Call this function in the power consuming structure to indicate to the network
 * a the end of a power request.
 */
private func UnregisterPowerRequest()
{
	GetPowerSystem()->UnregisterPowerConsumer(this);
	// Also ensure that the no-power symbol is not shown any more.
	RemoveStatusSymbol(Library_PowerConsumer);
}


/* -- Library Code -- */


/**
 * All power related local variables are stored in a single proplist.
 * This reduces the chances of clashing local variables. 
 *
 * See Construction for which variables are being used.
 */
local lib_power_system;


/**
 * Construction callback by the engine: check whether the no power need rule is active.
 */
private func Construction()
{
	// Initialize the single proplist for the power consumer library.
	if (lib_power_system == nil)
	{
		lib_power_system = {};
	}
	if (lib_power_system.consumer == nil)
	{
		lib_power_system.consumer = {};
	}

	// Power is not needed when the no power need rule is active.
	lib_power_system.consumer.ignore_power_need = FindObject(Find_ID(Rule_NoPowerNeed));
	
	// Default values
	lib_power_system.consumer.power_need = 0;			// Works without power by default
	lib_power_system.consumer.priority = 0;				// No priority
	lib_power_system.consumer.has_enough_power = true;	// Compatibility with producer library: Assume that it has power, then the update will switch it off
	return _inherited(...);
}


/**
 * Destruction callback by the engine: let power network know this object is not
 * a consumer anymore, it must always be unregistered from the power network.
 */
private func Destruction()
{
	// Only unregister if this object actually is a consumer.
	if (IsPowerConsumer())
	{
		UnregisterPowerRequest();
	}
	return _inherited(...);
}


/**
 * When ownership has changed, the consumer may have moved out of or into a new network.
 */
private func OnOwnerChanged(int new_owner, int old_owner)
{
	GetPowerSystem()->TransferPowerLink(this);
	return _inherited(new_owner, old_owner, ...);
}


/**
 * By calling this function you can make this consumer ignore the power need.
 *
 * This is  used by the power need rule and can be used by scripters
 * to temporarily turn off the need for power in a certain consumer.
 */
public func SetNoPowerNeed(bool ignore_power_need)
{
	lib_power_system.consumer.ignore_power_need = ignore_power_need;
}
