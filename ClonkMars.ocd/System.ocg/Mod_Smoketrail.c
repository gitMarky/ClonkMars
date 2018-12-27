
// Gives a more dusty feel to the smoke trail
global func FxSmokeTrailStart(object target, proplist e, int temp, int color)
{
	_inherited(target, e, temp, color);

	if (temp) return;

	var alpha = 100;
	e.particles_smoke.R = PV_Linear(255, 64);
	e.particles_smoke.G = PV_Linear(185, 64);
	e.particles_smoke.B = PV_Linear(121, 64);
	e.particles_smoke.Alpha = PV_KeyFrames(0, 0, alpha/4, 200, alpha, 1000, 0);
}
