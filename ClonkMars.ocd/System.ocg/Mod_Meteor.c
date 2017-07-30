#appendto Meteor

// gives a more dusty atmosphere to the meteor
public func OnAfterLaunch()
{
	_inherited(...);
	
	var fx = GetEffect("IntMeteor", this);
	
	if (fx)
	{
		fx.smoketrail.Alpha = PV_KeyFrames(1000, 0, 0, 30, 30, 1000, 0);
		fx.smoketrail.R = 255;
		fx.smoketrail.G = 185;
		fx.smoketrail.B = 121;
		fx.frontburn.Size = PV_Linear(1, 3);
	}
}


// modifies the meteor spawn interval. Default value is 20 frames
public func SetInterval(int interval, id spawn_id)
{
	var fx = FindMeteoriteEffectFor(spawn_id);
	if (fx)
	{
	 	fx.Interval = interval ?? 20;
	}
	else
	{
		FatalError("Meteor effect has to exist before calling this function");
	}

	return true;
}
