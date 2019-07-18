
#include Property_Breakable
#include Property_ElectricCurrent
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
local BreakOnHit = true;

/* --- Engine Callbacks --- */


func Definition(id type)
{
	type.PictureTransformation = Trans_Mul(Trans_Rotate(-25, 1, 0, 0), Trans_Rotate(40, 0, 1, 0));
}

func Hit(int x, int y)
{
	_inherited(x, y, ...);
	if (this)
	{
		Sound("Hits::Materials::Metal::DullMetalHit?");
	}
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
	//if (!idle) return Trans_Translate(400, 0, 700);
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
	_inherited(x, y, user, ...);
}


func EffectsOnElectricCurrent()
{
	CreateParticle("ElectroSpark", 0, -4, PV_Random(-5, 5), PV_Random(-5, 1), 10, { Prototype = Particles_ElectroSpark1(),  Size = PV_Random(2, 4),}, RandomX(2, 5));
}


func TriggerOnBreak()
{
	Electrify();
}
