
#include Property_Breakable
#include Property_Damage_Stunning
#include Property_Weapon_Melee
#include Property_Weapon_SilentAttack
#include Property_Weapon_Throwable

/* --- Properties --- */

local Collectible = true;
local Name = "$Name$";
local Description = "$Description$";

local BreakOnMelee = true;
local BreakOnThrow = true;
local BreakOnHit = 27;

/* --- Engine Callbacks --- */


func Definition(id type)
{
	type.PictureTransformation = Trans_Mul(Trans_Rotate(30, 0, 1, 0), Trans_Rotate(-40, 1, 0, 1));
}

func Hit(int x, int y)
{
	_inherited(x, y, ...);
	StonyObjectHit(x, y);
}

/* --- Display ---*/

func GetCarryMode(object user, bool idle)
{
	if (idle || GetEffect("ForceCarryNone", this))
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

func GetSilentAttackAnimation()
{
	return 
	{
		Animation = Strike_Animations.FastStrike,
		Direction = ".R",
	};
}

/* --- Effects --- */

func EffectsOnBreak(int x, int y, object user)
{
	var particles =
	{
		Prototype = Particles_Dust(),
		R = 170,
		G = 112,
		B = 77,
	};
	var particles_large = 
	{
		Prototype = particles,
		Rotation = PV_Linear(PV_Random(0, 360), PV_Random(0, 360)),
	};
	var particles_small =
	{
		Prototype = particles,
		ForceY = PV_Gravity(300),
		Size = PV_KeyFrames(0, 0, 0, 100, 2, 1000, 1),
		Alpha = PV_KeyFrames(0, 0, 0, 250, 180, 1000, 0),
	};
	var pv_name = "Dust";
	var pv_lifetime_large = 50;
	var pv_lifetime_small = PV_Random(36, 96);
	var amount = 20;
	var pv_bone = "pos_hand2";
	if (user && user->~IsClonk())
	{
		user->CreateParticleAtBone(pv_name, pv_bone, nil, nil, pv_lifetime_large, particles_large);
		for (var i = 0; i < amount; ++i) // PV_Random does not work in the array
		{
			user->CreateParticleAtBone(pv_name, pv_bone, [RandomX(-2, 2), RandomX(-2, 2), RandomX(-2, 2)], [RandomX(-3, 3) * 5, RandomX(-3, 3) * 5, RandomX(-3, 3) * 5], pv_lifetime_small, particles_small);
		}
		// Hide the mesh
		AddEffect("ForceCarryNone", this, 1);
		user->~UpdateAttach();
	}
	else
	{
		CreateParticle(pv_name, x, y, 0, -3, pv_lifetime_large, particles_large);
		CreateParticle(pv_name, PV_Random(x - 2, x + 2), PV_Random(y - 2, y + 2), PV_Random(-3, 3), PV_Random(-6, 0), pv_lifetime_small, particles_small, amount);
	}
	_inherited(x, y, user, ...);
}


func OnRemoveItem(bool removed_other)
{
	if (removed_other)
	{
		RemoveEffect("ForceCarryNone", this);
	}
}

