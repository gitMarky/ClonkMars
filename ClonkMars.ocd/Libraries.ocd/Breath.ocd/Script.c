/**
	Library_Breath

	Implements custom breath system.
	
	If you include this, make sure to call _inherited() in the following functions:
	- Construction
	- DoBreath
	- GetBreath
*/

local library_breath;


private func Construction()
{
	_inherited(...);
	library_breath = {
		Breath = GetMaxBreath(),
		NoBreath = this.NoBreath,
	};
	this.NoBreath = 1; // has no effect yet
	CreateEffect(FxBreathe, 1, 1);
}


public func GetBreath()
{
	_inherited(...);
	return library_breath.Breath;
}


public func DoBreath(int change)
{
	// change it
	var current = GetBreath();
	library_breath.Breath = BoundBy(current + change, 0, this.MaxBreath);
	// cause the engine callback, etc.
	_inherited(change, ...);
	// return the change
	return GetBreath() - current;
}


local FxBreathe = new Effect
{
	Construction = func()
	{
		this.do_deep_breath = false;
	},

	Timer = func(int time)
	{
		if (!Target->GetAlive())
		{
			return FX_Execute_Kill;
		}
		
		// skip handling breath?
		if (Target.library_breath.NoBreath) return FX_OK;

		if (Target->~CanBreathe())
		{
			// do nothing at the moment
			var max_supply = (Target->GetMaxBreath() - Target->GetBreath());
			var take_breath = Target->~TakeBreath(max_supply) ?? max_supply;
			
			if (take_breath > 0)
			{
				Target->DoBreath(take_breath);
				
				// sound effect?
				if (this.do_deep_breath)
				{
					Target->~DeepBreath();
					this.do_deep_breath = false;
				}
			}
		}
		else
		{
			// reduce breath / health
			if (Target->GetBreath() > 0)
			{
				Target->DoBreath(-1);
			}
			else if ((time % 5) == 0)
			{
				Target->DoEnergy(-1);
			}
			
			// needs a deep breath later
			this.do_deep_breath = true;
		}

		return FX_OK;
	},
};
