/**
	Power line

	@author ST-DDT, Marky
*/

local pipe_kit;

/* -- Engine callbacks -- */


func Construction()
{
	// Initialize the single proplist for the library.
	if (lib_linked == nil)
		lib_linked = {};
	// Set some default variables.
	lib_linked.linked_objects = [];
	return _inherited(...);
}


func Destruction()
{
	PowerLine->RefreshAllLineNetworks();
	if (GetActionTarget(0)) GetActionTarget(0)->~OnLineLineRemoval();
	if (GetActionTarget(1)) GetActionTarget(1)->~OnLineLineRemoval();
	return _inherited(...);
}


func Initialize()
{
	SetAction("Connect");
	SetVertexXY(0, GetX(), GetY());
	SetVertexXY(1, GetX(), GetY());
	this.LineColors = [RGB(120, 120, 160), RGB(80, 80, 120)];
	return _inherited(...);
}

/* -- Line code -- */

// Returns true if this object is a functioning power line.
public func IsPowerLine()
{
	return GetAction() == "Connect";
}


// Returns whether this power line is connected to an object.
// Returns only actually connected objects if the parameter 'strict' is true.
public func IsConnectedTo(object obj, bool strict)
{
	if (obj && obj->~IsPowerLine())
	{
		return obj->IsConnectedTo(GetActionTarget(0)) || obj->IsConnectedTo(GetActionTarget(1));
	}
	else
	{
		return GetActionTarget(0) == obj || GetActionTarget(1) == obj || (!strict && pipe_kit == obj);
	}
}


// Returns the object which is connected to obj through this power line.
public func GetConnectedObject(object obj)
{
	if (GetActionTarget(0) == obj)
		return GetActionTarget(1);
	if (GetActionTarget(1) == obj)
		return GetActionTarget(0);
	return;
}


// Switches connection from connected_to to another obj.
public func SwitchConnection(object connected_to, object obj)
{
	var target0 = GetActionTarget(0), target1 = GetActionTarget(1);

	if (target0 == connected_to) target0 = obj;
	if (target1 == connected_to) target1 = obj;

	SetActionTargets(target0, target1);

	PowerLine->RefreshAllLineNetworks();
}


// Saves the power line object that created this line.
public func SetLineKit(object obj)
{
	pipe_kit = obj;
}


public func GetLineKit()
{
	if (pipe_kit)
	{
		return pipe_kit;
	}
	else
	{
		FatalError("Unexpected error: This power line has lost its line kit!");
	}
}


func OnLineBreak(bool no_msg)
{
	Sound("Objects::LineSnap");
	if (!no_msg)
	{
		BreakMessage();
	}

	if (GetLineKit())
	{
		if (GetActionTarget(0)) GetActionTarget(0)->~OnLineDisconnect(GetLineKit());
		if (GetActionTarget(1)) GetActionTarget(1)->~OnLineDisconnect(GetLineKit());
	}

	PowerLine->RefreshAllLineNetworks();
	return;
}


func OnLineChange()
{
	// Notify action targets about line change.
	var act1 = GetActionTarget(0);
	var act2 = GetActionTarget(1);
	if (act1) act1->~OnLineLengthChange(this);
	if (act2) act2->~OnLineLengthChange(this);

	// Break line if it is too long.
	if (GetLineLength() > this.LineMaxLength)
	{
		OnLineBreak();
		RemoveObject();
	}
	return;
}


// Returns the length between all the vertices.
public func GetLineLength()
{
	var current_length = 0;
	for (var index = 0; index < GetVertexNum() - 1; index++)
		current_length += Distance(GetVertex(index, VTX_X), GetVertex(index, VTX_Y), GetVertex(index + 1, VTX_X), GetVertex(index + 1, VTX_Y));
	return current_length;
}


func BreakMessage()
{
	var line_end = GetLineKit();
	if (line_end) line_end->Report("$TxtLineBroke$");
	return;
}


public func SaveScenarioObject(props)
{
	if (!inherited(props, ...)) return false;
	SaveScenarioObjectAction(props);
	if (pipe_kit) props->AddCall("LineKit", this, "SetLineKit", pipe_kit);
	return true;
}


/*-- Properties --*/


local Name = "$Name$";
local LineMaxLength = 1200;
local BlockLineCutting = false;


