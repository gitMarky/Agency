#include Clonk_Generic
#include Clonk_Animations
#include Clonk_HandDisplay
#include Clonk_Skins
#include Clonk_Sounds

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
	type.PictureTransformation = Trans_Mul(Trans_Translate(0, 1000, 5000), Trans_Rotate(70, 0, 1, 0));
	type.ActMap = { Prototype = Clonk.ActMap, };

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
