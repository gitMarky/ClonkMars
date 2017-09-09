/**
	Power Library
	Handles the aspects for a single power networks, for each network in a round
	a copy of this library object is created. For each network it is being kept 
	track of which are the idle/active producers and idle/active consumers and 
	power is requested from producers and distributed for consumers according 
	to priority.
	
	Callbacks to the power producers (see producer library for details):
	 * OnPowerProductionStart(int amount)
	 * OnPowerProductionStop(int amount)
	 * GetProducerPriority()
	 * IsSteadyPowerProducer()
	 
	Callbacks to the power consumers (see consumer library for details):
	 * OnEnoughPower(int amount)
	 * OnNotEnoughPower(int amount)
	 * GetConsumerPriority()
	 * GetActualPowerConsumer()
	 
	The network object keeps track of the producers and consumers in lists.
	 * lib_power.idle_producers (currently not producing power)
	 * lib_power.active_producers (currently producing power)
	 * lib_power.waiting_consumers (waiting for power)
	 * lib_power.active_consumers (supplied consumers)
	Producers are stored according to {obj, prod_amount, priority} and consumers
	according to {obj, cons_amount, priority}.
	
	The power library and its components Library_Producer, Library_Consumer and
	Library_Storage depend on the following other definitions:
	 * StatusSymbol
	
	OPEN TODOS:
	 * Remove all the if (!link) checks, they are not needed in principle but errors arise when they are removed.
	 * Fix overproduction flowing into power storage (producers can be deactivated).
	 * Move network merging from flag to power library.
	 * Optimize network and flag removal.

	@author Zapper, Maikel, Marky
*/


/* -- Global stuff -- */

// A static variable is used to store the different power networks.
// This variable is also accessed by the flag library.
static LIB_POWR_Networks;

/**
 * Getter for the power system
 *
 * @return id the definition of the power system. You can overload this function
 *            if you want to use a different power system.
 */
global func GetPowerSystem()
{
	return Library_PowerSystem;
}


/* -- Definition Calls -- */

/**
 * Definition call: registers a power producer.
 */
public func RegisterPowerProducer(object producer)
{
	// Definition call safety checks.
	if (this != GetPowerSystem() || !producer || !producer->~IsPowerProducer())
	{
		return FatalError("RegisterPowerProducer() either not called from definition context or no producer specified.");
	}
	GetPowerSystem()->Init();
	GetPowerNetwork(producer)->AddPowerProducer(producer);
	return;
}


/**
 * Definition call: unregisters a power producer.
 */
public func UnregisterPowerProducer(object producer)
{
	// Definition call safety checks.
	if (this != GetPowerSystem() || !producer || !producer->~IsPowerProducer())
	{
		return FatalError("UnregisterPowerProducer() either not called from definition context or no producer specified.");
	}
	GetPowerSystem()->Init();
	GetPowerNetwork(producer)->RemovePowerProducer(producer);
	return;
}


/**
 * Definition call: registers a power consumer with specified amount.
 */
public func RegisterPowerConsumer(object consumer)
{
	// Definition call safety checks.
	if (this != GetPowerSystem() || !consumer || !consumer->~IsPowerConsumer())
	{
		return FatalError("RegisterPowerConsumer() either not called from definition context or no consumer specified.");
	}
	GetPowerSystem()->Init();
	GetPowerNetwork(consumer)->AddPowerConsumer(consumer);
	return;
}

/**
 * Definition call: unregisters a power consumer.
 */
public func UnregisterPowerConsumer(object consumer)
{
	// Definition call safety checks.
	if (this != GetPowerSystem() || !consumer || !consumer->~IsPowerConsumer())
	{
		return FatalError("UnregisterPowerConsumer() either not called from definition context or no consumer specified.");
	}
	GetPowerSystem()->Init();
	GetPowerNetwork(consumer)->RemovePowerConsumer(consumer);
	return;
}


/**
 * Definition call: transfers a power link from the network it is registered in to
 * the network it is currently in (base radius).
 */
