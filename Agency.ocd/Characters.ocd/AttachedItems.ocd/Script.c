/**
	Logic for displaying items in the hand.
	
	Copied from Objects.ocd
*/

static const CARRY_SLOT_MAIN = 0;
static const CARRY_SLOT_SIDE = 1;
static const CARRY_SLOT_BACK = 3;

/* Carry items on the clonk */

// Mesh attachment handling
/* Features 4 properties:
	hand_mesh: Array of attachment numbers for items on the clonk.
	hand_action: Determines whether the clonk's hands are busy if items can be used.
		one of three ints: -1, 0 or 1
		-1: no items are drawn on the clonk but they are usable
		 0: items are drawn and can be used
		+1: items are not drawn and cannot be used
	both_handed: The first item held is held with both hands, so draw the second one differently.
	on_back: The first item is currently on the clonk's back, so draw the second one differently (if it also goes on the back).
*/


/* --- Properties --- */

local item_display;


/* --- Engine Callbacks --- */

func Construction(object by)
{
	_inherited(by, ...);

	this.item_display = {};
	this.item_display.hand = { mesh_nr = nil, anim_nr = nil };
	this.item_display.back = { mesh_nr = nil, anim_nr = nil };
	this.item_display.side = { mesh_nr = nil, anim_nr = nil };
	
	this.item_display.both_handed = false;
}


/* --- Attach & Detach --- */

public func DetachObject(object item)
{
	if (this->~GetHandItem() == item)
	{
		DetachItem(this.item_display.hand);
	}
	if (this->~GetCarryOnlyItem() == item)
	{
		DetachItem(this.item_display.side);
	}
	if (this->~GetBackItem() == item)
	{
		DetachItem(this.item_display.back);
	}
}


func DetachItem(proplist slot)
{
	if (slot.mesh_nr)
	{
		DetachMesh(slot.mesh_nr);
		slot.mesh_nr = nil;
	}
	if (slot.anim_nr)
	{
		StopAnimation(slot.anim_nr);
		slot.anim_nr = nil;
	}
}


func UpdateAttach()
{
	if (this.item_display)
	{
		DetachItem(this.item_display.hand);
		DetachItem(this.item_display.side);
		DetachItem(this.item_display.back);
	}
	
	UpdateAttachedItem(this->~GetHandItem(), CARRY_SLOT_MAIN);
	UpdateAttachedItem(this->~GetCarryOnlyItem(), CARRY_SLOT_SIDE);
	UpdateAttachedItem(this->~GetBackItem(), CARRY_SLOT_BACK);
}


func UpdateAttachedItem(object item, int carry_slot)
{
	// Cancel display?
	if (!item)
	{
		return;
	}

	// Determine if hands are blocked
	var hands_blocked = !HasActionProcedure();
	//if (!HasHandAction(sec, 1)) hands_blocked = true;
	

	var attach_mode = item->~GetCarryMode(this, carry_slot, hands_blocked);
	if (attach_mode == CARRY_None)
	{
		return;
	}

	// Determine attachment bones
	var attach_item = "main";
	if (GetType(item.GetCarryBone) == C4V_Function)
	{
		attach_item  = item->GetCarryBone(this, carry_slot, hands_blocked);
	}
	var transform = item->~GetCarryTransform(this, carry_slot, hands_blocked);

	// Determine attachment at 
	var attach_user;
	var animation_name;
	var slot;
	if (carry_slot == CARRY_SLOT_MAIN)
	{
		attach_user = "pos_hand2";
		animation_name = "Close2Hand";
		slot = this.item_display.hand;
	}
	else if (carry_slot == CARRY_SLOT_SIDE)
	{
		attach_user = "pos_hand1";
		animation_name = "Close1Hand";
		slot = this.item_display.side;
	}
	else if (carry_slot == CARRY_SLOT_BACK)
	{
		attach_user = "pos_back1";
		// No animation
		slot = this.item_display.back;
	}
	var animation_phase = Anim_Const(0);
	if (animation_name)
	{
		animation_phase = Anim_Const(GetAnimationLength(animation_name));
	}

	// Modify settings for special carry modes
	if (attach_mode == CARRY_BothHands)
	{
		attach_user = "pos_tool1";
		animation_name = "CarryArms";
		animation_phase = Anim_Const(item->~GetCarryPhase(this));
		this.item_display.both_handed = true;
	}
	else if (attach_mode == CARRY_Spear)
	{
		// This is a one sided animation, so switch to back if not in the main hand
		if (carry_slot == CARRY_SLOT_MAIN)
		{
			animation_name = "CarrySpear";
			animation_phase = Anim_Const(0);
		}
	}
	else if (attach_mode == CARRY_Blunderbuss)
	{
		animation_name = "CarryMusket";
		animation_phase = Anim_Const(0);
		this.item_display.both_handed = true;
	}
	else if (attach_mode == CARRY_Grappler)
	{
		animation_name = "CarryCrossbow";
		animation_phase = Anim_Const(0);
		this.item_display.both_handed = true;
	}
	else if (attach_mode == CARRY_Belt)
	{
		attach_user =  "skeleton_leg_upper.R";;
		// Do some extra transforms for this kind of carrying
		if (transform)
		{
			transform = Trans_Mul(transform, Trans_Rotate(160, 0, 0, 1), Trans_Rotate(5, 0, 1), Trans_Rotate(30, 1), Trans_Translate(-2500, 0, 700), Trans_Scale(700));
		}
		else
		{
			transform = Trans_Mul(Trans_Rotate(160, 0, 0, 1), Trans_Rotate(5, 0, 1), Trans_Rotate(30, 1), Trans_Translate(-2500, 0, 800), Trans_Scale(700));
		}
	}
	else if (attach_mode == CARRY_Sword)
	{
		attach_user = "skeleton_hips";
	}
	
	// Attach the mesh
	slot.mesh_nr = AttachMesh(item, attach_user, attach_item, transform);
	if (animation_name)
	{
		slot.anim_nr = PlayAnimation(animation_name, CLONK_ANIM_SLOT_Hands + carry_slot, animation_phase);
	}
}


func HasActionProcedure(bool force_landscape_letgo)
{
	// Check if the clonk is currently in an action where he could use his hands
	// if force_landscape_letgo is true, also allow during scale/hangle assuming the clonk will let go
	var action = GetAction();
	if (action == "Walk" || action == "Jump" || action == "WallJump" || action == "Kneel" || action == "Ride" || action == "BridgeStand")
	{
		return true;
	}
	if (force_landscape_letgo) if (action == "Scale" || action == "Hangle")
	{
		return true;
	}
	return false;
}

public func ReadyToAction(fNoArmCheck)
{
	return HasActionProcedure();
}

