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
	this->~TriggerOnBreak();
}


func TriggerOnBreak()
{
	RemoveItem(true);
}


func EffectsOnBreak(int x, int y, object user)
{
	// Overwrite as desired
	_inherited(x, y, user, ...);
}


func Hit(int xdir, int ydir)
{
	var self = this; 
	_inherited(xdir, ydir, ...);

	// xdir and ydir are specified in precision 100, default velocity precision is 10
	var hit_speed = Distance(0, 0, xdir / 10, ydir / 10);
	if (self && this.BreakOnHit && hit_speed >= this.BreakOnHit)
	{
		Break();
	}
}
