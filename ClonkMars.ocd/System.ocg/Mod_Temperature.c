#appendto Temperature


public func GetMaterialTemperature(int material)
{
	if (material == Material("DuroLava"))
	{
		return 500;
	}
	else if (material == Material("Lava"))
	{
	 	return 250;
	}
	else if (material == Material("Snow") || material == Material("Ice"))
	{
		return Min(GetTemperature(), 0);
	}
	else if (material == Material("Sky"))
	{
		return GetTemperature();
	}
	else
	{
		var temperature = GetTemperature();

		var is_solid = GetMaterialVal("Density", "Material", material) > 30;
		var is_soil = GetMaterialVal("Soil", "Material", material) > 0;
		var is_diggable = GetMaterialVal("DigFree", "Material", material) > 0;

		var percent = 50;

		if (is_soil) percent += 10;
		if (is_diggable) percent += 10;
		if (!is_solid) percent += 40;

		return percent * temperature / 100;
	}
}
