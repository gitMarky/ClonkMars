#include Library_Structure
#include Library_DamageControl

/* -- Structure properties -- */

public func IsBuildableByConKit() { return true; }
public func GetBasementID(){ return Structure_Basement; }
public func GetBasementWidth(){ return GetID()->GetDefWidth(); }
