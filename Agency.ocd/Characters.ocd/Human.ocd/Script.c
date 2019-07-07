#include Clonk_Generic
#include Clonk_Animations
#include Clonk_Skins
#include Clonk_Sounds
#include Library_Character_Intenvory

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
	};

	_inherited(type, ...);
}


// Callback from Death() when the Clonk is really really dead
func DeathEffects(int killed_by)
{
	_inherited(killed_by,...);

	// Some effects on dying.
	if (!this.silent_death)
	{
		this->PlaySkinSound("Die*");
	}
}

/* --- Skin support --- */

func SetSkin(int new_skin)
{
	_inherited(new_skin);

	var prev_action = GetAction();

	SetGraphics(skin_name, Clonk);
	SetAction("Idle");
	SetAction(prev_action);

	if (GetColor() == 0)
	{
		SetColor(GetPlayerColor(GetOwner()));
	}
}

/* --- Status --- */

func IsIncapacitated()
{
	return !GetAlive();
}
