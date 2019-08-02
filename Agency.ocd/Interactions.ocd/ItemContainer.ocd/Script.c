/**
	Container

	This item functions as a container.
	
	The property
	- this.ContainerControl defines which control is used for concealing/retrieving items
*/

#include Library_HasExtraSlot
#include Property_Interactable

local MaxContentsCount = 1;

public func GetInteractions(object by_agent)
{
	var interactions = _inherited(by_agent, ...) ?? [];
	PushBack(interactions, new Interaction
	{
		Target = this,
		Name = "$DescConceal$",
		Desc = "",
		Control = this.ContainerControl ?? CON_Interact,
		Condition = this.CanConcealItem,
		Execute = this.DoConcealItem,
	});
	PushBack(interactions, new Interaction
	{
		Target = this,
		Name = "$DescRetrieve$",
		Desc = "",
		Control = this.ContainerControl ?? CON_Interact,
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
