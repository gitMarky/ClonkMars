/**
	Power System
	Unit tests for the power system. Invokes tests by calling the 
	global function Test*_OnStart(int plr) and iterate through all 
	tests. The test is completed once Test*_Completed() returns
	true. Then Test*_OnFinished() is called, to be able to reset 
	the scenario for the next test.
	
	With LaunchTest(int nr) a specific test can be launched when
	called during runtime. A test can be skipped by calling the
	function SkipTest().
	
	@author Maikel, Marky
*/


static script_plr;

protected func Initialize()
{
	// Create a script player for some tests.
	script_plr = nil;
	CreateScriptPlayer("PowerBuddy", RGB(0, 0, 255), nil, CSPF_NoEliminationCheck);
	GetPowerSystem()->SetDebugInfo(true);
	return;
}

protected func InitializePlayer(int plr)
{
	// Set zoom to full map size.
	SetPlayerZoomByViewRange(plr, LandscapeWidth(), nil, PLRZOOM_Direct);
	
	// No FoW to see everything happening.
	SetFoW(false, plr);
	
	// All players belong to the first team.
	// The second team only exists for testing.
	SetPlayerTeam(plr, 1);
		
	// Initialize script player.
	if (GetPlayerType(plr) == C4PT_Script)
	{
		// Store the player number.
		if (script_plr == nil)
			script_plr = plr;
		// No crew needed.
		GetCrew(plr)->RemoveObject();
		return;
	}	
	
	// Move player to the start of the scenario.
	GetCrew(plr)->SetPosition(120, 150);
	
	// Some knowledge to construct a flagpole.
	GetCrew(plr)->CreateContents(Hammer);
	SetPlrKnowledge(plr, Flagpole);
	
	// Add test control effect.
	var effect = AddEffect("IntTestControl", nil, 100, 2);
	effect.testnr = 1;
	effect.launched = false;
	effect.plr = plr;
	return;
}

protected func RemovePlayer(int plr)
{
	// Remove script player.
	if (GetPlayerType(plr) == C4PT_Script)
	{
		if (plr == script_plr)
			script_plr = nil;
		return;	
	}
	return;
}


/*-- Test Control --*/

// Aborts the current test and launches the specified test instead.
global func LaunchTest(int nr)
{
	// Get the control effect.
	var effect = GetEffect("IntTestControl", nil);
	if (!effect)
	{
		// Create a new control effect and launch the test.
		effect = AddEffect("IntTestControl", nil, 100, 2);
		effect.testnr = nr;
		effect.launched = false;
		effect.plr = GetPlayerByIndex(0, C4PT_User);
		return;
	}
	// Finish the currently running test.
	Call(Format("~Test%d_OnFinished", effect.testnr));
	// Start the requested test by just setting the test number and setting 
	// effect.launched to false, effect will handle the rest.
	effect.testnr = nr;
	effect.launched = false;
	return;
}

// Calling this function skips the current test, does not work if last test has been ran already.
global func SkipTest()
{
	// Get the control effect.
	var effect = GetEffect("IntTestControl", nil);
	if (!effect)
		return;
	// Finish the previous test.
	Call(Format("~Test%d_OnFinished", effect.testnr));
	// Start the next test by just increasing the test number and setting 
	// effect.launched to false, effect will handle the rest.
	effect.testnr++;
	effect.launched = false;
	return;
}


/*-- Test Effect --*/

global func FxIntTestControlStart(object target, proplist effect, int temporary)
{
	if (temporary)
		return FX_OK;
	// Set default interval.
	effect.Interval = 2;
	return FX_OK;
}

global func FxIntTestControlTimer(object target, proplist effect)
{
	// Launch new test if needed.
	if (!effect.launched)
	{
		// Log test start.
		Log("=====================================");
		Log("Test %d started:", effect.testnr);
		// Start the test if available, otherwise finish test sequence.
		if (!Call(Format("~Test%d_OnStart", effect.testnr), effect.plr))
		{
			Log("Test %d not available, the previous test was the last test.", effect.testnr);
			Log("=====================================");
			Log("All tests have been successfully completed!");
			return FX_Execute_Kill;
		}
		effect.launched = true;
	}		
	// Check whether the current test has been finished.
	if (Call(Format("Test%d_Completed", effect.testnr)))
	{
		effect.launched = false;
		//RemoveTest();
		// Call the test on finished function.
		Call(Format("~Test%d_OnFinished", effect.testnr));
		// Log result and increase test number.
		Log("Test %d successfully completed.", effect.testnr);
		effect.testnr++;
	}
	return FX_OK;
}


/*-- Power Tests --*/

// Simple test for one steady source and one steady consumer.
global func Test1_OnStart(int plr)
{
	// Power source: one wind generator.
	SetWindFixed(50);
	CreateObjectAbove(WindGenerator, 72, 160, plr);
	
	// Power consumer: one workshop.
	var workshop = CreateObjectAbove(ToolsWorkshop, 110, 160, plr);
	workshop->CreateContents(Wood, 2);
	workshop->CreateContents(Metal, 2);
	workshop->AddToQueue(Shovel, 2);
	
	// Log what the test is about.
	Log("A steady power source (wind generator) supplying an on-demand consumer (workshop).");
	return true;
}

global func Test1_Completed()
{
	if (ObjectCount(Find_ID(Shovel)) >= 2)
		return true;
	return false;
}

global func Test1_OnFinished()
{
	// Remove wind generator, compensator and workshop.
	RemoveAll(Find_Or(Find_ID(WindGenerator), Find_ID(ToolsWorkshop)));
	return;
}


/*-- Helper Functions --*/

global func SetWindFixed(int strength)
{
	strength = BoundBy(strength, -100, 100);
	var effect = GetEffect("IntFixedWind");
	if (!effect)
		effect = AddEffect("IntFixedWind", nil, 100, 1);
	effect.strength = strength;
	return;
}

global func RestoreWaterLevels()
{
	// Restore water levels.
	DrawMaterialQuad("Water", 144, 168, 208 + 1, 168, 208 + 1, 304, 144, 304, true);
	for (var x = 216; x <= 280; x++)
		for (var y = 24; y <= 120; y++)
			if (GetMaterial(x, y) == Material("Water"))
				ClearFreeRect(x, y, 1, 1);
	return;
}

global func RemoveWater()
{
	for (var x = 144; x <= 208 + 1; x++)
		for (var y = 168; y <= 304; y++)
			if (GetMaterial(x, y) == Material("Water"))
				ClearFreeRect(x, y, 1, 1);
	return;
}

global func FxIntFixedWindTimer(object target, proplist effect)
{
	SetWind(effect.strength);
	return FX_OK;
}

global func FxIntAlternatingWindTimer(object target, proplist effect, int time)
{
	if (((time / effect.Interval) % 2) == 0)
		SetWindFixed(effect.strength);
	else
		SetWindFixed(0);
	return FX_OK;
}
	
