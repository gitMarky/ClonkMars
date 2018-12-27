// TODO include Recycling
#include Library_MarsResearch

/* -- Properties -- */


local Name = "$Name$";
local Description = "$Description$";
local Collectible = 1;
local Components = { Metal = 1 };

local power_line;

/* -- Engine callbacks -- */

func Hit()
{
	Sound("Hits::GeneralHit?");
}

func Destruction()
{
	// Remove the line first, so that it does not provoke errors on destruction.
	// Actually there is an ill-defined state where line contains the line and is
	// removed. Then line = GetConnectedLine() causes an error, instead use the
	// slower find object variant.
	var line = GetConnectedLine();
	if (line)
		line->RemoveObject();
	return;
}


public func OnPowerLineRemoval()
{
	OnLineLengthChange();
	return;
}

public func OnLineLengthChange()
{
	// Update usage bar for a possible carrier (the clonk).
	var carrier = Contained();
	if (carrier)
		carrier->~OnInventoryChange();
	return;
}

// Display the line length bar over the line icon.
public func GetInventoryIconOverlay()
{
	var line = GetConnectedLine();
	if (!line) return;

	var percentage = 100 * line->GetLineLength() / line.LineMaxLength;
	var red = percentage * 255 / 100;
	var green = 255 - red;
	// Overlay a usage bar.
	var overlay =
	{
		Bottom = "0.75em",
		Margin = ["0.1em", "0.25em"],
		BackgroundColor = RGB(0, 0, 0),
		margin =
		{
			Margin = "0.05em",
			bar =
			{
				BackgroundColor = RGB(red, green, 0),
				Right = Format("%d%%", percentage),
			}
		}
	};
	return overlay;
}


/**
 * Container dies: Drop connected lines so they don't
 * draw huge lines over the landscape
 */
func IsDroppedOnDeath(object clonk)
{
	return !!GetConnectedLine();
}

/* ---------- Line Connection ---------- */


func ConnectLineTo(object target, string specific_line_state, bool block_cutting)
{
	if (!target || target->~QueryConnectLine(this)) return false;
	AddLineConnectionTo(target, block_cutting);
	target->~OnPowerLineConnect(this);
	Sound("Objects::Connect");
	return true;
}

/* ---------- Line Connection ---------- */


public func SetPowerLine(to_line)
{
	power_line = to_line;
}

/**
 Finds a line that is connected to this line kit.
 @return object the line, or nil if nothing was found.
 */
public func GetConnectedLine()
{
	return power_line;
}


/**
 * Connects a line to an object.
 *
 * The line kit will connect the line to the target object and itself first.
 * Otherwise, if the line kit already has a line, it connects that line to the target.
 *
 * Note: Reports a fatal error if the line would be connected to more than two targets
 *       at the same time.
 *     
 * @par target the target object
 */
func AddLineConnectionTo(object target, bool block_cutting)
{
	var line = GetConnectedLine();
	if (line)
	{
		if (line->IsConnectedTo(this, true))
		{
			line->SwitchConnection(this, target);
			SetPowerLine(line);
			line.BlockLineCutting = block_cutting;
			ScheduleCall(this, this.Enter, 1, nil, line); // delayed entrance, so that the message is still displayed above the clonk
			return line;
		}
		else
		{
			FatalError("This line is connected to two objects already!");
		}
	}
	else
	{
		return CreateLine(target, block_cutting);
	}
}


/**
 Cuts the connection between the line and an object.

 Note: Reports a fatal error if the target was not
       connected to the line.

 @par target the target object
 */
func CutLineConnection(object target)
{
	var line = GetConnectedLine();
	if (!line) return;

	// connected only to the kit and a structure
	if (line->IsConnectedTo(this, true))
	{
		target->OnLineDisconnect(this);
		line->RemoveObject();
	}
	// connected to the target and another structure
	else if (line->IsConnectedTo(target, true))
	{
		target->OnLineDisconnect(this);
		Exit(); // the kit was inside the line at this point.
		SetPosition(target->GetX(), target->GetY() + target->GetBottom() - this->GetBottom());
		line->SwitchConnection(target, this);
		SetPowerLine(line);
	}
	else
	{
		FatalError(Format("An object %v is trying to cut the line connection, but only objects %v and %v may request a disconnect", target, line->GetActionTarget(0), line->GetActionTarget(1)));
	}
}

// Returns whether the cutting line is blocked.
public func QueryCutLineConnection(object target)
{
	var line = GetConnectedLine();
	if (!line)
		return false;
	return line.BlockLineCutting;
}

/**
 * Creates a new line line that is connected to this line kit.
 *
 * @par target the target object.
 *
 * @return object the line that was created
 */
func CreateLine(object target, bool block_cutting)
{
	// Create and connect line.
	power_line = CreateObject(PowerLine, 0, 0, NO_OWNER);
	power_line->SetActionTargets(this, target);
	power_line->SetLineKit(this);
	power_line.BlockLineCutting = block_cutting;
	return power_line;
}


/** Will connect liquid line to building at the clonk's position. */
func ControlUse(object clonk, int x, int y)
{
	var target = FindObject(Find_AtPoint(), Find_Or(Find_Func("IsPowerProducer"), Find_Func("IsPowerConsumer") /* storages are producers, too */));
	if (target)
	{
		ConnectLineTo(target);
	}
	return true;
}


/**
 * Displays a message at top-level container of this object.
 * @par message the message
 */
func Report(string message)
{
	var reporter = this;
	var next = Contained();

	while(next)
	{
		reporter = next;
		next = reporter->Contained();
	}

	reporter->Message(message, ...);
}


/*-- Saving --*/

public func SaveScenarioObject(props)
{
	if (!inherited(props, ...)) return false;
	if (power_line) props->AddCall("PowerLine", this, "SetPowerLine", power_line);
	return true;
}
