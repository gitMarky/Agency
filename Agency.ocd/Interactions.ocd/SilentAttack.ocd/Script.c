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
		Condition = this.CanBeSilentAttacked,
		Execute = this.GetSilentAttacked,
	});
	return interactions;
}

func GetSilentAttacked(object by_agent, proplist interaction)
{
	if (by_agent)
	{
		var action = GetSilentAttackAction(by_agent, interaction);
		action.Caller->Call(action.Action, this, by_agent);
		return true;
	}
	return false;
}

func CanBeSilentAttacked(object by_agent, proplist interaction)
{
	AssertNotNil(by_agent);
	AssertNotNil(interaction);

	interaction.Desc = GetSilentAttackAction(by_agent, interaction).Desc;

	if (GetAlive())
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

func GetSilentAttackAction(object by_agent, proplist interaction)
{
	AssertNotNil(by_agent);
	AssertNotNil(interaction);
	
	// Default action
	var description = "$DescSubdue$";
	var damage = nil;
	var caller = this;
	var fn = nil;

	// Override by item?
	var item = by_agent->~GetHandItem();
	if (item && item->~IsSilentAttackWeapon())
	{
		caller = item;
		fn = item.DoSilentAttack;

		if (item->~CausesLethalDamage())
		{
			description = "$DescEliminate$";
			damage = Property_Damage_Lethal;
		}
		else if (item->~CausesStunningDamage())
		{
			description = "$DescSubdue$";
			damage = Property_Damage_Stunning;
		}
	}
	return {
		Caller = caller,
		Action = fn,
		Desc = description,
		Damage = damage
	};
}