public func TransferPowerLink(object link)
{
	// Definition call safety checks.
	if (this != GetPowerSystem() || !link)
	{
		return FatalError("TransferPowerLink() either not called from definition context or no link specified.");
	}
	/*
	// Get the new network for this power link.
	var new_network = GetPowerNetwork(link);
	// Loop over existing networks and find the link.
	var old_network;
	for (var network in LIB_POWR_Networks)
	{
		if (network && network->ContainsPowerLink(link))
		{
			old_network = network;
			break;
		}
	}
	// Only perform a transfer if the link was registered in an old network which is not equal to the new network.
	if (old_network && old_network != new_network)
	{
		var producer = old_network->GetProducerLink(link);
		if (producer)
		{
			old_network->RemovePowerProducer(producer.obj);
			new_network->AddPowerProducer(producer.obj);		
		}
		var consumer = old_network->GetConsumerLink(link);
		if (consumer)
		{
			old_network->RemovePowerConsumer(consumer.obj);
			new_network->AddPowerConsumer(consumer.obj);		
		}
	}
	*/
	return;
}

/**
 * Definition call: updates the network for this power link.
 */
public func UpdateNetworkForPowerLink(object link)
{
	// Definition call safety checks.
	if (this != GetPowerSystem() || !link)
	{
		return FatalError("UpdateNetworkForPowerLink() either not called from definition context or no link specified.");
	}
	GetPowerNetwork(link)->CheckPowerBalance();
	return;
}


/**
 * Definition call: gives the power helper object.
 */
public func GetPowerNetwork(object for_obj)
{
	// Definition call safety checks.
	if (this != GetPowerSystem() || !for_obj)
	{
		return FatalError("GetPowerNetwork() either not called from definition context or no object specified.");
	}
	
	// Get the flag corresponding to the object.	
	var flag = GetFlagpoleForPosition(for_obj->GetX() - GetX(), for_obj->GetY() - GetY());
	
	// Find the network helper object for this flag.
	var helper = nil;
	
	// Just get the helper from the flag.
	if (flag)
	{
		helper = flag->GetPowerHelper();
		// Create the helper if it does not exist yet.
		if (helper == nil)
		{
			helper = CreateObject(GetPowerSystem(), 0, 0, NO_OWNER);
			LIB_POWR_Networks[GetLength(LIB_POWR_Networks)] = helper;
			// Add to all linked flags.
			flag->SetPowerHelper(helper);
			for (var linked_flag in flag->GetLinkedFlags())
			{
				if (!linked_flag)
				{
					continue;
				}
				// Assert different power helpers for the same network.
				if (linked_flag->GetPowerHelper() != nil)
				{
					FatalError("Flags in the same network have different power helpers.");
				}
				linked_flag->SetPowerHelper(helper);
			}
		}
	}
	// Otherwise, if no flag was available the object is neutral and needs a neutral helper.
	else
	{
		if (!LIB_POWR_Networks)
		{
			LIB_POWR_Networks = [];
		}
		for (var network in LIB_POWR_Networks)
		{
			if (!network) // TODO || !network.lib_power.neutral_network)
			{
				continue;
			}
			helper = network;
			break;
		}
		// Create the helper if it does not exist yet.
		if (helper == nil)
		{
			helper = CreateObject(GetPowerSystem(), 0, 0, NO_OWNER);
			// TODO helper.lib_power.neutral_network = true;
			//LIB_POWR_Networks[GetLength(LIB_POWR_Networks)] = helper;
			PushBack(LIB_POWR_Networks, helper);
		}
	}
	
	return helper;
}


/**
 * Definition call: Initializes tracking of the power networks.
 */
public func Init()
{
	// Definition call safety checks.
	if (this != GetPowerSystem())
	{
		return;
	}
	// Initialize the list of networks if not done already.
	if (GetType(LIB_POWR_Networks) != C4V_Array)
	{
		LIB_POWR_Networks = [];
	}
	return;
}


/* -- Network object - Interface -- */

/**
 * Adds a power producer to the network.
 */
public func AddPowerProducer(object producer)
{
	if (!IsValueInArray(power_producers, producer))
	{
		PushBack(power_producers, producer);		
		Log("POWR - AddPowerProducer(): network = %v, frame = %d, producer = %v", this, FrameCounter(), producer);
		CheckPowerBalance(); // Check the power balance of this network, since a change has been made.
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
		Log("POWR - RemovePowerProducer(): network = %v, frame = %d, producer = %v", this, FrameCounter(), producer);
		CheckPowerBalance(); // Check the power balance of this network, since a change has been made.
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
		Log("POWR - AddPowerConsumer(): network = %v, frame = %d, consumer = %v", this, FrameCounter(), consumer);
		CheckPowerBalance(); // Check the power balance of this network, since a change has been made.
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
		Log("POWR - RemovePowerConsumer(): network = %v, frame = %d, consumer = %v", this, FrameCounter(), consumer);
		CheckPowerBalance(); // Check the power balance of this network, since a change has been made.
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
		Log("POWR - AddPowerStorage(): network = %v, frame = %d, storage = %v", this, FrameCounter(), storage);
		CheckPowerBalance(); // Check the power balance of this network, since a change has been made.
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
		Log("POWR - RemovePowerStorage(): network = %v, frame = %d, storage = %v", this, FrameCounter(), storage);
		CheckPowerBalance(); // Check the power balance of this network, since a change has been made.
	}
}


