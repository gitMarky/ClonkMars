
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
