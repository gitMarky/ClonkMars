

/* -- Network object - Interface -- */

/**
 * Adds a power producer to the network.
 */
public func AddPowerProducer(object producer)
{
	if (!IsValueInArray(power_producers, producer))
	{
		PushBack(power_producers, producer);		
		SortProducers(); // This is necessary only when adding an object to the list
		GetPowerSystem()->DebugInfo("POWR - AddPowerProducer(): network = %v, frame = %d, producer = %s, all producers: %v", this, FrameCounter(), LogObject(producer), power_producers);
		SchedulePowerBalanceUpdate(); // Check the power balance of this network, since a change has been made.
	}
}


/**
 * Removes a power producer from the network.
 */
public func RemovePowerProducer(object producer)
{
	if (IsValueInArray(power_producers, producer))
	{
		if (producer->IsPowerProductionActive())
		{
			producer->OnPowerProductionStop();
		}
		RemoveArrayValue(power_producers, producer);		
		GetPowerSystem()->DebugInfo("POWR - RemovePowerProducer(): network = %v, frame = %d, producer = %s, all producers: %v", this, FrameCounter(), LogObject(producer), power_producers);
		SchedulePowerBalanceUpdate(); // Check the power balance of this network, since a change has been made.
	}
}


/**
 * Adds a power consumer to the network.
 */
public func AddPowerConsumer(object consumer)
{
	if (!IsValueInArray(power_consumers, consumer))
	{
		PushBack(power_consumers, consumer);
		SortConsumers(); // This is necessary only when adding an object to the list
		GetPowerSystem()->DebugInfo("POWR - AddPowerConsumer(): network = %v, frame = %d, consumer = %s, all consumers: %v", this, FrameCounter(), LogObject(consumer), power_consumers);
		SchedulePowerBalanceUpdate(); // Check the power balance of this network, since a change has been made.
	}
}


/**
 * Removes a power consumer from the network.
 */
public func RemovePowerConsumer(object consumer)
{
	if (IsValueInArray(power_consumers, consumer))
	{
		RemoveArrayValue(power_consumers, consumer);		
		GetPowerSystem()->DebugInfo("POWR - RemovePowerConsumer(): network = %v, frame = %d, consumer = %s, all consumers: %v", this, FrameCounter(), LogObject(consumer), power_consumers);
		SchedulePowerBalanceUpdate(); // Check the power balance of this network, since a change has been made.
	}
}


/**
 * Adds a power storage to the network.
 */
public func AddPowerStorage(object storage)
{
	if (!IsValueInArray(power_storages, storage))
	{
		PushBack(power_storages, storage);
		GetPowerSystem()->DebugInfo("POWR - AddPowerStorage(): network = %v, frame = %d, storage = %s, all storages: %v", this, FrameCounter(), LogObject(storage), power_storages);
		SchedulePowerBalanceUpdate(); // Check the power balance of this network, since a change has been made.
	}
}


/**
 * Removes a power storage from the network.
 */
public func RemovePowerStorage(object storage)
{
	if (IsValueInArray(power_storages, storage))
	{
		RemoveArrayValue(power_storages, storage);		
		GetPowerSystem()->DebugInfo("POWR - RemovePowerStorage(): network = %v, frame = %d, storage = %s, all storages: %v", this, FrameCounter(), LogObject(storage), power_storages);
		SchedulePowerBalanceUpdate(); // Check the power balance of this network, since a change has been made.
	}
}


/**
 * Returns the total power available in the network: idle + active producers.
 */
public func GetPowerAvailable()
{
	var total = 0;
	for (var producer in power_producers)
	{
		total += producer->GetPowerProduction();
	}
	return total;
}


// Returns the total bare power available in the network: that is the idle
// + active producers which are not power storages.
public func GetBarePowerAvailable()
{
	return 0; // TODO: necessary?
}

/**
 * Returns the total active power available in the network.
 */
public func GetActivePowerAvailable()
{
	var total = 0;
	for (var producer in power_producers)
	{
		if (producer->~IsPowerProductionActive())
		{
			total += producer->GetPowerProduction();
		}
	}
	return total;
}


/**
 * Returns the bare amount of power needed by all consumer requests.
 */
public func GetPowerConsumptionNeed()
{
	var total = 0;
	for (var consumer in power_consumers)
	{
		total += consumer->GetPowerConsumption();
	}
	return total;
}


