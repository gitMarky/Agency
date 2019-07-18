/**
	Marks an object being able to electrify.
 */

public func HasElectricCurrent(){ return true; }


public func Electrify()
{
	var current = GetEffect(FxElectricCurrent.Name, this);
	if (current == nil)
	{
		current = CreateEffect(FxElectricCurrent, 100, 10); // Timer of 10 is OK, should be enough to electrocute
	}
}


public func DeElectrify()
{
	RemoveEffect(FxElectricCurrent.Name, this);
}


local FxElectricCurrent = new Effect
{
	Name = "ElectricCurrent",

	// TODO: Timer has to look for objects that can be electrified, e.g. a puddle, and the puddle object will kill anyone with its own logic!
	Timer = func ()
	{
		this.Target->~EffectsOnElectricCurrent();
	},
};
