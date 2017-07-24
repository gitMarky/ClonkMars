#include Library_Breath
#include Library_BreatheAir
#include Library_BreatheRespirator
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


public func GetRespirator()
{
	return FindObject(Find_Container(this), Find_Func("IsRespiratorFor", this));
}
