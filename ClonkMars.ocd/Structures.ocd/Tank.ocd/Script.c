#include Library_MarsStructure
#include Library_MarsResearch
#include Library_ConstructionAnimation


/* --- Properties --- */

local Name = "$Name$";
local Description = "$Description$";
local ContainBlast = 1;
local Components = { Metal=3 };


/* --- Engine Callbacks --- */

func Definition(type)
{
	type.PictureTransformation = Trans_Translate(0, 0, -20000);
	return _inherited(type, ...);
}
