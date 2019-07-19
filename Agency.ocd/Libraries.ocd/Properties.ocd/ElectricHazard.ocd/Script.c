/**
	Marks an object as an electric hazard.
	
	Gets callbacks from Property_ElectricCurrent:
	- EffectsOnElectricCurrent(object source) for effects
	- TriggerOnElectricCurrent(object source) for modifications of the game world
 */

public func CanBeElectrified(){  return true; }

