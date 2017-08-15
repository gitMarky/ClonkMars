#include Library_Breath
#include Library_BreatheAir
#include Library_BreatheOxygenSupplier
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


public func OnBreathe()
{
	// refill respirators
	var supplier = GetOxygenSupplier();
	var supplier_refill = 0;
	if (supplier) supplier_refill = supplier->GetOxygenRefillRate();
	
	for (var respirator in FindObjects(Find_Container(this), Find_Func("IsRespirator")))
	{
		var refill = Min(respirator->GetMaxOxygen() - respirator->GetOxygen(), supplier_refill);

		var taken = supplier->DoOxygen(-refill);
		var given = respirator->DoOxygen(-taken);

		supplier->DoOxygen(given - taken); // just in case, should yield 0 most of the time
	}
}


public func GetRespirator()
{
	return FindObject(Find_Container(this), Find_Func("IsRespiratorFor", this));
}


public func GetOxygenSupplier()
{
	var container = Contained();
	if (container && container->~IsOxygenSupplier())
	{
		return container;
	}
	return nil;
}
