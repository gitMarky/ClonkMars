

/* -- Network object - Interface -- */

/**
 * Adds a power producer to the network.
 */
public func AddPowerProducer(object producer)
{
	if (!IsValueInArray(power_producers, producer))
	{
		PushBack(power_producers, producer);		
		GetPowerSystem()->DebugInfo("POWR - AddPowerProducer(): network = %v, frame = %d, producer = %v", this, FrameCounter(), producer);
		SortProducers(); // This is necessary only when adding an object to the list
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
		RemoveArrayValue(power_producers, producer);		
		GetPowerSystem()->DebugInfo("POWR - RemovePowerProducer(): network = %v, frame = %d, producer = %v", this, FrameCounter(), producer);
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
		GetPowerSystem()->DebugInfo("POWR - AddPowerConsumer(): network = %v, frame = %d, consumer = %v", this, FrameCounter(), consumer);
		SortConsumers(); // This is necessary only when adding an object to the list
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
		GetPowerSystem()->DebugInfo("POWR - RemovePowerConsumer(): network = %v, frame = %d, consumer = %v", this, FrameCounter(), consumer);
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
		GetPowerSystem()->DebugInfo("POWR - AddPowerStorage(): network = %v, frame = %d, storage = %v", this, FrameCounter(), storage);
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
		GetPowerSystem()->DebugInfo("POWR - RemovePowerStorage(): network = %v, frame = %d, storage = %v", this, FrameCounter(), storage);
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


/* -- Internals -- */

private func SortProducers()
{
	SortPowerNodes(power_producers, false);
}

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

// TODO
// Activates producers according to priotrity from all producers in the network until needed power is met.
// This function automatically deactivates producers which had a lower priority over newly activated ones.
private func RefreshProducers(int power_need)
{
/*
	// Debugging logs.
	//GetPowerSystem()->DebugInfo("POWR - RefreshProducers(): network = %v, frame = %d, power_need = %d", this, FrameCounter(), power_need);
	// Keep track of the power found and which was the last link to satisfy the need. 
	var power_found = 0;
	var satisfy_need_link = nil;
	for (var index = GetLength(all_producers) - 1; index >= 0; index--)
	{
		var link = all_producers[index];
		if (!link)
			continue;
		// Still need for a new producer, check is the link was not already active, if so activate.
		if (power_found < power_need)
		{
			// Update the power found and the last link.
			power_found += link.prod_amount;
			satisfy_need_link = index;
			var idx = GetIndexOf(lib_power.idle_producers, link);
			if (idx != -1)
			{
				PushBack(lib_power.active_producers, link);
				RemoveArrayIndex(lib_power.idle_producers, idx);
				// On production start callback to the activated producer.
				link.obj->OnPowerProductionStart(link.prod_amount);
				VisualizePowerChange(link.obj, 0, link.prod_amount, false);
			}
		}
		// No need to activate producers anymore, check if producer is active, if so deactivate.
		else
		{
			var idx = GetIndexOf(lib_power.active_producers, link);
			// It is not possible to deactivate a steady power producer.
			if (idx != -1 && !link.obj->IsSteadyPowerProducer())
			{
				PushBack(lib_power.idle_producers, link);
				RemoveArrayIndex(lib_power.active_producers, idx);
				// On production stop callback to the deactivated producer.
				link.obj->OnPowerProductionStop(link.prod_amount);
				VisualizePowerChange(link.obj, link.prod_amount, 0, false);
			}
		}
	}
	// This procedure might actually have activated too much power and a power producer
	// with high priority but low production might not be necessary, deactivate these.
	for (var index = satisfy_need_link + 1; index < GetLength(all_producers); index++)
	{
		var link = all_producers[index];
		if (!link)
			continue;
		// Power producer is not needed so it can be deactivated.
		if (power_found - link.prod_amount >= power_need)
		{
			var idx = GetIndexOf(lib_power.active_producers, link);
			// It is not possible to deactivate a steady power producer.
			if (idx != -1 && !link.obj->IsSteadyPowerProducer())
			{
				power_found -= link.prod_amount;
				PushBack(lib_power.idle_producers, link);
				RemoveArrayIndex(lib_power.active_producers, idx);
				// On production stop callback to the deactivated producer.
				link.obj->OnPowerProductionStop(link.prod_amount);
				VisualizePowerChange(link.obj, link.prod_amount, 0, false);
			}		
		}	
	}
	return;
*/
}


private func SortConsumers()
{
	SortPowerNodes(power_consumers, true);
}


// Activates consumers according to priotrity from all consumers in the network until available power is used.
// This function automatically deactivates consumer which had a lower priority over newly activated ones.
private func RefreshConsumers(int power_available)
{
/*
	// Debugging logs.
	//GetPowerSystem()->DebugInfo("POWR - RefreshConsumers(): network = %v, frame = %d, power_available = %d", this, FrameCounter(), power_available);
	// Keep track of the power used.
	var power_used = 0;
	for (var index = GetLength(all_consumers) - 1; index >= 0; index--)
	{
		var link = all_consumers[index];
		if (!link)
			continue;
		// Determine the consumption of this consumer, taking into account the power need.
		var consumption = link.cons_amount;
		if (!link.obj->HasPowerNeed())
			consumption = 0;			
		// Too much power has been used, check if this link was active, if so remove from active.
		// Or if the links is a power storage and there is other storage actively producing remove as well.
		if (power_used + consumption > power_available || (link.obj->~IsPowerStorage() && HasProducingStorage()))
		{
			var idx = GetIndexOf(lib_power.active_consumers, link);
			if (idx != -1)
			{
				PushBack(lib_power.waiting_consumers, link);
				RemoveArrayIndex(lib_power.active_consumers, idx);
				// On not enough power callback to the deactivated consumer.
				link.obj->OnNotEnoughPower(consumption);
				VisualizePowerChange(link.obj, consumption, 0, true);
			}
		}
		// In the other case see if consumer is not yet active, if so activate.
		else
		{
			power_used += consumption;
			var idx = GetIndexOf(lib_power.waiting_consumers, link);
			if (idx != -1)
			{
				PushBack(lib_power.active_consumers, link);
				RemoveArrayIndex(lib_power.waiting_consumers, idx);
				// On enough power callback to the activated consumer.
				link.obj->OnEnoughPower(consumption);
				VisualizePowerChange(link.obj, 0, consumption, false);
			}		
		}	
	}
	return;
*/
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
 * Returns whether this network contains a power link.
 */
/*
public func ContainsPowerLink(object link)
{
	return !!GetProducerLink(link) || !!GetConsumerLink(link);
}
*/

/**
 * Returns the producer link in this network.
 */
 /*
public func GetProducerLink(object link)
{
	for (var test_link in Concatenate(lib_power.idle_producers, lib_power.active_producers))
		if (test_link.obj == link)
			return test_link;
	return;
}

// Returns the consumer link in this network.
public func GetConsumerLink(object link)
{
	for (var test_link in Concatenate(lib_power.waiting_consumers, lib_power.active_consumers))
		if (test_link.obj == link)
			return test_link;
	return;
}

// Returns a list of all the power links in this network.
public func GetAllPowerLinks()
{
	// Combine producers and consumers into a list of all links.
	var all_producers = Concatenate(lib_power.idle_producers, lib_power.active_producers);
	var all_consumers = Concatenate(lib_power.waiting_consumers, lib_power.active_consumers);
	var all_links = Concatenate(all_producers, all_consumers);
	// Remove duplicate entries with the same link object.
	for (var index = GetLength(all_links) - 1; index >= 1; index--)
	{
		var obj = all_links[index].obj;
		for (var test_index = index - 1; test_index >= 0; test_index--)
		{
			if (obj == all_links[test_index].obj)
			{
				RemoveArrayIndex(all_links, index);
				break;
			}
		}
	}
	return all_links;
}
*/

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






/* -- Control effect -- */
/*
local FxPowerSystem = new Effect
{
	Construction = func()
	{
		this.Interval = 50;
	},
	

	Timer = func()
	{
		for (var network in FindObjects(Find_Func("PowerSystem_IsNetwork")))
		{
			network->DoPowerControlCycle();
		}
	}
};

public func Init()
{
	CreateEffect(FxPowerSystem, 1, 50);
}
*/
/* -- Properties -- */

//local power_level;
//local power_demand;
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
 * of a consumer or producer. The producers and consumers will be refreshed such that 
 * the ones with highest priority will be active.
 */
private func DoPowerBalanceUpdate()
{
	var power_level = 0;
	var power_demand = 0;
	
	GetPowerSystem()->DebugInfo("==========================================================================");
	GetPowerSystem()->DebugInfo("POWR - Performing power balance update for network %v in frame %d", this, FrameCounter());


	// Add up all power needs to get the total demand
	for (var consumer in power_consumers)
	{
		power_demand += consumer->GetPowerConsumption();
	}

	GetPowerSystem()->DebugInfo("POWR - Consumers demand %d units", power_demand);

	// Activate producers if necessary
	for (var producer in power_producers)
	{
		// Not supplied yet? Switch on the producer, if possible
		if (power_level < power_demand)
		{
			var supply = producer->GetPowerProduction();

			if (producer->IsPowerProductionActive())
			{
				// TODO: Update production symbol in case the amount has changed
			}
			else
			{
				// If production can be started
				if (producer->OnPowerProductionStart())
				{
					power_level += supply; // The producer supplies, so raise the power level
				}
			}
		}
		// All consumers have enough power, so switch off the remaining producers
		else
		{
			if (producer->IsPowerProductionActive())
			{
				producer->OnPowerProductionStop();
			}
		}
	}
	
	GetPowerSystem()->DebugInfo("POWR - Producers supply %d units", power_level);

	// Supply the consumers
	for (var consumer in power_consumers)
	{
		var demand = consumer->GetPowerConsumption();
		
		// Still enough power for this consumer?
		if (power_level >= demand)
		{
			// Reduce available power
			power_level -= demand;

			// Non on? Switch on
			if (!consumer->HasEnoughPower())
			{
				consumer->OnEnoughPower();
			}
		}
		// Not enough power
		else
		{
			// Still on? Switch off
			if (consumer->HasEnoughPower())
			{
				consumer->OnNotEnoughPower();
			}
		}
	}

	// TODO: Put the remaining power into storages
	// This is done separately from supplying consumers, so that storages do not hinder the consumers
	GetPowerSystem()->DebugInfo("POWR - Excess energy is %d units", power_level);
	GetPowerSystem()->DebugInfo("==========================================================================");

	NotifyOnPowerBalanceChange();
}

/* -- Setters & Getters -- */
/*
private func ResetPowerBalance()
{
	power_level = 0;
	power_demand = 0;
}

private func DoPowerDemand(int change)
{
	power_demand += change;
}
*/