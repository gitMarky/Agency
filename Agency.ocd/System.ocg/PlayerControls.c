/**
	Use this for redefining certain button behaviour.
 */


global func PlayerControl(int player, int control, id spec_id, int x, int y, int strength, bool repeat, int status)
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
	return ctrl == CON_Interact
	    || ctrl == CON_PickUp
		|| ctrl == CON_ExtraSlot;
}
