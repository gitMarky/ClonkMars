/**
	Power line

	@author ST-DDT, Marky
*/

local pipe_kit;

private func Initialize()
{
	SetAction("Connect");
	SetVertexXY(0, GetX(), GetY());
	SetVertexXY(1, GetX(), GetY());
	this.LineColors = [RGB(80, 80, 120), RGB(80, 80, 120)];
}


// Returns true if this object is a functioning power line.
public func IsPowerLine()
{
	return GetAction() == "Connect";
}


// Returns whether this power line is connected to an object.
// Returns only actually connected objects if the parameter 'strict' is true.
public func IsConnectedTo(object obj, bool strict)
{
	return GetActionTarget(0) == obj || GetActionTarget(1) == obj || (!strict && pipe_kit == obj);
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


private func OnLineBreak(bool no_msg)
{
	Sound("Objects::LineSnap");
	if (!no_msg)
		BreakMessage();

	if (GetLineKit())
	{
		if (GetActionTarget(0)) GetActionTarget(0)->~OnLineDisconnect(GetLineKit());
		if (GetActionTarget(1)) GetActionTarget(1)->~OnLineDisconnect(GetLineKit());
	}
	
	return;
}


private func OnLineChange()
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


private func Destruction()
{
	if (GetActionTarget(0)) GetActionTarget(0)->~OnLineLineRemoval();
	if (GetActionTarget(1)) GetActionTarget(1)->~OnLineLineRemoval();
	return;
}


private func BreakMessage()
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
