/**
	Crate
	Used for deliveries.
	
	@author Ringwaul
*/

#include Interaction_Container_Item
#include Interaction_PickUp
#include Property_Item_CarryOnly

/* --- Properties --- */

local Name = "$Name$";
local Description = "$Description$";
local Collectible = true;
local ContainerControl = CON_ExtraSlot;


/* --- Engine Callbacks --- */


func Definition(id type)
{
	type.PictureTransformation = Trans_Mul(Trans_Translate(-500, -1500, -3000), Trans_Rotate(-30, 1, 0, 0), Trans_Rotate(30, 0, 1, 0));
	_inherited(type, ...);
}


func Construction(object by)
{
	PlayAnimation("Open", 1, Anim_Linear(0, 0, 1, 20, ANIM_Hold));
	this.MeshTransformation = Trans_Rotate(RandomX(20, 80), 0, 1, 0);
	return _inherited(by, ...);
}


func Hit()
{
	Sound("Hits::Materials::Wood::DullWoodHit?");
	_inherited(...);
}


func RejectCollect(id type, object item)
{
	return ContentsCount() >= MaxContentsCount || item->~IsCarryHeavy() || item->~IsCarryOnly();
}


/* --- Interface --- */


func Open()
{
	PlayAnimation("Open", 5, Anim_Linear(0, 0, GetAnimationLength("Open"), 22, ANIM_Hold));
	Sound("Structures::Chest::Open");
}


func Close()
{
	PlayAnimation("Close", 5, Anim_Linear(0, 0, GetAnimationLength("Close"), 15, ANIM_Hold));
	Sound("Structures::Chest::Close");
}


/* --- Display --- */

func GetCarryMode()
{
	return CARRY_BothHands;
}


func GetCarryPhase() { return 800; }


func GetCarryTransform(object clonk)
{
	//if (GetCarrySpecial(clonk))
	//	return Trans_Translate(3500, 6500, 0);
	
	return Trans_Translate(0, 0, -1500);
}
