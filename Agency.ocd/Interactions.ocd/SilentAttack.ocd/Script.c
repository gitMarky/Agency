/**
	Silent attack

	This character can be attacked silently, from behind.
*/

#include Property_Interactable

public func GetInteractions(object by_agent)
{
	var interactions = _inherited(by_agent, ...) ?? [];
	PushBack(interactions, new Interaction
	{
		Target = this,
		Name = "$DescSilentAttack$",
		Desc = "$DescEliminate$",
		Condition = this.CanBeSilentAttacked,
		Execute = this.GetSilentAttacked,
	});
	return interactions;
}

func GetSilentAttacked(object by_agent)
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