local ActMap = {
	Connect = {
		Prototype = Action,
		Name = "Connect",
		Procedure = DFA_CONNECT,
		NextAction = "Connect"
	}
};


/**
	Link Library

	Helps creating a network of linked objects.

	Important notes when including this library:
	 * The object including this library should return _inherited(...) in the
	   Construction, Initialize and Destruction callback if overloaded.

	@author Zapper, Maikel
*/


// All object related local variables are stored in a single proplist.
// This reduces the chances of clashing local variables. See
// Construction for which variables are being used.
local lib_linked;


/* -- Library Code -- */

func RefreshAllLineNetworks()
{
	Log("Refresh all line networks");
	var all_lines = FindObjects(Find_Func("IsPowerLine"));

	for (var line in all_lines)
	{
		line->RefreshLinkedObjects(all_lines);
	}

	GetPowerSystem()->RefreshAllPowerNetworks();
}


// Refreshes the linked objects for this object and also updates the linked objects of the objects linked to this object.
// TODO: Maybe there is a need to update the links of the objects which were linked before but are not now.
func RefreshLinkedObjects(array found_objects)
{
	// Construct a list fo currently linked objects (to this object).
	var current_linked_objects = [];
	// Do this by iterating over all directly linked objects and go outward from this object.
	var iterate_objects = [this];
	// Once the list of iterated objects is empty we are done.
	while (GetLength(iterate_objects))
	{
		// Store all the objects found in the last iteration which are not this object.
		for (var link in iterate_objects)
		{
			if (link != this)
				PushBack(current_linked_objects, link);
		}
		// Find the new iteration of objects which are connected to the objects in the previous iteration.
		var previous_iterate_objects = iterate_objects;
		iterate_objects = [];
		for (var prev_object in previous_iterate_objects)
		{
			for (var link in found_objects)
			{
				// And must not be an already found object or this object.
				if (GetIndexOf(current_linked_objects, link) != -1 || link == this)
				{
					continue;
				}
				// Neither may it be an already found object in this loop.
				if (GetIndexOf(iterate_objects, link) != -1)
				{
					continue;
				}
				// Last, check whether the new object is really connected to the previous object.
				if (!prev_object->~IsConnectedTo(link))
				{
					continue;
				}
				PushBack(iterate_objects, link);
			}
		}
	}

	//Log("Updated power lines for %v: %v", this, current_linked_objects);


	// Update the linked objects of this object.
	lib_linked.linked_objects = current_linked_objects;

	// Update the linked objects for all other linked objects as well.
	for (var other in GetLinkedObjects())
	{
		other->CopyLinkedObjects(this, current_linked_objects);
	}
	// Since the connected objects have been updated it is necessary to update the power helper as well.
	// We then just possibly remove the old ones if they exist.
	var helper = GetPowerSystem()->CreateNetwork(false);
	//Log("Created network helper %v", helper);
	SetPowerHelper(helper, true, false);
	// Now merge all networks into the newly created network.
	GetPowerSystem()->RefreshAllPowerNetworks();
	//Log("Network helper after refreshing all networks %v", helper);
	//ScheduleCall(this, this.Log, 2, 0, "After network update helper is %v", helper);
}


// Copy the linked objects from another object (from) and its object list.
func CopyLinkedObjects(object from, array object_list)
{
	lib_linked.linked_objects = object_list[:];
	for (var i = GetLength(lib_linked.linked_objects) - 1; i >= 0; --i)
		if (lib_linked.linked_objects[i] == this)
			lib_linked.linked_objects[i] = from;
	return;
}


public func GetLinkedObjects() {return lib_linked.linked_objects; }


/*-- Power System --*/


public func GetPowerHelper() { return lib_linked.power_helper; }

public func SetPowerHelper(object to, bool update_linked_objects, bool report_inconsistency)
{
	var old_network = GetPowerHelper();
	lib_linked.power_helper = to;
	// Update linked objects
	if (update_linked_objects)
	{
		for (var linked_object in GetLinkedObjects())
		{
			if (!linked_object)
				continue;
			// Assert different power helpers for the same network.
			if (report_inconsistency && linked_object->GetPowerHelper() != old_network)
			{
				FatalError("Flags in the same network have different power helpers.");
			}
			linked_object->SetPowerHelper(to);
		}
	}
	return;
}

