#include Library_MarsStructure
#include Library_MarsResearch


/* -- Properties -- */

local Name = "$Name$";
local Description = "$Description$";
local ContainBlast = 1;
local Components = { Metal=1, Plastic=1 };

local ActMap = {
construction_progress = {
	Prototype = Action,
	Procedure = DFA_NONE,
	Name = "construction_progress",
	Animation = "construction_progress",
	Length = 1001,
	Delay = 0,
},
};


/* -- Engine callbacks -- */

private func Construction()
{
	_inherited(...);
	
	if (GetOwner() == NO_OWNER)
	{
		SetColor(RGB(150, 111, 139));
	}
}