/**
 * Returns the amount of power the currently active power consumers need.
 */
public func GetPowerConsumption(bool exclude_storages)
{
	var total = 0;
	for (var consumer in power_consumers)
	{
		if (consumer->HasEnoughPower())
		{
			total += consumer->GetPowerConsumption();
		}
	}
	return total;
}


/**
 * Returns the total capacity for storing power of all power storages in the network (in power frames).
 */
public func GetStoredPowerCapacity()
{
	var total = 0;
	for (var storage in power_storages)
	{
		// TODO 	total += consumer->GetPowerConsumption();
	}
	return total;
}


/**
 * Returns the total stored power of all power storages in the network (in power frames).
 */
public func GetStoredPower()
{
	var total = 0;
	for (var storage in power_storages)
	{
		// TODO 	total += consumer->GetPowerConsumption();
	}
	return total;
}


/**
 * Find out whether this is a neutral network, instead of a player owned network.
 */
public func IsNeutral()
{
	return is_neutral;
}


/**
 * Set this to be neutral, or player owned.
 */
private func SetNeutral(bool neutral)
{
	is_neutral = neutral;
}


/* -- Network State -- */

/**
 * Returns whether the network does not control any power nodes.
 */
public func IsEmpty()
{
	return GetLength(power_producers) == 0
		&& GetLength(power_consumers) == 0
		&& GetLength(power_storages) == 0;
}


/**
 * Sort the producers by priority, descending.
 */
private func SortProducers()
{
	SortPowerNodes(power_producers, false);
}


/**
 * Sort the consumers by priority, descending.
 */
private func SortConsumers()
{
	SortPowerNodes(power_consumers, true);
}


/**
 * Sort a list by descending priority.
 *
 * This is done with the help of a proplist with priority information, then filled back into an array.
 * The priority information is used only here, so there is no need to carry a proplist with all information
 * all the time, and iterating over an array is easier that going through the objects in the proplist.
 */
private func SortPowerNodes(array power_nodes, bool for_consumers)
{
	// Sort only if it makes sense
	if (GetLength(power_nodes) > 1)
	{
		// First update the priorities and then sort them according to priority, descending order.
		var array_of_proplists = GetPriorityList(power_nodes, for_consumers);
		SortArrayByProperty(array_of_proplists, "priority", true);
		// Sort the thing back into the array
		SetLength(power_nodes, 0);
		for (var description in array_of_proplists)
		{
			PushBack(power_nodes, description.node);
		}
	}
}


/**
 * Gets the priorities of either a list of consumers or producers,
 * as a list of proplists, so that they can be sorted with
 * array sorting functions.
 */
private func GetPriorityList(array power_nodes, bool for_consumers)
{
	var list = [];
	for (var node in power_nodes)
	{
		var description;
		if (for_consumers)
		{
			description = { node = node, priority = node->GetConsumerPriority()};
		}
		else
		{
			description = { node = node, priority = node->GetProducerPriority()};
		}
		PushBack(list, description);
	}
	return list;
}

/**
 * Returns whether this network contains a power link.
 */
public func ContainsPowerLink(object link)
{
	return !!GetProducerLink(link) || !!GetConsumerLink(link);
}


/**
 * Returns the producer link in this network.
 */
public func GetProducerLink(object link)
{
	if (IsValueInArray(power_producers, link))
	{
		return link;
	}
}


/**
 * Returns the consumer link in this network.
 */
public func GetConsumerLink(object link)
{
	if (IsValueInArray(power_consumers, link))
	{
		return link;
	}
}


/**
 * Returns the storage link in this network.
 */
public func GetStorageLink(object link)
{
	if (IsValueInArray(power_storages, link))
	{
		return link;
	}
}


/**
 * Merge all the producers and consumers into their actual networks.
 */
