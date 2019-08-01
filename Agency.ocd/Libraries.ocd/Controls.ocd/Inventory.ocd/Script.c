/**
	Handles the interaction with the inventory.

	Copied from Objects.ocd
*/


public func ObjectControl(int plr, int ctrl, int x, int y, int strength, bool repeat, int status)
{
	if (!this)
	{
		return false;
	}
	
	// Holster or unholster
	if (ctrl == CON_Holster && status == CONS_Down && this->ReadyToAction())
	{
		var in_hands = this->GetHandItem();
		var selected = this->GetActiveItem();
		if (selected == in_hands) // Just holster then
		{
			selected = nil;
		}
		if (Contents() && !GetEffect(this.FxInventorySwitchItem.Name, this)) // TODO: Move
		{
			CreateEffect(this.FxInventorySwitchItem, 1, 1, in_hands, selected); // TODO: Move
		}
		return true;
	}

	// shift inventory
	var inventory_shift = 0;
	if (ctrl == CON_InventoryShiftForward)
	{
		inventory_shift = 1;
	}
	else if (ctrl == CON_InventoryShiftBackward)
	{
		inventory_shift = -1;
	}
	
	if (inventory_shift)
	{
		var current = (this->~GetHandItemPos(0) + inventory_shift) % this.MaxContentsCount;
		if (current < 0) current = this.MaxContentsCount + current;
		this->~SetHandItemPos(0, current);
		return true;
	}

	return inherited(plr, ctrl, x, y, strength, repeat, status, ...);
}
