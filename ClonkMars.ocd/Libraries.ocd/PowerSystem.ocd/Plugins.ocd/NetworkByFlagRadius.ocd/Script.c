
public func GetPowerLink(object for_obj)
{
	return GetFlagpoleForPosition(for_obj->GetX() - GetX(), for_obj->GetY() - GetY());
}