private func RefreshPowerNetwork()
{
	RemoveHoles(power_producers);
	RemoveHoles(power_consumers);
	RemoveHoles(power_storages);
	GetPowerSystem()->DebugInfo("**************************************************************************");
	GetPowerSystem()->DebugInfo("POWR - Refreshing network %s", LogObject(this));
	for (var producer in power_producers)
	{
		// Remove from old network and add to new network.
		var actual_network = GetPowerSystem()->GetPowerNetwork(producer);
		if (actual_network && actual_network != this)
		{
			this->RemovePowerProducer(producer);
			actual_network->AddPowerProducer(producer);
		}
	}
	for (var consumer in power_consumers)
	{
		// Remove from old network and add to new network.
		var actual_network = GetPowerSystem()->GetPowerNetwork(consumer);
		if (actual_network && actual_network != this)
		{
			this->RemovePowerConsumer(consumer);
			actual_network->AddPowerConsumer(consumer);
		}
	}
	for (var storage in power_storages)
	{
		// Remove from old network and add to new network.
		var actual_network = GetPowerSystem()->GetPowerNetwork(storage);
		if (actual_network && actual_network != this)
		{
			this->RemovePowerStorage(storage);
			actual_network->AddPowerStorage(storage);
		}
	}
	GetPowerSystem()->DebugInfo("POWR - Refreshing network %s done - will list all contents now", LogObject(this));
	GetPowerSystem()->DebugInfo("POWR - Network %s producers: %v", LogObject(this), power_producers);
	GetPowerSystem()->DebugInfo("POWR - Network %s consumers: %v", LogObject(this), power_consumers);
	GetPowerSystem()->DebugInfo("POWR - Network %s storages: %v", LogObject(this), power_storages);
	GetPowerSystem()->DebugInfo("**************************************************************************");
}


/*-- Logging --*/


private func LogState(string tag)
{
	GetPowerSystem()->DebugInfo("==========================================================================");
	GetPowerSystem()->DebugInfo("POWR - State for network %v in frame %d with tag %s", this, FrameCounter(), tag);
	GetPowerSystem()->DebugInfo("==========================================================================");
	GetPowerSystem()->DebugInfo("POWR - GetPowerConsumptionNeed() = %d", GetPowerConsumptionNeed());
	GetPowerSystem()->DebugInfo("POWR - GetBarePowerAvailable() = %d", GetBarePowerAvailable());
	GetPowerSystem()->DebugInfo("POWR - GetPowerAvailable() = %d", GetPowerAvailable());
	GetPowerSystem()->DebugInfo("POWR - GetActivePowerAvailable() = %d", GetActivePowerAvailable());
	GetPowerSystem()->DebugInfo("POWR - GetPowerConsumption() = %d", GetPowerConsumption());
	GetPowerSystem()->DebugInfo("==========================================================================");
	return;
}


/* -- Saving -- */


/**
 * Helper object should not be saved.
 */
public func SaveScenarioObject()
{
	if (GetID() == GetPowerSystem()) 
	{
		return false;
	}
	return inherited(...);
}


/* -- Properties -- */

local power_producers;
local power_consumers;
local power_storages;
local is_neutral;

private func Construction()
{
	power_producers = [];
	power_consumers = [];
	power_storages = [];
	is_neutral = false;
}

/* -- Internals -- */

/**
 * Does an update of the power balance in the next frame.
 *
 * This is scheduled, so that objects can manipulate
 * the network as they like, without having to worry
 * about infinite recursions and the like.
 */
public func SchedulePowerBalanceUpdate()
{
	if (!GetEffect("FxUpdatePowerBalance", this))
		CreateEffect(FxUpdatePowerBalance, 1, 1);
}

local FxUpdatePowerBalance = new Effect {
	Timer = func ()
	{
		Target->DoPowerBalanceUpdate();
		return FX_Execute_Kill;
	},
};


/**
 * Checks the power balance after a change to this network: i.e. removal or addition
 * of a consumer or producer. 
 */
