
/* -- Power Visualization -- */

// Visualizes the power change on an object from before to to.
private func VisualizePowerChange(int old_value, int new_value, bool loss)
{
	// Safety: object must exist.
	if (GetType(this) != C4V_C4Object)
	{
		return FatalError("VisualizePowerChange() is called for a non-existing object.");
	}

	// Don't do anything if old and new value are the same.
	if (old_value == new_value)
	{
		return;
	}

	var before_current = nil;
	var fx = GetEffect("FxVisualPowerChange", this);
	if (!fx)
	{
		fx = CreateEffect(FxVisualPowerChange, 1, 5);
	}
	else
	{
		before_current = fx.current;
	}

	var old_abs = Abs(old_value);
	var new_abs = Abs(new_value);

	fx.max = Max(old_abs, new_abs);
	fx.current = before_current ?? old_abs;
	fx.to = new_abs;

	if (loss)
	{
		fx.back_graphics_name = "Red";
	}
	else 
	{
		fx.back_graphics_name = nil;
	}

	if (new_value < 0) 
	{
		fx.graphics_name = "Yellow";
	}
	else if	(new_value > 0) 
	{
		fx.graphics_name = "Green";
	}
	else // off now
	{
		if (old_value < 0)
		{
			fx.graphics_name = "Yellow";
		}
		else 
		{
			fx.graphics_name = "Green";
		}
	}

	fx->Refresh();
}


local FxVisualPowerChange = new Effect {

	Refresh = func ()
	{
		if (this.bar) 
		{
			this.bar->Close();
		}
		var vis = VIS_Allies | VIS_Owner;
		var controller = Target->GetController();
		
		if (controller == NO_OWNER) 
		{
			vis = VIS_All;
		}
			
		var off_x = -(Target->GetDefCoreVal("Width", "DefCore") * 3) / 8;
		var off_y = Target->GetDefCoreVal("Height", "DefCore") / 2 - 10;
		var bar_properties = {
			size = 1000, 
			bars = this.max / 10, 
			graphics_name = this.graphics_name, 
			back_graphics_name = this.back_graphics_name, 
			image = Icon_Lightbulb, 
			fade_speed = 1	
		};
		
		this.bar = Target->CreateProgressBar(GUI_BarProgressBar, this.max, this.current, 35, controller, {x = off_x, y = off_y}, vis, bar_properties);

		// Appear on a GUI level in front of other objects, e.g. trees.
		this.bar->SetPlane(1010);
	},


	Timer = func ()
	{
		if (!this.bar) 
		{
			return FX_Execute_Kill;
		}
		if (this.current == this.to) 
		{
			return FX_OK;
		}
		
		if (this.to < this.current) 
		{
			this.current = Max(this.current - 15, this.to);
		}
		else 
		{
			this.current = Min(this.current + 15, this.to);
		}
	
		this.bar->SetValue(this.current);
		return FX_OK;
	}
};

