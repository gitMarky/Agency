
#include Property_Damage_Lethal
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


/* --- Display --- */

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

func GetThrowableHitTransform(bool hit_from_behind)
{
	var skewed = Trans_Rotate(RandomX(-20, 10), 0, 1, 0);
	if (hit_from_behind)
	{
		return Trans_Mul(Trans_Translate(3000, 7000, 0), Trans_Rotate(-60, 0, 0, 1), skewed);
	}
	else
	{
		var mirrored = Trans_Scale(1000, -1000, 1000);
		return Trans_Mul(Trans_Translate(1000, -6500, 0), Trans_Rotate(40, 0, 0, 1), skewed, mirrored);
	}
}
