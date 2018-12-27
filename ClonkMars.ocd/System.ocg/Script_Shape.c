/**
	Shape

	Some helper functions for shapes

	@author Marky
*/

global func GetShapeRectangle()
{
	AssertObjectContext();

	return Shape->Rectangle(GetDefOffset(0), GetDefOffset(1), GetDefWidth(), GetDefHeight());
}
