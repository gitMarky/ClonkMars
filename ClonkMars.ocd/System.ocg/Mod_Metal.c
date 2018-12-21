#appendto Metal

public func GetFuelNeed() { return 0; }

public func GetSubstituteComponent(id component, int amount)
{
	if (component == Ore)
	{
		return [
			{Resource = Earth, Amount = amount * 2}
	        //{Resource = Ashes, Amount = amount * 3}
        ];
	}
}
