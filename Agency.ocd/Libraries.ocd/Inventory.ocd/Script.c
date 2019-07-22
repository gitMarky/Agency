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
	- GetOffHandItem() returns the object in the (left) hand. This is
	  an item that is too just carried and is too larget to sling on
	  the back.
	- GetBackItem() returns the object that is carried on your back.
	- GetSelectedItem() returns the object that is selected.

	used properties:
	this.inventory.objects: items in the inventory, array
	this.inventory.disableautosort: used to get default-Collection-behaviour (see Collection2)
	this.inventory.force_collection: used to pick stuff up, even though the hand-slots are all full (see RejectCollect + Collect with CON_Collect)
	this.inventory.in_hands: item index
	this.inventory.on_back: item index
	this.inventory.active_item: itemindex
	this.inventory.carry_only; item index
*/

/* --- Engine Callbacks --- */

func Construction(object by)
{
	if (this.inventory == nil)
	{
		this.inventory = {};
	}
	this.inventory.objects = [];
	this.inventory.disableautosort = false;
	this.inventory.force_collection = false;
	this.inventory.in_hands = nil;
	this.inventory.on_back = nil;
	this.inventory.active_item = nil;
	this.inventory.carry_only = nil;
	return _inherited(...);
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
	return GetItem(this.inventory.active_item);
}


/**
	This item is selected for holstering/drawing.
	This is also the item for the main hand,
	which may be confusing (and maybe I find a better solution)
*/
func GetActiveItem()
{
	return GetItem(this.inventory.in_hands);
}


/**
	This item is carried on the back.
*/
func GetBackItem()
{
	return GetItem(this.inventory.on_back);
}


/**
	This item is carried in the left hand.
*/
func GetCarryOnlyItem()
{
	return GetItem(this.inventory.carry_only);
}


/** Set the 'hand'th use-item to the 'inv'th slot */
public func SetHandItemPos(int hand, int inv)
{
/*
	// indices are in range?
	if (hand >= HandObjects || inv >= MaxContentsCount)
		return nil;
	if (hand < 0 || inv < 0) return nil;
	// no slot change?
	if (inv == GetHandItemPos(hand)) return nil;
	// the current hand object needs to be both notified and asked
	var old_object = GetHandItem(hand);
	if (old_object && old_object->~QueryRejectDeselection(this, hand))
		return nil;
	// when we want to swap placed with another hand, the other object also has to be asked
	var hand2 = GetHandPosByItemPos(inv);
	var old_hand2_object = nil;
	if (hand2 != nil)
	{
		old_hand2_object = GetHandItem(hand2);
		if (old_hand2_object && old_hand2_object->~QueryRejectDeselection(this, hand2)) return nil;
	}
	
	// changing slots cancels using, if the slot with the used object is contained
	if (this.control.current_object) // declared in ClonkControl.ocd
	{
		var used_slot = GetItemPos(this.control.current_object);
		if (used_slot != nil)
			if (used_slot == GetHandItemPos(hand) || used_slot == inv)
				this->~CancelUseControl(0, 0);
	}
	
	// If the item is already selected, we can't hold it in another one too.
	if (hand2 != nil)
	{
		// switch places
		this.inventory.hand_objects[hand2] = this.inventory.hand_objects[hand];
		this.inventory.hand_objects[hand] = inv;
		
		// additional callbacks
		if (old_hand2_object)
			old_hand2_object->~Deselection(this, hand2);
		if (old_object)
			old_object->~Deselection(this, hand);
			
		// notify new hand2 item (which should be old hand-item) that it has been selected
		var hand_item = GetHandItem(hand2);
		if (hand_item)
		{
			this->~OnSlotFull(hand2);
			// OnSlotFull might have done something to the item
			if (GetHandItem(hand2) == hand_item)
				hand_item->~Selection(this, hand2);
		}
		else
			this->~OnSlotEmpty(hand2);
	}
	else
	{
		this.inventory.hand_objects[hand] = inv;
		
		// notify the old object that it was already deselected
		if (old_object)
			old_object->~Deselection(this, hand);
	}
	
	// notify the new item that it was selected
	var item = GetItem(inv);
	if (item)
	{
		this->~OnSlotFull(hand);
		// OnSlotFull might have done something to the item
		if (GetItem(inv) == item)
			GetItem(inv)->~Selection(this, hand);
	}
	else
	{
		this->~OnSlotEmpty(hand);
	}
*/
}


