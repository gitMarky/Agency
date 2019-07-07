/**
	Marks an object to act as a silent attack weapon.
 */
public func IsSilentAttackWeapon(){ return true; }


/**
	Performs a silent attack on a victim.

	@par victim the victim, must not be nil.
	@par attacker the attacker, must not be nil.
*/
func StartSilentAttack(object victim, object attacker)
{
	AssertNotNil(victim);
	AssertNotNil(attacker);

	var anim_name = "StrikePickaxe";
	var anim_length = attacker->GetAnimationLength(anim_name);
	var anim_start = 1500;
	var play_time = 50;

	attacker->SetTurnType(1);
	attacker->SetHandAction(1);
	attacker->UpdateAttach();

	var anim_nr = attacker->PlayAnimation(anim_name, CLONK_ANIM_SLOT_Arms, Anim_Linear(anim_start, 0, anim_length, play_time, ANIM_Loop), Anim_Linear(0, 0, 1000, 10));

	var attack = CreateEffect(FxSilentAttack, 1, 1);
	if (attack)
	{
		attack.attacker = attacker;
		attack.victim = victim;
		attack.time_strike = (3235 - anim_start) * play_time / anim_length;	// Create an offset, so that the hit matches with the animation
		attack.time_stop = (anim_length + 0) * play_time / anim_length;
		attack.anim = anim_nr;
	}
}


func DoSilentAttack(object victim, object attacker)
{
	if (this->~CausesLethalDamage())
	{
		victim->SetKiller(attacker->GetOwner());
		victim->Kill();
	}
}


func FinishSilentAttack(object attacker, int anim_nr)
{
	if (attacker)
	{
		attacker->SetTurnType(0);
		attacker->SetHandAction(false);
		attacker->UpdateAttach();
		attacker->StopAnimation(anim_nr ?? attacker->GetRootAnimation(CLONK_ANIM_SLOT_Arms));
		RemoveEffect(FxSilentAttack.Name, attacker);
	}
}


local FxSilentAttack = new Effect
{
	Name = "SilentAttack",

	Timer = func (int time)
	{
		// The attacker may be taken out himself, before he can finish
		// the attack, so cancel if he cannot do anything anymore.
		if (!this.attacker || this.attacker->~IsIncapacitated())
		{
			return FX_Execute_Kill;
		}

		// Do the actual strike?
		CheckStrike(time);

		if (time >= this.time_stop)
		{
			return FX_Execute_Kill;
		}
	},

	CheckStrike = func (int time)
	{
		if (this.struck)
		{
			return;
		}

		// If the victim is already down, you should still finish
		// the animation, but not strike the victim
		if (!this.victim || this.victim->~IsIncapacitated())
		{
			this.struck = true;
			return;
		}

		// Strike!
		if (time >= this.time_strike)
		{
			this.struck = true;
			Target->DoSilentAttack(this.victim, this.attacker);
		}
	},

	Stop = func ()
	{
			Target->FinishSilentAttack(this.attacker, this.anim_nr);
	},
};
