#include Library_ScenarioMars

public func Initialize()
{
    _inherited(...);
    SetSky("Sky_Clouds");
    SetSkyParallax(0, 10, 0, 3, 0);

	// some natural disasters
	Earthquake->SetChance(3);
	Meteor->SetChance(31);
	Meteor->SetInterval(30 * 35);
	SetOxygenInAtmosphere(0);
}
