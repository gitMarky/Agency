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
		if (action == Property_Damage_Lethal)
		{
			SetKiller(by_agent->GetOwner());
			Kill();
		}
		return true;
	}
	return false;
}

func CanBeSilentAttacked(object by_agent, proplist interaction)
{
	AssertNotNil(by_agent);
	AssertNotNil(interaction);

	var action = GetSilentAttackAction(by_agent, interaction);
	if (action == Property_Damage_Lethal)
	{
		interaction.Desc = "$DescEliminate$";
	}
	else if (action == Property_Damage_Stunning)
	{
		interaction.Desc = "$DescPacify$";
	}
	else
	{
		interaction.Desc = "$DescSubdue$";
	}

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
	
	var item = by_agent->~GetActiveItem();
	if (item)
	{
		if (item->~CausesLethalDamage())
		{
			return Property_Damage_Lethal;
		}
		else if (item->~CausesStunningDamage())
		{
			return Property_Damage_Stunning;
		}
	}
	return nil; // Default action
}
