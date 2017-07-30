#appendto Shovel


func GetDigSpeed(object clonk)
{
	return 3 * clonk.ActMap.Dig.Speed / 2;
}


public func ControlUseHolding(object clonk, int x, int y)
{
	var dig_effect = GetEffect("ShovelDig", clonk);
	if (dig_effect)
	{
		dig_effect.dig_x = x;
		dig_effect.dig_y = y;		
		dig_effect.dig_angle = Angle(0, 0, x, y);	
		
		// lock angle in 5° increments
		var diff = dig_effect.dig_angle % 10;
		var target;
		if (diff > 0 && diff < 3)
		{
			target = 0;
		}
		else if (diff < 8)
		{
			target = 5;
		}
		else
		{
			target = 10;
		}

		dig_effect.dig_angle += target - diff;
	}
	return true;
}