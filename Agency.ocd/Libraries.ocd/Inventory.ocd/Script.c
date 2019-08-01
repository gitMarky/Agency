/**
	Inventory
*/


/*
	The inventory management:
	The objects in the inventory are saved (parallel to Contents()) in the
	array 'inventory'. They are accessed via GetItem(i) and GetItemPos(obj).
	
	The functions return:
	- GetHandItem() returns the object in the (right) hand. This is the
	  item that you will use by default.
	- GetSideItem() returns the object in the (left) hand. This is
	  an item that is too just carried and is too larget to sling on
	  the back.
	- GetBackItem() returns the object that is carried on your back.
	- GetActiveItem() returns the object that is selected.

	used properties:
	this.inventory.objects: items in the inventory, array
	this.inventory.force_collection: marks that the user is actively trying to pick something up, instead of external collection
	this.inventory.in_hands: item in right hand
	this.inventory.on_back: item on back
	this.inventory.active_item: item that is selected (and in hands if drawn)
	this.inventory.carry_only; item in the left hand
*/

/* --- Engine Callbacks --- */

func Construction(object by)
{
	if (this.inventory == nil)
	{
		this.inventory = {};
	}
	this.inventory.objects = [];
	this.inventory.force_collection = false;
	this.inventory.in_hands = nil;
	this.inventory.on_back = nil;
	this.inventory.active_item = nil;
	this.inventory.carry_only = nil;
	return _inherited(...);
}


/**
	Overload of Collect function
	Allows blocking collection via RejectCollect
*/
func Collect(object item, bool ignoreOCF, bool force)
{
	// Whenever a script calls the Collect function manually, an intended force is assumed.
	// That means, items will usually be collected with Collect().
	SetCollecting(force ?? true);
	var success = _inherited(item, ignoreOCF);
	SetCollecting(false);
	return success;
}


func Collection2(object item)
{
	TrySelectActiveItem(item);

	// OnSlotFull might have done something to obj
	if (GetActiveItem() == item)
	{
		// Handled in tryselectactiveitem now
		//if (this.inventory.force_collection && GetHandItem() == nil)
		//{
		//	SetHandItem(item);
		//}
		item->~Selection(this);
	}
	this->~UpdateAttach();

	return _inherited(item, ...);
}


func Ejection(object item)
{
	// If an object leaves this object
	// find obj in array and delete (cancel using too)
	var pos = GetItemPos(item);
	if (pos != nil)
	{
		this.inventory.objects[pos] = nil;
	}

    // Variable declared in ClonkControl.ocd
	if (this->~GetUsedObject() == item)
	{
		this->~CancelUse();
	}

	// Delete item from inventory
	var was_in_hand = GetHandItem() == item;
	if (was_in_hand)
	{
		item->~Deselection(this);
		SetHandItem(nil);
	}
	if (GetActiveItem() == item)
	{
		SetActiveItem(nil);
	}
	if (GetBackItem() == item)
	{
		SetBackItem(nil);
	}
	if (GetSideItem() == item)
	{
		SetSideItem(nil);
	}
	
	// Get new active item?
	var type = item->GetID();
	TrySelectActiveItem(nil, type, was_in_hand);
	
	// Callbacks
	this->~UpdateAttach();	
	_inherited(item, ...);
}


func ContentsDestruction(object item)
{
	if (item == GetActiveItem())
	{
		SetActiveItem(nil);
	}
	TrySelectActiveItem(nil, item->GetID(), item == GetActiveItem());
	this->~OnInventoryChange();
	_inherited(item, ...);
}


func RejectCollect(id type, object obj)
{
	// Collection of that object magically disabled?
	if (GetEffect("NoCollection", obj))
	{
		return true;
	}

	// Only handle extra-slot objects if the object was not dropped on purpose.
	if (IsCollecting())
	{
		// Try to stuff obj into an object with an extra slot
		for (var i = 0; Contents(i); ++i)
			if (Contents(i)->~HasExtraSlot())
				if (!(Contents(i)->Contents(0)))
					if (Contents(i)->Collect(obj, true))
						return true;
						
		// Try to stuff an object in clonk into obj if it has an extra slot
		if (obj->~HasExtraSlot())
			if (!(obj->Contents(0)))
				for (var i = 0; Contents(i); ++i)
					if (obj->Collect(Contents(i),true))
						return false;
	}
	else
	{
		// Prevent picking up stuff on the ground.
		if (!obj->Contained())
		{
			return true;
		}
	}
	// Can't carry bucket material with bare hands.
	if (obj->~IsBucketMaterial())
	{
		return true;
	}
	
	return _inherited(type, obj,...);
}


