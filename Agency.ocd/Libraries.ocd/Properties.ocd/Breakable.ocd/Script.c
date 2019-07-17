/**
	Marks an object as breakable.
	
	The following properties are referenced by other scripts:
	
		.BreakOnMelee  = bool, breaks when you do a successful melee attack with it
		.BreakOnThrow  = bool, breaks when thrown at someone
		.BreakOnHit    = bool, breaks when hitting the landscape with this velocity (precision = 10)
		
	These are not defined here, however.
 */

public func IsBreakable(){ return true; }


public func Break(bool ignore_effects)
{
	if (!ignore_effects)
	{
		this->~EffectsOnBreak();
	}
	RemoveItem(true);
}


func EffectsOnBreak(int x, int y, object user)
{
	// Overwrite as desired
	_inherited(x, y, user, ...);
}
