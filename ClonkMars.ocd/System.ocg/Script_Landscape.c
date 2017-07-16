
global func GetHorizon(int x)
{
	var y;
	while(!GBackSemiSolid(x, AbsY(y)) && y < LandscapeHeight())
	{
		y += 1;
	}
	return y;
}
