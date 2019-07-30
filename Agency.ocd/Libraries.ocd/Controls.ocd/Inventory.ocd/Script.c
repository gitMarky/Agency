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
		if (Contents() && !GetEffect(FxInventorySwitchItem.Name, this))
		{
			CreateEffect(FxInventorySwitchItem, 1, 1, in_hands, selected);
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


// Shows an effect when you picked up an item.
func TryToCollect(object item)
{
	if (!GetEffect(FxPickUpItem.Name, this))
	{
		CreateEffect(FxPickUpItem, 1, 10, item);
	}
}


local FxPickUpItem = new Effect
{
	Name = "FxPickUpItem",

	Start = func (int temporary, object item)
	{
		if (!temporary)
		{
			this.Item = item;
			this.Target->~DoHolsterHandItem(true);
			this.Target->~PlayAnimation("ThrowArms", CLONK_ANIM_SLOT_Arms, Anim_Linear(1000, 0, 1500, 50, ANIM_Remove), Anim_Linear(0, 0, 1000, 10, ANIM_Remove));
		}
	},

	Timer = func ()
	{
		this.Target->~TryToPickUp(this.Item);
		return FX_Execute_Kill;
	},
};


func TryToPickUp(object item)
{
	if (!item)
	{
		return false;
	}

	// Remember stuff for a possible message - the item might have removed itself later.
	var x = item->GetX();
	var y = item->GetY();
	var name = item->GetName();
	
	// Otherwise, try to collect the item myself.
	if (item && !item->Contained())
	{
		Collect(item);
	}

	// If anything happened, assume collection.
	if (!item || item->Contained())
	{
		var message = CreateObject(FloatingMessage, AbsX(x), AbsY(y), GetOwner());
		message.Visibility = VIS_Owner;
		message->SetMessage(name);
		message->SetYDir(-10);
		message->FadeOut(1, 20);
		return true;
	}
	return false;
}


/* Backpack control */
func Selected(object mnu, object mnu_item)
{
	var backpack_index = mnu_item->GetExtraData();
	var hands_index = 0;
	// Update menu
	var show_new_item = this->GetItem(hands_index);
	mnu_item->SetSymbol(show_new_item);
	// swap index with backpack index
	this->Switch2Items(hands_index, backpack_index);
}


local FxInventorySwitchItem = new Effect
{
	Name = "FxInventorySwitchItem",

	Start = func (int temporary, object stash, object draw)
	{
		if (!temporary)
		{
			this.StashItem = stash; // Stashes this item away
			this.DrawItem = draw;   // Draws this item
			var holster = "Holster";
			var length = this.Target->GetAnimationLength(holster);
			this.AnimTime = 30;
			this.Anim = this.Target->PlayAnimation(holster, CLONK_ANIM_SLOT_Arms, Anim_Linear(length, length, 0, this.AnimTime, ANIM_Remove), Anim_Linear(0, 0, 1000, 10, ANIM_Remove));
		}
	},
	
	Timer = func (int time)
	{
		if (this.Target->ReadyToAction())
		{
			// Switch items here
			if (time == this.AnimTime / 2)
			{
				this.Target->DoHolsterHandItem(true);
				this.Target->SetHandItem(this.DrawItem);
				if (this.DrawItem)
				{
					this.Target->SetActiveItem(this.DrawItem);
				}
				this.Target->UpdateAttach();
			}

			if (time < this.AnimTime)
			{
				return FX_OK;
			}
		}
		return FX_Execute_Kill;
	},
};

