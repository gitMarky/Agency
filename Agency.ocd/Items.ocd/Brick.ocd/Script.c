
#include Property_Damage_Stunning
#include Property_Weapon_Melee
#include Property_Weapon_SilentAttack
#include Property_Weapon_Throwable

/* --- Properties --- */

local Collectible = true;
local Name = "$Name$";
local Description = "$Description$";

/* --- Engine Callbacks --- */


func Definition(id type)
{
	type.PictureTransformation = Trans_Mul(Trans_Rotate(30, 0, 1, 0), Trans_Rotate(-40, 1, 0, 1));
}

func Hit(int x, int y)
{
	StonyObjectHit(x, y);
}

/* --- Display ---*/

func GetCarryMode(object user, bool idle)
{
	if (idle)
	{
		return CARRY_None;
	}
	else
	{
		return CARRY_Hand;
	}
}

func GetCarryTransform(object user, bool idle)
{
	if (!idle) return Trans_Translate(400, 0, 700);
}
