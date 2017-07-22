
global func CreateObjects(proplist objects, int x, int y, int owner)
{
	owner = owner ?? (-1);
	for (var entry in GetProperties(objects))
	{
		var type = GetDefinition(entry);
		if (type)
		{
			var amount = objects[entry];
			for (var i = 0; i < amount; ++i)
			{
				CreateObject(type, x, y, owner);
			}
		}
	}
}
