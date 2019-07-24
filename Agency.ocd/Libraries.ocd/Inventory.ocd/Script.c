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
	- GetCarryOnlyItem() returns the object in the (left) hand. This is
	  an item that is too just carried and is too larget to sling on
	  the back.
	- GetBackItem() returns the object that is carried on your back.
	- GetActiveItem() returns the object that is selected.

	used properties:
	this.inventory.objects: items in the inventory, array
	this.inventory.force_collection: used to pick stuff up, even though the hand-slots are all full (see RejectCollect + Collect with CON_Collect)
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
func GetCarryOnlyItem()
{
	return this.inventory.carry_only;
}

func SetCarryOnlyItem(object item)
{
	this.inventory.carry_only = item;
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
			// Collect but do not sort in_
			// Collection2 will be called which attempts to automatically sort in
			// the collected item into the next free inventory slot. Since 'pos'
			// is given as a parameter, we don't want that to happen and sort it
			// in manually afterwards
			var success = _inherited(item);
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