// TODO: Remove this function
public func CheckPowerBalance()
{
	DoPowerBalanceUpdate();
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

private func SortProducers()
{
	// First update the priorities of the producers and then sort them according to priority.
	//UpdatePriorities(power_producers, false);
	if (GetLength(power_producers) > 1) // TODO: this check should not be necessary.
	{
		SortArrayByProperty(power_producers, "lib_power_system.producer.priority", true);
	}
}

// TODO
// Activates producers according to priotrity from all producers in the network until needed power is met.
// This function automatically deactivates producers which had a lower priority over newly activated ones.
private func RefreshProducers(int power_need)
{
/*
	// Debugging logs.
	//Log("POWR - RefreshProducers(): network = %v, frame = %d, power_need = %d", this, FrameCounter(), power_need);
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
	//UpdatePriorities(power_consumers, true);
	if (GetLength(power_consumers) > 1) // TODO: this check should not be necessary.
	{
		SortArrayByProperty(power_consumers, "lib_power_system.consumer.priority", true);
	}
}


// Activates consumers according to priotrity from all consumers in the network until available power is used.
// This function automatically deactivates consumer which had a lower priority over newly activated ones.
private func RefreshConsumers(int power_available)
{
/*
	// Debugging logs.
	//Log("POWR - RefreshConsumers(): network = %v, frame = %d, power_available = %d", this, FrameCounter(), power_available);
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
 * Updates the priorities of either a list of consumers or producers.
 */
private func UpdatePriorities(array link_list, bool for_consumers)
{
	for (var link in link_list)
	{
		if (for_consumers)
		{
			link.priority = link.obj->~GetConsumerPriority();
		}
		else
		{
			link.priority = link.obj->~GetProducerPriority();
		}
	}
	return;
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
	Log("==========================================================================");
	Log("POWR - State for network %v in frame %d with tag %s", this, FrameCounter(), tag);
	Log("==========================================================================");
	Log("POWR - GetPowerConsumptionNeed() = %d", GetPowerConsumptionNeed());
	Log("POWR - GetBarePowerAvailable() = %d", GetBarePowerAvailable());
	Log("POWR - GetPowerAvailable() = %d", GetPowerAvailable());
	Log("POWR - GetActivePowerAvailable() = %d", GetActivePowerAvailable());
	Log("POWR - GetPowerConsumption() = %d", GetPowerConsumption());
	Log("==========================================================================");
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

local power_level;
local power_demand;
local power_producers;
local power_consumers;
local power_storages;

private func Construction()
{
	power_producers = [];
	power_consumers = [];
	power_storages = [];
}

/* -- Internals -- */

/**
 * Checks the power balance after a change to this network: i.e. removal or addition
 * of a consumer or producer. The producers and consumers will be refreshed such that 
 * the ones with highest priority will be active.
 */
private func DoPowerBalanceUpdate()
{
	ResetPowerLevel();
	SortConsumers();
	SortProducers();

	for (var consumer in power_consumers)
	{
		DoPowerDemand(consumer->GetPowerConsumption());
	}

	for (var producer in power_producers)
	{
		var supply = producer->GetPowerProduction();
	}

	//	for(var pObj in FindObjects(Find_Func("PowerConsumption", 0))); // alle Produzenten liefern Energie
	//	for(var pObj in FindObjects(Find_Func("PowerConsumption", 1))); // alle Verbraucher ziehen Energie ab und schalten sich bei Mangel aus
	//	for(var pObj in FindObjects(Find_Func("PowerConsumption", 2))); // alle Speicher nehmen Energie auf - separat von Verbrauchern, weil sonst der Speicher den Verbrauchern den Strom wegzieht


	NotifyOnPowerBalanceChange();
}

/* -- Setters & Getters -- */

private func ResetPowerLevel()
{
	power_level = 0;
	power_demand = 0;
}

private func DoPowerDemand(int change)
{
	power_demand += change;
}