func GrabContents(object source, ...)
{
	// Try to put grabbed items into same slot (for respawn)
	if (source)
	{
		var i = source->ContentsCount();
		while (--i >= 0)
		{
			var item = source->Contents(i);
			if (item)
			{
				var item_pos = source->GetItemPos(item);
				// Collect this into same slot index if it's a valid, free slot for this object
				if (GetType(item_pos) && item_pos >=0 && !GetItem(item_pos))
				{
					Collect(item, true, item_pos);
				}
			}
		}
	}
	// Grab remaining items
	return inherited(source, ...);
}


/* --- Interface --- */

/**
	Get the 'i'th item in the inventory
*/
func GetItem(int i)
{
	if (i < 0 || i >= GetLength(this.inventory.objects))
	{
		return nil;
	}	
	return this.inventory.objects[i];
}


/**
	Returns all items in the inventory
*/
func GetItems()
{
	var inv = this.inventory.objects[:];
	RemoveHoles(inv);
	return inv;
}


/**
	Returns how many items are in the clonks inventory
    Does not have to be the same as ContentCounts() because of objects with special handling, like CarryHeavy
*/
func GetItemCount()
{
	var count = 0;
	for (var i = 0; i < GetLength(this.inventory.objects); i++)
		if (this.inventory.objects[i])
			count++;
	
	return count;
}


/**
	This item is held in the right hand
*/
func GetHandItem()
{
	return this.inventory.in_hands;
}

func SetHandItem(object item)
{
	this.inventory.in_hands = item;
}


/**
	This item is selected for holstering/drawing.
	This is also the item for the main hand,
	which may be confusing (and maybe I find a better solution)
*/
func GetActiveItem()
{
	return this.inventory.active_item;
}

func SetActiveItem(object item)
{
	this.inventory.active_item = item;
}


/**
	This item is carried on the back.
*/
func GetBackItem()
{
	return this.inventory.on_back;
}

func SetBackItem(object item)
{
	this.inventory.on_back = item;
}


/**
	This item is carried in the left hand.
*/
func GetSideItem()
{
	return this.inventory.carry_only;
}

func SetSideOnlyItem(object item)
{
	this.inventory.carry_only = item;
}


/**
	The player is actively picking something up.
*/
func SetCollecting(bool value)
{
	this.inventory.force_collection = value;
}

func IsCollecting()
{
	return this.inventory.force_collection;
}


/**
	Search for the index of an item
*/
func GetItemPos(object item)
{
	if (item && item->Contained() == this)
	{
		var index = GetIndexOf(this.inventory.objects, item);
		if (index >= 0)
		{
			return index;
		}
	}
	return nil;
}


/* --- Internals --- */

/**
	Drops the item , if any
*/
func DropItem(object item)
{
	if (!item || item->~QueryRejectDeparture(this))
	{
		return false;
	}
	// Notify other libraries of deliberate drop.
	this->~OnDropped(item);
	// And make the engine drop the object.
	//this->AddCommand("Drop", item);
	if (item)
	{
		item->Exit(0, 8);
	}
	return true;
}

/** Switch two items in the clonk's inventory */
public func Switch2Items(int one, int two)
{
/*
	// no valid inventory index: cancel
	if (!Inside(one, 0, MaxContentsCount-1)) return;
	if (!Inside(two, 0, MaxContentsCount-1)) return;

	// switch them around
	var temp = this.inventory.objects[one];
	this.inventory.objects[one] = this.inventory.objects[two];
	this.inventory.objects[two] = temp;
	
	// callbacks: cancel use, variable declared in ClonkControl.ocd
	if (this.control.current_object == this.inventory.objects[one] || this.control.current_object == this.inventory.objects[two])
		this->~CancelUse();
	
	var handone, handtwo;
	handone = GetHandPosByItemPos(one);
	handtwo = GetHandPosByItemPos(two);
	
	// callbacks: (de)selection
	if (handone != nil)
		if (this.inventory.objects[two]) this.inventory.objects[two]->~Deselection(this, one);
	if (handtwo != nil)
		if (this.inventory.objects[one]) this.inventory.objects[one]->~Deselection(this, two);
		
	if (handone != nil)
		if (this.inventory.objects[one]) this.inventory.objects[one]->~Selection(this, one);
	if (handtwo != nil)
		if (this.inventory.objects[two]) this.inventory.objects[two]->~Selection(this, two);
	
	// callbacks: to self, for HUD
	if (handone != nil)
	{
		if (this.inventory.objects[one])
			this->~OnSlotFull(handone);
		else
			this->~OnSlotEmpty(handone);
	}
	if (handtwo != nil)
	{
		if (this.inventory.objects[two])
			this->~OnSlotFull(handtwo);
		else
			this->~OnSlotEmpty(handtwo);
	}
	
	this->~OnInventoryChange(one, two);
	this->~UpdateAttach();
*/
}


