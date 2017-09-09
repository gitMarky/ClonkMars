/**
	Power Producer
	Handles some aspects of power producing structures, this library should be 
	included by all power producing structures. Certain functions should be 
	overloaded and others can be used to implement the production of power
	in a uniform way consistent with the network, see text below.
	 

	Important notes when including this library:
	 * The object including this library should return _inherited(...) in the
	   Construction and Destruction callback if overloaded.
	
	@author Zapper, Maikel, Marky
*/


// This object is a power producer.
public func IsPowerProducer() { return true; }


/* -- Interface -- */


/**
 * Gets the amount of power that this producer creates.
 *
 * Preferrably, do not override this function.
 *
 * @return the created amount, usually > 0;
 */
private func GetPowerProduction()
{
	return lib_power_system.producer.power_production;
}


/**
 * Sets the amount of power that this producer creates.
 *
 * Calculations for power production should be done
 * in a separate function in an object, then set with
 * this function.
 *
 * @par amount the amount, must be >= 0;
 */
private func SetPowerProduction(int amount)
{
	if (amount < 0)
	{
		FatalError(Format("Power production must be >= 0, was %d", amount));
	}

	lib_power_system.producer.power_production = amount;
}


/**
 * Producer priority: the willingsness of a producer to deliver energy.
 * This is high for steady producers like the wind generator and low
 * for producers like the steam engine. 
 *
 * @return the priority. Typical return values are:
 *	       - Windmill:    100
 *         - Compensator:  50
 *         - Steam engine:  0	
 */
private func GetProducerPriority()
{
	return lib_power_system.producer.priority;
}


/**
 * Sets the producer priority: the willingsness of a producer to deliver energy.
 * This is high for steady producers like the wind generator and low
 * for producers like the steam engine. 
 *
 * @par priority typical values are:
 *	       - Windmill:    100
 *         - Compensator:  50
 *         - Steam engine:  0	
 */
private func SetProducerPriority(int priority)
{
	if (priority < 0)
	{
		FatalError(Format("Producer priority must be >= 0, was %d", priority));
	}

	lib_power_system.producer.priority = priority;
}


/* -- Callbacks -- */

/**
 * Find out whether this producer is producing power.
 * 
 * The power system asks this before issuing the callbacks
 * OnPowerProductionStart() and OnPowerProductionStop()
 */
public func IsPowerProductionActive()
{
	return lib_power_system.producer.is_producing_power;
}


/**
 * Find out whether this consumer has enough power.
 */
public func SetPowerProductionActive(bool status)
{
	return lib_power_system.producer.is_producing_power = status;
}


/**
 * Callback by the power network. Overload this function and start the production 
 * of power in this structure for the requested amount if possible. 
 */
private func OnPowerProductionStart(int amount) 
{ 
	// A return value of false indicates to the network that power production is not possible.
	return false;
}


/**
 * Callback by the power network. Overload this function and stop the production
 * of power in this structure if possible.
 */
private func OnPowerProductionStop(int amount)
{
	// A return value of false indicates to the network that stopping the current production
	// was not possible, this should in principle never happen. However, if so the network
	// must assume this producer is still actively delivering power.
	return true;
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
	if (lib_power_system.producer == nil)
	{
		lib_power_system.producer = {};
	}
	
	// Default values
	lib_power_system.producer.power_production = 0;		// Works without power by default
	lib_power_system.producer.priority = 0;				// No priority
	lib_power_system.producer.is_producing_power = 0; 	// Is not producing power right now
	return _inherited(...);
}


/**
 * Destruction callback by the engine: let power network know this object is not
 * a producer anymore, it must always be unregistered from the power network.
 */
private func Destruction()
{
	// TODO: UnregisterPowerProduction();
	return _inherited(...);
}


/**
 * When ownership has changed, the consumer may have moved out of or into a new network.
 */
private func OnOwnerChanged(int new_owner, int old_owner)
{
	Library_Power->TransferPowerLink(this);
	return _inherited(new_owner, old_owner, ...);
}
