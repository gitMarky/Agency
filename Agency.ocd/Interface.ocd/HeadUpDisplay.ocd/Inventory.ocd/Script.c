/**
	Displays inventory slots and extra information.
	
	Copied from Objects.ocd, will be modified from there
*/

/*
	inventory_slot contains an array of proplists with the following attributes:
		ID: submenu ID. Unique in combination with the target == this
		obj: last object that was shown here
		hand: bool, whether select with a hand
		quick: bool, whether this is the quick switch slot
*/

local inventory_slot;
local inventory_gui_menu;
local inventory_gui_id;

/* GUI creation */

// For custom HUD graphics overload the following function as deemed fit.

func AssembleInventoryButton()
{
	// The gui already exists, only update it with a new submenu
	var pos = CalculateButtonPosition();

	return
	{
		Target = this,
		Style = GUI_NoCrop,
		Symbol = Icon_Menu_Circle,
		ID = 1,
		Left = pos.Left, Top = pos.Top, Right = pos.Right, Bottom = pos.Bottom,
		count =
		{
			ID = 1000,
			Style = GUI_TextRight | GUI_TextBottom,
			Text = nil,
			Priority = 2
		},
		// Prepare (invisible) extra-slot display circle.
		extra_slot =
		{
			Top = ToEmString(GUI_Controller_InventoryBar_IconSize),
			Bottom = ToEmString(GUI_Controller_InventoryBar_IconSize + GUI_Controller_InventoryBar_IconSize/2),
			Style = GUI_TextLeft,
			Text = nil,
			symbol =// used to display an infinity sign if necessary (Icon_Number)
			{
				Right = ToEmString(GUI_Controller_InventoryBar_IconSize/2),
				GraphicsName = "Inf",
			},
			circle =// shows the item in the extra slot
			{
				Left = ToEmString(GUI_Controller_InventoryBar_IconSize/2),
				Symbol = nil,
				symbol = {}
			}
		},
		overlay = // Custom inventory overlays can be shown here.
		{
			ID = 2000,
		}
	};
}

/* Creation / Destruction */

func Construction()
{
	inventory_slot = 
	{
		empty = true,
		item = nil,
		last_count = 0,
		had_custom_overlay = false
		
	};

	inventory_gui_menu =
	{
		Target = this,
		Player = NO_OWNER, // will be shown once a gui update occurs
		Style = GUI_Multiple | GUI_IgnoreMouse | GUI_NoCrop,
		Button = AssembleInventoryButton(),
	};
	inventory_gui_id = GuiOpen(inventory_gui_menu);

	return _inherited(...);
}

func Destruction()
{
	GuiClose(inventory_gui_id);

	_inherited(...);
}

/* --- Callbacks --- */

func OnCrewDisabled(object clonk)
{
	ScheduleUpdateInventory();

	return _inherited(clonk, ...);
}

func OnCrewEnabled(object clonk)
{
	ScheduleUpdateInventory();

	return _inherited(clonk, ...);
}

func OnCrewSelection(object clonk, bool deselect)
{
	ScheduleUpdateInventory();

	return _inherited(clonk, deselect, ...);
}

// call from HUDAdapter (Clonk)
func OnSlotObjectChanged(int slot)
{
	// refresh inventory
	ScheduleUpdateInventory();

	return _inherited(slot, ...);
}

// Updates the Inventory in 1 frame
func ScheduleUpdateInventory()
{
	if (!GetEffect("UpdateInventory", this))
	{
		AddEffect("UpdateInventory", this, 1, 1, this);
	}
}

func FxUpdateInventoryTimer()
{
	UpdateInventory();
	return FX_Execute_Kill;
}

/* --- Display --- */

