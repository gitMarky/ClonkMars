
// This object is a power storage.
public func IsPowerStorage() { return true; }


/* -- Interface -- */


/**
 * Gets the amount of power that this storage can
 * transfer between itself and the network.
 *
 * Preferrably, do not override this function.
 *
 * @return the input/output amount, usually > 0;
 */
private func GetStoragePower()
{
	return lib_power_system.storage.max_rate;
}


/**
 * Sets the amount of power that this storage can
 * transfer between itself and the network.
 *
 * @par amount the input/output amount, usually > 0;
 */
private func SetStoragePower(int amount)
{
	lib_power_system.storage.max_rate = amount;
}


/**
 * Set the amount of stored power in this storage.
 * This is limited to be a number between zero and what GetStorageCapacity() returns;
 */
private func SetStoredPower(int to_power)
{
	var old_power = GetStoredPower();
	lib_power_system.storage.stored_power = BoundBy(to_power, 0, GetStorageCapacity());
	var change = GetStoredPower() - old_power;
	// Callback to this object that the power has changed.
	if (change != 0)
	{
		OnStoredPowerChange();
	}
	return change;
}


/**
 * Returns the amount of stored power in the storage.
 */
private func GetStoredPower()
{ 
	return lib_power_system.storage.stored_power;
}


/**
 * Storage capacity: the amount of energy a power storage can store.
 * The amount is expressed in power frames.
 */
private func GetStorageCapacity() { return lib_power_system.storage.capacity; }


/**
 * Storage capacity: the amount of energy a power storage can store.
 * The amount is expressed in power frames.
 */
private func SetStorageCapacity(int amount)
{
	return lib_power_system.storage.capacity = amount;
}


private func SetStorageInput(int amount)
{
	var remaining = GetStorageCapacity() - GetStoredPower();
	var rate = Min(remaining, BoundBy(amount, 0, GetStoragePower()));
	
	lib_power_system.storage.current_rate = rate;
	
	var fx = GetEffect("FxStorageCharge", this);
	if (rate > 0)
	{
		if (!fx) CreateEffect(FxStorageCharge, 1, 36);
	}
	else
	{
		if (fx) RemoveEffect(nil, nil, fx);
	}
	
	return rate;
}


/**
 * This many power is added to the storage each frame.
 */
private func GetStorageInput()
{
	return lib_power_system.storage.current_rate;
}


/* -- Backward compatibility -- */

/**
 * Call this function in the power producing structure to indicate to the network
 * that this structure is available and able to produce the specified amount of power.
 */
private func RegisterPowerStorage()
{
	GetPowerSystem()->RegisterPowerStorage(this);
}


/**
 * Call this function in the power producing structure to indicate to the network
 * that this structure is not able to produce any power any more.
 */
private func UnregisterPowerStorage()
{
	GetPowerSystem()->UnregisterPowerStorage(this);
}


/* -- Callbacks -- */


/**
 * Callback by the power storage when the amount of stored power has changed.
 */
private func OnStoredPowerChange()
{
	return _inherited(...);
}


/**
 * Callback by the power storage it starts storing power.
 */
private func OnStorageStart()
{
	GetPowerSystem()->DebugInfo("Start charging frame %d, %s (%d)", FrameCounter(), GetName(), ObjectNumber());
	return _inherited(...);
}


/**
 * Callback by the power storage it stops storing power.
 */
private func OnStorageStop()
{
	GetPowerSystem()->DebugInfo("Stop charging frame %d, %s (%d)", FrameCounter(), GetName(), ObjectNumber());
	return _inherited(...);
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
	if (lib_power_system.storage == nil)
	{
		lib_power_system.storage = {};
	}
	
	// Default values
	lib_power_system.storage.max_rate = 0;		// Do not store by default
	lib_power_system.storage.current_rate = 0;	// Do not store by default
	lib_power_system.storage.stored_power = 0; 	// Empty by default
	lib_power_system.storage.capacity = 0;		// Cannot store anything by default
	return _inherited(...);
}


/**
 * Destruction callback by the engine: let power network know this object is not
 * a storage anymore, it must always be unregistered from the power network.
 */
private func Destruction()
{
	UnregisterPowerStorage();
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


local FxStorageCharge = new Effect
{
	Construction = func()
	{
		if (Target) Target->OnStorageStart();
	},

	Timer = func ()
	{
		var expected_change = Target->GetStorageInput() * this.Interval;
		var actual_change = 0;
		if (expected_change > 0)
		{
			actual_change = Target->SetStoredPower(Target->GetStoredPower() + expected_change);
		}
		
		if (expected_change == 0 || actual_change < expected_change)
		{
			return FX_Execute_Kill;
		}
	},
	
	Destruction = func ()
	{
		if (Target) Target->OnStorageStop();
	},
};

