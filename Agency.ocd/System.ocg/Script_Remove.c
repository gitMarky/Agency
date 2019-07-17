/**
	Additional removal functions.
 */


/**
	Removes an item. Calls RemoveObject(),
	but if the item is stackable it only
	removes one from the stack instead of
	the complete object.

	Issues a callback OnRemoveItem(bool removed_other)
 */
global func RemoveItem(bool eject_contents)
{
	AssertObjectContext();

	var item;
	if (this->~IsStackable())
	{
		item = this->TakeObject();
	}
	else
	{
		item = this;
	}
	this->~OnRemoveItem(item != this);
	item->RemoveObject(eject_contents, ...);
}
