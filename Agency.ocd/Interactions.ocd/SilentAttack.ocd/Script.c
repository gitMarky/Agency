/**
	Silent attack

	This character can be attacked silently, from behind.
*/


public func IsInteractable(object by_agent)
{
	return _inherited(...) || CanBeSilentAttacked(by_agent);
}

public func GetInteractionMetaInfo(object by_agent, int index)
{
	return { Description = Format("%s: %s", "$DescSilentAttack$", "$DescEliminate$"), Selected = true };
}

public func Interact(object by_agent)
{
	if (by_agent)
	{
		SetKiller(by_agent->GetOwner());
		Kill();
		return true;
	}
	return false;
}

func CanBeSilentAttacked(object by_agent)
{
	if (GetAlive() && by_agent)
	{
		var distance = ObjectDistance(by_agent, this);
		var direction = by_agent->GetCalcDir();
		var side = Sign(GetX() - by_agent->GetX());
		
	    return (distance > 3)              // Agent must be a bit away
	        && (direction == GetCalcDir()) // Agent and target must be looking in the same direction
		    && (direction == side);        // Agent must be behind the target
	}
	return false;
}
