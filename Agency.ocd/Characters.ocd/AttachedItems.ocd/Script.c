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
	this.item_display.hand_mesh = nil;
	this.item_display.back_mesh = nil;
	this.item_display.side_mesh = nil;
	
	this.item_display.both_handed = false;
	this.item_display.on_back = false;
}


/* --- Attach & Detach --- */

public func DetachObject(object item)
{
	if (this->~GetHandItem() == item)
	{
		DetachHandItem();
	}
	if (this->~GetCarryOnlyItem() == item)
	{
		DetachSideItem();
	}
	if (this->~GetBackItem() == item)
	{
		DetachBackItem();
	}
}


func DetachBackItem()
{
	if (this.item_display.back_mesh)
	{
		DetachMesh(this.item_display.back_mesh);
		this.item_display.back_mesh = nil;
	}
}


func DetachHandItem()
{
	if (this.item_display.hand_mesh)
	{
		DetachMesh(this.item_display.hand_mesh);
		PlayAnimation("Close2Hand", CLONK_ANIM_SLOT_Hands + CARRY_SLOT_MAIN, Anim_Const(0));
		this.item_display.hand_mesh = nil;
	}
}


func DetachSideItem()
{
	if (this.item_display.side_mesh)
	{
		DetachMesh(this.item_display.side_mesh);
		PlayAnimation("Close1Hand", CLONK_ANIM_SLOT_Hands + CARRY_SLOT_SIDE, Anim_Const(0));
		this.item_display.side_mesh = nil;
	}
}


func UpdateAttach()
{
	StopAnimation(GetRootAnimation(CLONK_ANIM_SLOT_Hands));

	DetachBackItem();
	DetachHandItem();
	DetachSideItem();
	
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
	var hands_blocked = false;
	//if (!HasHandAction(sec, 1)) hands_blocked = true;
	

	var attach_mode = item->~GetCarryMode(this, carry_slot, hands_blocked);
	if (attach_mode == CARRY_None)
	{
		return;
	}
	
	// Determine attachment bones
	var bone = "main";
	if (item->~GetCarryBone())
	{
		bone  = item->~GetCarryBone(this, carry_slot, hands_blocked);
	}
	var bone2;
	if (item->~GetCarryBone2())
	{
		bone2 = item->~GetCarryBone2(this, carry_slot, hands_blocked);
	}
	else
	{
		bone2 = bone;
	}

	var transform = item->~GetCarryTransform(this, carry_slot, hands_blocked);

	var pos_hand = "pos_hand2";
	//if (sec) pos_hand = "pos_hand1";
	var pos_back = "pos_back1";
	//if (sec) pos_back = "pos_back2";
	var closehand = "Close2Hand";
	//if (sec) closehand = "Close1Hand";
	var pos_belt = "skeleton_leg_upper.R";
	//if (sec) pos_belt = "skeleton_leg_upper.L";


	if (attach_mode == CARRY_Hand || attach_mode == CARRY_HandBack || attach_mode == CARRY_HandAlways)
	{
		this.item_display.hand_mesh = AttachMesh(item, pos_hand, bone, transform);
		PlayAnimation(closehand, CLONK_ANIM_SLOT_Hands + carry_slot, Anim_Const(GetAnimationLength(closehand)));
	}
	else if (attach_mode == CARRY_Back)
	{
		this.item_display.hand_mesh = AttachMesh(item, pos_back, bone2, transform);
	}
	else if (attach_mode == CARRY_BothHands)
	{
			this.item_display.hand_mesh = AttachMesh(item, "pos_tool1", bone, transform);
			PlayAnimation("CarryArms", CLONK_ANIM_SLOT_Hands + carry_slot, Anim_Const(item->~GetCarryPhase(this)));
			this.item_display.both_handed = true;
	}
	else if (attach_mode == CARRY_Spear)
	{
		// This is a one sided animation, so switch to back if not in the main hand
		if (carry_slot == CARRY_SLOT_MAIN)
		{
			this.item_display.hand_mesh[sec] = AttachMesh(item, pos_hand, bone, transform);
			PlayAnimation("CarrySpear", CLONK_ANIM_SLOT_Hands + carry_slot, Anim_Const(0));
		}
		else
		{
			this.item_display.hand_mesh[sec] = AttachMesh(item, pos_back, bone2, transform);
		}
	}
	else if (attach_mode == CARRY_Blunderbuss)
	{
		if (!hands_blocked)
		{
			this.item_display.hand_mesh[sec] = AttachMesh(item, "pos_hand2", bone, transform);
			PlayAnimation("CarryMusket", CLONK_ANIM_SLOT_Hands + sec, Anim_Const(0), Anim_Const(1000));
			this.item_display.both_handed = true;
		}
		else
		{
			this.item_display.hand_mesh[sec] = AttachMesh(item, pos_back, bone2, transform);
		}
	}
	else if (attach_mode == CARRY_Grappler)
	{
		if (!hands_blocked)
		{
			this.item_display.hand_mesh[sec] = AttachMesh(item, "pos_hand2", bone, transform);
			PlayAnimation("CarryCrossbow", CLONK_ANIM_SLOT_Hands + sec, Anim_Const(0), Anim_Const(1000));
			this.item_display.both_handed = true;
		}
		else
		{
			this.item_display.hand_mesh[sec] = AttachMesh(item, pos_back, bone2, transform);
		}
	}
	else if (attach_mode == CARRY_Belt)
	{
		// Do some extra transforms for this kind of carrying
		if (transform)
		{
			transform = Trans_Mul(transform, Trans_Rotate(160, 0, 0, 1), Trans_Rotate(5, 0, 1), Trans_Rotate(30, 1), Trans_Translate(-2500, 0, 700), Trans_Scale(700));
		}
		else
		{
			transform = Trans_Mul(Trans_Rotate(160, 0, 0, 1), Trans_Rotate(5, 0, 1), Trans_Rotate(30, 1), Trans_Translate(-2500, 0, 800), Trans_Scale(700));
		}
		this.item_display.hand_mesh = AttachMesh(item, pos_belt, bone, transform);
	}
	else if (attach_mode == CARRY_Sword)
	{
		this.item_display.hand_mesh = AttachMesh(item, "skeleton_hips", bone, transform);
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