func UpdateInventory()
{
	// only display if we have a clonk and it's not disabled
	var cursor = GetCursor(GetOwner());
	if (!cursor || !cursor->GetCrewEnabled())
	{
		if (inventory_gui_menu.Player != NO_OWNER)
		{
			inventory_gui_menu.Player = NO_OWNER;
			GuiUpdate(inventory_gui_menu, inventory_gui_id);
		}
		return;
	}

	// Make sure inventory is visible
	if (inventory_gui_menu.Player != GetOwner())
	{
		inventory_gui_menu.Player = GetOwner();
		GuiUpdate(inventory_gui_menu, inventory_gui_id);
	}

	// update inventory-slots

	var item = cursor->~GetHandItem();
	if (!item)
	{
		item = cursor->~GetActiveItem();
	}
	// Enable objects to provide a custom overlay for the icon slot.
	// This could e.g. be used by special scenarios or third-party mods.
	var custom_overlay = nil;
	// For stacked objects, we will have multiple virtual objects in one slot.
	var stack_count = nil;
	if (item)
	{
		stack_count = item->~GetStackCount();
		custom_overlay = item->~GetInventoryIconOverlay();
	}
	var has_extra_slot = item && item->~HasExtraSlot();
	if ((!!item == inventory_slot.empty) || (item != inventory_slot.item) || (stack_count != inventory_slot.last_count) || has_extra_slot || inventory_slot.had_custom_overlay || custom_overlay)
	{
		// Hide or show extra-slot display?
		var extra_slot_player = NO_OWNER;
		var extra_symbol = nil;
		var extra_symbol_stack_count = nil;
		var contents = nil;
		var extra_slot_background_symbol = nil;
		if (has_extra_slot)
		{
			// Show!
			contents = item->Contents(0);
			if (contents)
			{
				extra_symbol = contents;
				// Stack count: either actual stack count or stacked object count.
				extra_symbol_stack_count = contents->~GetStackCount();
				if (extra_symbol_stack_count == nil)
				{
				    // Stack count fallback to actually stacked objects
				    extra_symbol_stack_count = item->ContentsCount(contents->GetID());
				}
			}
			extra_slot_player = GetOwner();
			extra_slot_background_symbol = Icon_Menu_Circle;
			// And attach tracker..
			var i = 0, e = nil;
			var found = false;
			while (e = GetEffect("ExtraSlotUpdater", item, i++))
			{
				if (e.CommandTarget != this) continue;
				found = true;
				break;
			}
			if (!found) AddEffect("ExtraSlotUpdater", item, 1, 30 + Random(60), this);
		}
		// What to display in the extra slot?
		var extra_text = nil, number_symbol = nil;
		if (extra_symbol && extra_symbol_stack_count)
		{
			if (contents->~IsInfiniteStackCount())
				number_symbol = Icon_Number;
			else extra_text = Format("%dx", extra_symbol_stack_count);
		}
		
		// Close a possible lingering custom overlay for that slot.
		var custom_overlay_id = 2000;
		GuiClose(inventory_gui_id, custom_overlay_id, nil);
		
		// Compose the update!
		var update =
		{
			slot = { Symbol = item },
			extra_slot =
			{
				Player = extra_slot_player,
				Text = extra_text,
				symbol =
				{
					Symbol = number_symbol
				},
				circle =
				{
					Symbol = extra_slot_background_symbol,
					symbol = { Symbol = extra_symbol }
				}
			},
			count = 
			{
				Text = ""
			}
		};
		
		if (item)
		{
			if (stack_count > 1 && !item->~IsInfiniteStackCount())
			{
				update.count.Text = Format("%dx", stack_count);
				inventory_slot.last_count = stack_count;
			}
		}
		else
		{
			inventory_slot.last_count = nil;
		}
		
		if (custom_overlay)
		{
			update.overlay = custom_overlay;
			update.overlay.ID = custom_overlay_id;
			inventory_slot.had_custom_overlay = true;
		}
		else
		{
			inventory_slot.had_custom_overlay = false;
		}
		
		GuiUpdate(update, inventory_gui_id, 1, this);

		inventory_slot.obj = item;
		inventory_slot.empty = !item;
	}
}



// Calculates the position of a specific button and returns a proplist.
func CalculateButtonPosition()
{
	var pos_x_offset = -(GUI_Controller_InventoryBar_IconSize + GUI_Controller_InventoryBar_IconMargin - GUI_Controller_InventoryBar_IconMargin) / 2;
	var pos_x = pos_x_offset;
	var pos_y = GUI_Controller_InventoryBar_IconMarginScreenTop;
	var pos =
	{
		Left = Format("50%%%s", ToEmString(pos_x)),
		Top = Format("0%%%s", ToEmString(pos_y)),
		Right = Format("50%%%s", ToEmString(pos_x + GUI_Controller_InventoryBar_IconSize)),
		Bottom = Format("0%%%s", ToEmString(pos_y + GUI_Controller_InventoryBar_IconSize))
	};
	return pos;
}

func FxExtraSlotUpdaterTimer(object target, proplist effect)
{
	if (!this) return FX_Execute_Kill;
	if (!target) return FX_Execute_Kill;
	if (target->Contained() != GetCursor(GetOwner())) return FX_Execute_Kill;
	return FX_OK;
}

func FxExtraSlotUpdaterUpdate(object target, proplist effect)
{
	if (this) ScheduleUpdateInventory();
}