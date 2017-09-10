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
static POWER_SYSTEM_NETWORKS;

// A static variable that handles debug information being logged.
static POWER_SYSTEM_DEBUG;

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
	
	// Get the new network for this power link.
	var new_network = GetPowerNetwork(link);
	// Loop over existing networks and find the link.
	var old_network;
	for (var network in POWER_SYSTEM_NETWORKS)
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
			old_network->RemovePowerProducer(producer);
			new_network->AddPowerProducer(producer);		
		}
		var consumer = old_network->GetConsumerLink(link);
		if (consumer)
		{
			old_network->RemovePowerConsumer(consumer);
			new_network->AddPowerConsumer(consumer);		
		}
	}
}


/**
 * Definition call: Refreshes all power networks
 */
public func RefreshAllPowerNetworks()
{
	// Don't do anything if there are no power helpers created yet.
	if (GetType(POWER_SYSTEM_NETWORKS) != C4V_Array) return;
	
	// Special handling for neutral networks of which there should be at most one.
	var neutral_network_count = 0;
	
	// Do the same for all other helpers: delete / refresh.
	for (var index = GetLength(POWER_SYSTEM_NETWORKS) - 1; index >= 0; index--)
	{
		var network = POWER_SYSTEM_NETWORKS[index];
		if (!network)
		{
			continue;
		}

		if (network->IsEmpty())
		{
			network->RemoveObject();
			RemoveArrayIndex(POWER_SYSTEM_NETWORKS, index);
			continue;
		}

		RefreshPowerNetwork(network);
		if (network->IsNeutral())
		{
			neutral_network_count += 1;
		}
	}
	
	if (neutral_network_count > 1)
	{
		FatalError(Format("There were a total of %d neutral networks, at most there should be one", neutral_network_count));
	}
	return;
}


/**
 * Definition call: Merge all the producers and consumers into their actual networks.
 */
private func RefreshPowerNetwork(object network)
{
	if (network) network->RefreshPowerNetwork();
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
	GetPowerNetwork(link)->SchedulePowerBalanceUpdate();
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
	
	Init();

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
			helper = CreateNetwork();
			// Add to all linked flags.
			flag->SetPowerHelper(helper, true, true);
		}
	}
	// Otherwise, if no flag was available the object is neutral and needs a neutral helper.
	else
	{
		for (var network in POWER_SYSTEM_NETWORKS)
		{
			if (!network || !network->IsNeutral())
			{
				continue;
			}
			helper = network;
			break;
		}
		// Create the helper if it does not exist yet.
		if (helper == nil)
		{
			helper = CreateNetwork(true);
		}
	}
	
	return helper;
}

/**
 * Definition call: Create a new network and add it to the list of networks.
 * Can be a neutral network, if desired.
 */
private func CreateNetwork(bool neutral)
{
	Init();
	var network = CreateObject(GetPowerSystemNetwork(), 0, 0, NO_OWNER);
	PushBack(POWER_SYSTEM_NETWORKS, network);
	network->SetNeutral(neutral);
	return network;
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
	if (GetType(POWER_SYSTEM_NETWORKS) != C4V_Array)
	{
		POWER_SYSTEM_NETWORKS = [];
	}
	return;
}


/**
 * Definition call: Get the type of network helper object to create.
 *
 * @return id the definition of the network helper object. You can overload this function
 *            if you want to use a different power system.
 */
private func GetPowerSystemNetwork()
{
	return Library_PowerSystem_Network;
}


/**
 * Enables or disables the debugging information.
 */
public func SetDebugInfo(bool enable)
{
	POWER_SYSTEM_DEBUG = enable;
}


/**
 * Logs the power system info, if debugging is on.
 */
private func DebugInfo(string message)
{
	if (POWER_SYSTEM_DEBUG)
	{
		Log(message, ...);
	}
}