/**
	Drops the item in the inventory slot, if any
*/
func DropInventoryItem(int slot)
{
	var item = GetItem(slot);
	if (!item || item->~QueryRejectDeparture(this))
	{
		return nil;
	}
	// Notify other libraries of deliberate drop.
	this->~OnDropped(item);
	// And make the engine drop the object.
	this->AddCommand("Drop", item);
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


/**
	Overload of Collect function
	Allows inventory/hands-Handling with forced-collection
*/
func Collect(object item, bool ignoreOCF, int pos, bool force)
{
	// Whenever a script calls the Collect function manually, an intended force is assumed.
	// That means, items will usually be collected with Collect() even if the current hand-slot is not free.
	force = force ?? true;
	
	this.inventory.force_collection = force;
	var success = false;
	if (pos == nil || item->~IsCarryHeavy())
	{
		success = _inherited(item, ignoreOCF);
		this.inventory.force_collection = false;
		return success;
	}
	// Fail if the specified slot is full
	if (GetItem(pos) == nil && pos >= 0)
	{
		if (item)
		{
			this.inventory.disableautosort = true;
			// Collect but do not sort in_
			// Collection2 will be called which attempts to automatically sort in
			// the collected item into the next free inventory slot. Since 'pos'
			// is given as a parameter, we don't want that to happen and sort it
			// in manually afterwards
			var success = _inherited(item);
			this.inventory.disableautosort = false;
			if (success)
			{
				this.inventory.objects[pos] = item;
			}
		}
	}
	
	this.inventory.force_collection = false;
	return success;
}


func Collection2(object obj)
{

	// See Collect()
	if (this.inventory.disableautosort)
	{
		return _inherited(obj,...);
	}

	var sel = 0;
	var success = false;

	// Sort into selected hands if empty

	// Otherwise, first empty slot, expand inventory by 1 if necessary
	for (var i = 0; i <= GetLength(this.inventory.objects); ++i)
	{
		if (!GetItem(i))
		{
			sel = i;
			this.inventory.objects[sel] = obj;
			success = true;
			break;
		}
	}
	
	// Callbacks
	if (success)
	{
		// OnSlotFull might have done something to obj
		if (GetActiveItem() == obj)
		{
			obj->~Selection(this);
		}
	}

	return _inherited(obj,...);
}


func Ejection(object obj)
{
	// If an object leaves this object
	// find obj in array and delete (cancel using too)
	var success = false;

	var pos = GetItemPos(obj);
	if (pos != nil)
	{
		this.inventory.objects[pos] = nil;
		success = true;
	}

    // Variable declared in ClonkControl.ocd
	if (this.control.current_object == obj)
	{
		this->~CancelUse();
	}

	// Callbacks
	if (success)
	{
		if (GetHandItem() == obj)
		{
			obj->~Deselection(this);
		}
		this->~UpdateAttach();
	}
	
	_inherited(obj,...);
}


func ContentsDestruction(object obj)
{
	// tell the Hud that something changed
	this->~OnInventoryChange();
	_inherited(obj, ...);
}

func RejectCollect(id type, object obj)
{
	// Collection of that object magically disabled?
	if (GetEffect("NoCollection", obj))
	{
		return true;
	}

	// Only handle extra-slot objects if the object was not dropped on purpose.
	if (this.inventory.force_collection)
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
	if (obj->~IsBucketMaterial()) return true;
	
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

