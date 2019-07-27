/**
	Container

	This item functions as a container.
*/

#include Library_HasExtraSlot
#include Property_Interactable

public func GetInteractions(object by_agent)
{
	var interactions = _inherited(by_agent, ...) ?? [];
	PushBack(interactions, new Interaction
	{
		Target = this,
		Name = "$DescConceal$",
		Desc = "",
		Condition = this.CanConcealItem,
		Execute = this.DoConcealItem,
	});
	PushBack(interactions, new Interaction
	{
		Target = this,
		Name = "$DescRetrieve$",
		Desc = "",
		Condition = this.CanRetrieveItem,
		Execute = this.DoRetrieveItem,
	});
	return interactions;
}

func DoConcealItem(object by_agent, proplist interaction)
{
	return false;
}

func CanConcealItem(object by_agent, proplist interaction)
{
	AssertNotNil(by_agent);
	AssertNotNil(interaction);
	return !Contained() && ContentsCount() < this.MaxContentsCount;
}

func DoRetrieveItem(object by_agent, proplist interaction)
{
	return false;
}

func CanRetrieveItem(object by_agent, proplist interaction)
{
	AssertNotNil(by_agent);
	AssertNotNil(interaction);
	return !Contained() && Contents();
}