// Pick up item via interaction.
func TryToCollect(object item)
{
	if (!GetEffect(FxPickUpItem.Name, this))
	{
		CreateEffect(FxPickUpItem, 1, 10, item);
	}
}


// Callback from animation effect
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


func TrySelectActiveItem(object preferred_item, id preferred_type, bool was_in_hands)
{
	if (GetActiveItem())
	{
		return;
	}
	var candidate = preferred_item;

	if (!candidate && GetHandItem())
	{
		candidate = GetHandItem();
		was_in_hands = false; // Actually 'true' but there is no need to set it again in the end
	}
	if (!candidate)
	{
		var contents = Find_Container(this);
		var candidates;
		if (preferred_type)
		{
			candidates = FindObjects(contents, Find_ID(preferred_type));
			candidates = Concatenate(candidates, FindObjects(contents, Find_Not(Find_ID())));
		}
		else
		{
			candidates = FindObjects(contents);
		}
		for (var item in candidates)
		{
			if (item == GetHandItem() || item == GetBackItem() || item == GetSideItem())
			{
				continue;
			}
			candidate = item;
			if (preferred_type != item->GetID())
			{
				was_in_hands = false;
			}
			break;
		}
	}
	SetActiveItem(candidate);
	if (GetHandItem() == nil && IsCollecting() && candidate == preferred_item)
	{
		was_in_hands = true;
	}
	if (was_in_hands)
	{
		SetHandItem(candidate);
	}
}


func DoHolsterHandItem(bool reset_active_item)
{
	var item = GetHandItem();
	if (item)
	{
		if (item->~IsCarryOnly())
		{
			if (reset_active_item)
			{
				DropItem(item);
			}
			else
			{
				return;
			}
		}
		else if (item->~IsLargeItem())
		{
			if (GetBackItem())
			{
				DropItem(item);
			}
			else
			{
				SetBackItem(item);
			}
		}
		SetHandItem(nil);

		if (reset_active_item)
		{
			SetActiveItem(nil);
		}
		this->UpdateAttach();
	}
}


func CanHolsterItem(object item)
{
	if (item)
	{
		// These can never be holstered
		if (item->~IsCarryOnly() || item->~IsCarryHeavy())
		{
			return false;
		}
		// Only if there is a free slot on your back
		else if (item->~IsLargeItem())
		{
			return GetBackItem() == nil;
		}
		// Otherwise you can always holster an item
		return true;
	}
	return false;
}


func CanCollectItem(object item)
{
	var info =
	{
		Collectible = false,
		Holster = nil, // Holsterable right-hand item
		Swap = nil, // Swappable right-hand item
		Drop = nil, // Droppable left-hand item
	};
	if (item)
	{
		// Collect to the left hand?
		if (item->~IsSideItem())
		{
			// Left hand can carry one-handed items only
			if (!IsCarriedInBothHands(item))
			{
				info.Collectible = true;
				SetCollectExchange(GetSideItem(), info);
			}
		}
		// Collect to the right hand?
		else
		{
			// Two handed item?
			if (IsCarriedInBothHands(item))
			{
				// Something in left hand?
				if (GetSideItem())
				{
					info.Drop = GetSideItem();
				}
				// Collect!
				info.Collectible = true;
				SetCollectExchange(GetHandItem(), info);
			}
			// One handed item?
			else
			{
				info.Collectible = true;
				SetCollectExchange(GetHandItem(), info);
			}
		}
	}
	return info;
}


func IsCarriedInBothHands(object item)
{
	if (item)
	{
		return item->~IsCarryHeavy() || item.BothHandedCarry;
	}
	return false;
}


func SetCollectExchange(object current, proplist info)
{
	if (CanHolsterItem(current))
	{
		info.Holster = current;
	}
	else // current may be nil, that just means that you swap nothing
	{
		info.Swap = current;
	}
}


/* --- Animations & Effects --- */

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
