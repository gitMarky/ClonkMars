
/**
 * Finds out whether the way to the sky is free, if you follow
 * a particular angle from the starting point.
 *
 * @par angle the angle
 * @par value this value will be returned if the angle is free. Default is 1.
 * @par x the x coordinates. Object-local.
 * @par y the y coordinates. Object-local.
 *
 * @return int the {@code value} is returned if the way is free.
 */
global func GetAngleFree(int angle, int value, int x, int y)
{
	x = x ?? GetX();
	y = y ?? GetY();
	value = value ?? 1;

	// Angle is definitely not free if the starting point is covered
	if (GBackSemiSolid(AbsX(x), AbsY(y))) return 0;
	
	var dx = +Sin(angle, 50);
	var dy = -Cos(angle, 50);
	
	var pos_x, pos_y;
	while (true)
	{
		pos_x = x + dx;
		pos_y = y + dy;
		
		// Workaround für 1px-wall at the side
		if (pos_x < 0 && GameCall("LeftClosed") && GetMaterial(AbsX(0), AbsY(pos_y)) == -1)
		{
			return value;
		}
		if (pos_x > LandscapeWidth() && GameCall("RightClosed") && GetMaterial(AbsX(LandscapeWidth() - 1), AbsY(pos_y)) == -1)
		{
			return value;
		}
		
		// Path segment is free and there is no liquid? Continue
		if (PathFree(pos_x, pos_y, x, y) && !GBackLiquid(AbsX(pos_x), AbsY(pos_y)))
		{
			// Reached the border? It's free!
			if (pos_x < 0 || pos_x > LandscapeWidth() || pos_y < 0)
			{
				return value;
			}
		}
		else // Path segment is blocked or there is liquid? Not free!!!
		{
			return 0;
		}
		
		// Go to the next path segment
		x = pos_x;
		y = pos_y;
	}
}
