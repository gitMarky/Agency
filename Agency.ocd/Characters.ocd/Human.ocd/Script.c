#include Clonk_Generic
#include Clonk_Animations
#include Clonk_Skins
#include Clonk_Sounds
#include Library_Character_Intenvory
#include Character_Control // Required for now, or else the inventory will not work correctly..........
#include Library_Character_Electrocution
#include Library_Character_Pacification
#include Library_Character_ThrowableTarget

#include Interaction_SilentAttack

/* --- Properties --- */

local Name = "Human";
local Description = "$Description$";
local MaxEnergy = 50000;
local MaxBreath = 720; // Clonk can breathe for 20 seconds under water.
local JumpSpeed = 400;
local ThrowSpeed = 294;
local ContactIncinerate = 10;

/* --- Engine Callbacks --- */

func Construction(object by)
{
	SetGraphics(nil, Clonk);
	 _inherited(by, ...);
}


func Definition(id type)
{
	var actmap_prototype;
	if (type.ActMap)
	{
		actmap_prototype = type;
	}
	else
	{
		actmap_prototype = Clonk;
	}

	type.PictureTransformation = Trans_Mul(Trans_Translate(0, 1000, 5000), Trans_Rotate(70, 0, 1, 0));
	type.ActMap = {
		Prototype = actmap_prototype.ActMap,
		Walk = {
			Prototype = actmap_prototype.ActMap.Walk,
			Speed = 50,
			Accel = 4,
			Decel = 16,
		},
		Run = {
			Prototype = actmap_prototype.ActMap.Walk,
			Accel = 12,
			Decel = 16,
		},
		Pacified = {
			Prototype = Action,
			Name = "Pacified",
			Directions = 2,
			Length = 1,
			Delay = 0,
			NextAction = "Hold",
			ObjectDisabled = 1,
		},
	};

	_inherited(type, ...);
}


// Callback from Death() when the Clonk is really really dead
func DeathEffects(proplist killed_by)
{
	_inherited(killed_by,...);

	// Some effects on dying.
	if (!this.silent_death)
	{
		this->PlaySkinSound("Die*");
	}
}

// Never get hit by objects, but let them bounce off of you
func QueryCatchBlow(object by)
{
	var block = "BlockBlowRebound";
	if (this->GetAlive() && !GetEffect(block, by))
	{
		// Calculate an angle for bouncing off
		var prec_a = 1000;
		var prec_p = 10;
		var prec_v = 100;
		var impact_angle = Normalize(Angle(0, 0, by->GetXDir(), by->GetYDir(), prec_a), -180 * prec_a, prec_a);
		var axis_angle = Normalize(Angle(GetX(prec_p), GetY(prec_p), by->GetX(prec_p), by->GetY(prec_p), prec_a), -180 * prec_a, prec_a);
		var rebound_angle = Normalize(180 * prec_a + 2 * axis_angle - impact_angle, -180 * prec_a, prec_a);
		var velocity = by->GetSpeed(prec_v);
		// Prevent it from bouncing off again, launch it
		AddEffect(block, by, 300, 10);
		by->SetVelocity(rebound_angle, velocity / 3, prec_a, prec_v);
	}
	return true;
}

/* --- Skin support --- */

func SetSkin(int new_skin)
{
	_inherited(new_skin);

	var prev_action = GetAction();

	SetGraphics(skin_name, Clonk);
	SetAction("Idle");
	SetAction(prev_action);

	if (GetColor() == 0 && GetOwner())
	{
		SetColor(GetOwner()->GetColor());
	}
}

