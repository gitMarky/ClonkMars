/*--- Rock ---*/

public func Construction()
{
	var graphic = Random(10);
	if(graphic)
		SetGraphics(Format("%d",graphic));
}

public func Hit(x, y)
{
	StonyObjectHit(x,y);
	return true;
}

local Collectible = 1;
local Name = "$Name$";
local Description = "$Description$";
local Plane = 450;