private func DoPowerBalanceUpdate()
{
	var power_level = 0;	// how much is produced?
	var power_demand = 0;	// how much is demanded?
	var power_capacity = 0;	// how much can be saved?
	var lowest_demand = nil;
	
	RemoveHoles(power_producers);
	RemoveHoles(power_consumers);
	RemoveHoles(power_storages);
	
	GetPowerSystem()->DebugInfo("==========================================================================");
	GetPowerSystem()->DebugInfo("POWR - Performing power balance update for network %v in frame %d", this, FrameCounter());


	// Add up all power needs to get the total demand
	for (var consumer in power_consumers)
	{
		var demand = consumer->GetPowerConsumption();
		power_demand += demand;
		
		if (lowest_demand == nil || demand < lowest_demand)
		{
			lowest_demand = demand;
		}
	}
	var should_produce_power = GetPowerAvailable() >= lowest_demand;

	GetPowerSystem()->DebugInfo("POWR - Consumers demand %d units", power_demand);
	
	// Add up all storage powers to get total storage power
	for (var storage in power_storages)
	{
		if (storage->GetStorageRemaining() > 0)
		{
			power_capacity += storage->GetStoragePower();
		}
	}

	// Activate producers if necessary
	for (var producer in power_producers)
	{
		var supply = producer->GetPowerProduction();

		// Not supplied yet? Switch on the producer, if possible
		if (should_produce_power && (power_level < power_demand))
		{
			// If production can be started
			if (!producer->IsPowerProductionActive() && producer->CanPowerProductionStart(Min(power_demand - power_level, supply)))
			{
				GetPowerSystem()->DebugInfo("POWR - Switch on producer %s", LogObject(producer));
				producer->OnPowerProductionStart();
			}
		}
		// All consumers have enough power, so switch off the remaining producers
		// or the lowest demand cannot be met, so switch all producers off, too
		else if (power_level >= power_demand + power_capacity)
		{
			if (producer->IsPowerProductionActive() && !producer->~IsSteadyPowerProducer())
			{
				GetPowerSystem()->DebugInfo("POWR - Switch off producer %s", LogObject(producer));
				producer->OnPowerProductionStop();
			}
		}

		// Production is on?	
		if (producer->IsPowerProductionActive())
		{
			power_level += supply;
			GetPowerSystem()->DebugInfo("POWR - %d units created by %s", supply, LogObject(producer));
		}
	}
	
	GetPowerSystem()->DebugInfo("POWR - Producers supply %d units", power_level);

	// Supply the consumers
	for (var consumer in power_consumers)
	{
		var demand = consumer->GetPowerConsumption();
		
		GetPowerSystem()->DebugInfo("POWR - Demand for %s: %d, current power level %d", LogObject(consumer), demand, power_level);

		// Still enough power for this consumer?
		if (power_level >= demand)
		{
			// Reduce available power
			power_level -= demand;

			// Non on? Switch on
			if (!consumer->HasEnoughPower())
			{
				GetPowerSystem()->DebugInfo("POWR - Consumer has enough power: %s", LogObject(consumer));
				consumer->OnEnoughPower();
			}
			GetPowerSystem()->DebugInfo("POWR - %d units consumed by %s", demand, LogObject(consumer));
		}
		// Not enough power
		else
		{
			// Still on? Switch off
			if (consumer->HasEnoughPower())
			{
				GetPowerSystem()->DebugInfo("POWR - Consumer has Insufficient power: %s", LogObject(consumer));
				consumer->OnNotEnoughPower();
			}
		}
	}

	// Put the remaining power into storages
	// This is done separately from supplying consumers, so that storages do not hinder the consumers
	// Distribution is equal among all power storages, so that as many storages as possible can
	// produce at the same time
	GetPowerSystem()->DebugInfo("POWR - Excess energy is %d units", power_level);

	for (var storage in power_storages)
	{
		storage->SetStorageInput(0);
	}
	while (power_level > 0)
	{	
		var total_change = 0;
		for (var storage in power_storages)
		{
			// Update remaining power level
			var change = storage->DoStorageInput(1);
			power_level -= change;
			total_change += change;
		}
		if (total_change == 0)
		{
			break;
		}
	}
	for (var storage in power_storages)
	{
		GetPowerSystem()->DebugInfo("Store %d power in %s", storage->GetStorageInput(), LogObject(storage));
	}
	
	GetPowerSystem()->DebugInfo("POWR - Wasted energy is %d units", power_level);
	GetPowerSystem()->DebugInfo("==========================================================================");

	NotifyOnPowerBalanceChange();
}


/**
 * Called when the power balance of this network changes: notify other objects depending on this.
 */
private func NotifyOnPowerBalanceChange()
{
	// Notify all power display objects a balance change has occured.
	for (var display_obj in FindObjects(Find_Func("IsPowerDisplay")))
	{
		if (GetPowerSystem()->GetPowerNetwork(display_obj) == this)
		{
			display_obj->~OnPowerBalanceChange(this);
		}
	}
}


private func LogObject(object target)
{
	return Format("%s (%d)", target->GetName(), target->ObjectNumber());
}

