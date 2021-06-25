/**
	Use this for redefining certain button behaviour.
 */


global func PlayerControl(proplist player, int control, id spec_id, int x, int y, int strength, bool repeat, int status)
{
	// Map the CON_CMC_UseAlt key to the regular CON_UseAlt
	if (control == CON_Throw)
	{
		control = CON_UseAlt;
	}

	return _inherited(player, control, spec_id, x, y, strength, repeat, status);
}


// Control has the goal of interacting with some other object (Interaction, Grabbing, Entering,...).
global func IsInteractionControl(int ctrl)
{
	return IsValueInArray(GetInteractionControls(), ctrl);
}

global func GetInteractionControls()
{
	return [
		CON_Interact,
		CON_Attack,
		CON_Disguise,
		CON_DragBody,
		CON_ExtraSlot,
		CON_Manipulation,
		CON_PickUp
	];
}
