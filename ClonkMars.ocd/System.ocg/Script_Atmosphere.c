/**
	Atmosphere

	@author Marky
*/

static atmosphere_oxygen;


/**
 * Gets the amount of oxygen in the atmosphere, as per mil.
 */
global func GetOxygenInAtmosphere()
{
	return atmosphere_oxygen ?? 1000;
}


/**
 * Sets the amount of oxygen in the atmosphere.
 *
 * @par amount the new value, per mil. Will be bounded to [0; 1000];
 */
global func SetOxygenInAtmosphere(int amount)
{
	atmosphere_oxygen = BoundBy(amount ?? 1000, 0, 1000);
}


/**
 * Changes the amount of oxygen in the atmosphere.
 *
 * @par change the change to the value, per mil. The final value will be bounded to [0; 1000];
 */
global func DoOxygenInAtmosphere(int change)
{
	SetOxygenInAtmosphere(GetOxygenInAtmosphere() + change);
}
