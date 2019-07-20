
#include Property_Damage_Stunning
#include Property_Weapon_Melee
#include Property_Weapon_SilentAttack
#include Property_Weapon_Throwable

/* --- Properties --- */

local Collectible = true;
local Name = "$Name$";
local Description = "$Description$";
local BlastIncinerate = 30;
local MaterialIncinerate = true;
local BurnDownTime = 140;

/* --- Engine Callbacks --- */


func Definition(id type)
{
	type.PictureTransformation = Trans_Rotate(20, 1, 0, 1);
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
		return CARRY_Belt;
	}
	else
	{
		return CARRY_HandBack;
	}
}

public func GetCarryTransform(object user, bool idle)
{
	if (!idle) return Trans_Rotate(-90,1,0,0);
}
