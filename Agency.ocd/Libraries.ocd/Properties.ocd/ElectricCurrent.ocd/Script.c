/**
	Marks an object being able to electrify.
	
	While the object is electrified it can connect to one hazard
	and issue callbacks to it (Property_ElectricHazard):
	- EffectsOnElectricCurrent(object source) for effects
	- TriggerOnElectricCurrent(object source) for modifications of the game world
 */

public func HasElectricCurrent(){ return true; }


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
		if (!this.Hazard)
		{
			this.Hazard = this.Target->FindObject(Find_Or(Find_AtPoint(), Find_Distance(5)), Find_NoContainer(), Find_Func("CanBeElectrified"));
		}

		// Create effects at the target, and if we power a hazard
		// then we forward effect calls and triggering calls to it
		this.Target->~EffectsOnElectricCurrent();
		if (this.Hazard)
		{
			this.Hazard->~EffectsOnElectricCurrent(this.Target); // Create effect
			this.Hazard->~TriggerOnElectricCurrent(this.Target); // Logic for electrocuting victims
		}
	},
};
