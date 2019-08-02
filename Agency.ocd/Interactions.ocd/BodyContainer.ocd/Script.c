/**
	Container

	This objects functions as a container for bodies. You can hide some inside, or hide yourself.
*/

#include Property_Interactable

local MaxContentsCount = 2;

public func GetInteractions(object by_agent)
{
	var interactions = _inherited(by_agent, ...) ?? [];
	PushBack(interactions, new Interaction
	{
		Target = this,
		Name = "$DescEnter$",
		Desc = "",
		Condition = this.CanEnterHere,
		Execute = this.DoEnterHere,
	});
	PushBack(interactions, new Interaction
	{
		Target = this,
		Name = "$DescDump$",
		Desc = "",
		Condition = this.CanDumpBody,
		Execute = this.DoDumpBody,
	});
	return interactions;
}

func DoDumpBody(object by_agent, proplist interaction)
{
	return false;
}

func CanDumpBody(object by_agent, proplist interaction)
{
	AssertNotNil(by_agent);
	AssertNotNil(interaction);
	return HasCapacity() && GetDumpableBody(by_agent) != nil;
}

func DoEnterHere(object by_agent, proplist interaction)
{
	return false;
}

func CanEnterHere(object by_agent, proplist interaction)
{
	AssertNotNil(by_agent);
	AssertNotNil(interaction);
	return HasCapacity() && GetDumpableBody(by_agent) == nil;
}

func HasCapacity()
{
	return ContentsCount() < this.MaxContentsCount;
}

func GetDumpableBody(object by_agent)
{
	return nil;
}
