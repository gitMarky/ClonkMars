
global func Smoke(int x, int y, int level, int color, bool heavy)
{
	level = level ?? 10;
	var particles = Particles_Smoke(heavy);
	if (color)
	{
		color = SplitRGBaValue(color);
		particles.Alpha = PV_Linear(color.Alpha, 0);
		particles.R = color.R;
		particles.G = color.G;
		particles.B = color.B;
	}
	else
	{
		particles.Alpha = PV_Linear(55 + BoundBy(GetOxygenInAtmosphere(), 0, 1000)/5, 0);
	}
	particles.Size = PV_Linear(PV_Random(level/2, level), PV_Random(2 * level, 3 * level));
	CreateParticle("Smoke", x, y, PV_Random(-level/3, level/3), PV_Random(-level/2, -level/3), PV_Random(level * 2, level * 10), particles, BoundBy(level/5, 3, 20));
}
