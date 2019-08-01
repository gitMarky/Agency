/**
	Pick Up

	This item can be collected.
*/

#include Property_Interactable

public func GetInteractions(object by_agent)
{
	var interactions = _inherited(by_agent, ...) ?? [];
	PushBack(interactions, new Interaction
	{
		Target = this,
		Name = "$DescPickUp$",
		Control = CON_PickUp,
		Condition = this.CanBePickedUp,
		Execute = this.GetPickedUp,
	});
	return interactions;
}

func GetPickedUp(object by_agent, proplist interaction)
{
	if (by_agent)
	{
		by_agent->~TryToCollect(this);
		return true;
	}
	return false;
}

func CanBePickedUp(object by_agent, proplist interaction)
{
	AssertNotNil(by_agent);
	AssertNotNil(interaction);
	
	var info = by_agent->~GetPickUpInfo(this);
	if (info && info.DropL || info.DropR)
	{
		interaction.Name = "$DescSwap$";
	}

	interaction.Desc = this->~GetName();
	return this.Collectible && !Contained();
}
