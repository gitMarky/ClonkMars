
global func Particles_Glass()
{
	return
	{
		Size = PV_Random(1, 3),
		Phase = PV_Linear(0, 3),
		Alpha = PV_KeyFrames(0, 0, 255, 900, 255, 1000, 0),
		CollisionVertex = 500,
		OnCollision = PC_Stop(),
		ForceX = PV_Wind(20),
		ForceY = PV_Gravity(100),
		Rotation = PV_Direction(PV_Random(750, 1250)),
		BlitMode = GFX_BLIT_Additive,
	};
}


global func Particles_DustAdvanced(color, int size, int alpha)
{
	if (GetType(color) == C4V_String)
	{
		return Particles_DustAdvanced(GetAverageTextureColor(color), size, alpha);
	}
	else if (GetType(color) == C4V_Int)
	{
		return Particles_DustAdvanced(SplitRGBaValue(color), size, alpha);
	}
	else if (GetType(color) != C4V_PropList)
	{
		FatalError(Format("Unsupported parameter type: %v - expected int, proplist, or string", GetType(color)));
	}

	var default_size = 12;
	size = size ?? default_size;
	return
	{
		Prototype = Particles_Dust(),
		Alpha = PV_KeyFrames(0, 0, 0, 250, alpha ?? 60, 1000, 0),
		R = color.R, B = color.B, G = color.G,
		Size = PV_KeyFrames(0, 0, 5 * size / default_size, 100, 12 * size / default_size, 1000, 7 * size / default_size),
	};
}