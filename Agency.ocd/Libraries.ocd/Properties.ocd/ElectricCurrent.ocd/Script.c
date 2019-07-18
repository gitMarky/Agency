/**
	Marks an object being able to electrify.
 */

public func HasElectricCurrent(){ return this.ElectricCurrent; }
public func CanBeElectrified(){  return !this.ElectricCurrent; }


local ElectricCurrent = true; // You can overwrite this, to switch between the modes. False = object can be electrified passively


public func Electrify(object hazard)
{
	var current = GetEffect(FxElectricCurrent.Name, this);
	if (current == nil)
	{
		current = CreateEffect(FxElectricCurrent, 100, 10); // Timer of 10 is OK, should be enough to electrocute
		if (hazard)
		{
			current.Hazard = hazard;
		}
	}
}


public func DeElectrify()
{
	RemoveEffect(FxElectricCurrent.Name, this);
}


local FxElectricCurrent = new Effect
{
	Name = "ElectricCurrent",

	Timer = func ()
	{
		// Look for objects that can be electrified, e.g. a puddle
		if (this.Target->~HasElectricCurrent() && !this.Hazard)
		{
			this.Hazard = this.Target->FindObject(Find_Or(Find_AtPoint(), Find_Distance(5)), Find_NoContainer(), Find_Func("CanBeElectrified"));
		}

		// Create effects at the target, and if we power a hazard
		// then we forward effect calls and triggering calls to it
		this.Target->~EffectsOnElectricCurrent();
		if (this.Hazard)
		{
			this.Hazard->~EffectsOnElectricCurrent(); // Create effect
			this.Hazard->~TriggerOnElectricCurrent(); // Logic for electrocuting victims
		}
	},
};
