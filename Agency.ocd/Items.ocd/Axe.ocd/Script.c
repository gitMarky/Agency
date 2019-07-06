
#include Property_Damage_Lethal
#include Property_Weapon_Melee

/* --- Properties --- */

local Collectible = true;
local Name = "$Name$";
local Description = "$Description$";
local BlastIncinerate = 30;
local MaterialIncinerate = true;
local BurnDownTime = 140;


/* --- Display --- */

func GetCarryMode(object clonk, bool idle)
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

func GetCarryTransform(object user, bool idle)
{
	if (idle) return;

	if (user->~IsWalking() || user->~IsJumping())
	{
		return Trans_Rotate(-90, 1, 0, 0);
	}
	else
	{
		return Trans_Mul(Trans_Translate(4500, 0, 0), Trans_Rotate(90, 1, 0, 0), Trans_Rotate(180, 0, 1, 0));
	}
}

func GetCarrySpecial(object user)
{
	return "pos_hand2";
}
