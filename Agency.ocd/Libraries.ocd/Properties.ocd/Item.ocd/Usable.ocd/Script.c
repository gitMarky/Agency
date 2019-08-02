/**
	Provides basic using logic for items.
	
	Objects that include this should add an _inherited() call in RejectUse, such as:
	
	func RejectUse(object user)
	{
		if (_inherited(user, ...))
		{
			return true;
		}
		else
		{
			return [further conditions];
		}
	}
	
	or
	
	func RejectUse(object user)
	{
		return _inherited(user, ...) || [further conditions];
	}
 */


func RejectUse(object user)
{
	if (user->~GetHandItem() == this)
	{
		return _inherited(user, ...);
	}
	else
	{
		return true;
	}
}

