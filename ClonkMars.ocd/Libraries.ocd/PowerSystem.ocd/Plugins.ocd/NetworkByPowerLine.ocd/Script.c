
public func GetPowerLink(object for_obj)
{
	return FindObject(Find_Func("IsPowerLine"), Find_Func("IsConnectedTo", for_obj));
}
