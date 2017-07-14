#include Library_Breath
#include Library_BreatheAir
#appendto Clonk


public func BreatheAt()
{
	var head_vertex = 1;
	var y = 0;
	if (InLiquid())
	{
		y = -5;
	}
	
	return { X = GetVertex(head_vertex, 0),
	         Y = GetVertex(head_vertex, 1) + y};
}
