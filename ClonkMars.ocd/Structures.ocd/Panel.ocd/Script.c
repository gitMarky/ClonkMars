/* -- Engine callbacks -- */

private func Construction()
{
	_inherited(...);
	
	if (GetOwner() == NO_OWNER)
	{
		SetColor(RGB(150, 111, 139));
	}
}


/* -- Properties -- */

local Name = "$Name$";
local Description = "$Description$";
local ContainBlast = 1;
local Components = { Metal=1, Plastic=1 };
