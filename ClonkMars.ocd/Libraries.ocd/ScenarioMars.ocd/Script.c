
func Initialize()
{
	_inherited(...);
    Sky_Mars();
    Scenario_Gravity();
}


private func Scenario_Gravity()
{
    SetGravity(8); // 37% of default value 20, rounded up
}
