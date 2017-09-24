#include Library_MarsStructure
#include Library_MarsResearch
#include Library_ConstructionAnimation
#include Library_PowerSystem_Storage
#include Library_PowerSystem_DisplayStatus

/* -- Properties -- */

local Name = "$Name$";
local Description = "$Description$";
local ContainBlast = 1;
local Components = { Metal=2, Silicon=2 };
local HitPoints = 25;

/* -- Power System -- */


private func Initialize()
{
	SetStoragePower(10);
	SetStorageCapacity(1500 * POWER_SYSTEM_TICK); // 150 seconds * 10 production * 36 frames
	SetProducerPriority(50);
	RegisterPowerStorage(this);
	
	return _inherited(...);
}
