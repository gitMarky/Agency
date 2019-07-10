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
	
	var animation = Strike_Animations.Pickaxe;

	var anim_name = animation->GetName();
	var anim_length = animation->GetAnimationLength();
	var anim_start = animation->GetAnimationStart();
	var anim_ending = animation->GetAnimationEnding();
	var play_time = 50;

	attacker->SetTurnForced(attacker->GetDir());
	attacker->SetTurnType(1);
	attacker->SetHandAction(1);
	attacker->UpdateAttach();

	victim->SetTurnForced(victim->GetDir());
	var anim_nr = attacker->PlayAnimation(anim_name, CLONK_ANIM_SLOT_Arms, Anim_Linear(anim_start, 0, anim_length, play_time, anim_ending), Anim_Linear(0, 0, 1000, 10));

	var attack = CreateEffect(FxSilentAttack, 1, 1);
	if (attack)
	{
		attack.dir = attacker->GetDir();
		attack.attacker = attacker;
		attack.victim = victim;
		attack.time_strike = animation->GetStrikeTime(play_time); // Create an offset, so that the hit matches with the animation
		attack.time_stop = play_time;
		attack.anim = anim_nr;
		attack.constraint = CreateEffect(FxSpringConstraint, 1, 1, 12, 15)->SetBodyA(attacker)->SetBodyB(victim, true);
	}
}


func DoSilentAttack(object victim, object attacker)
{
	CauseDamage(victim, attacker->GetController());
}


func FinishSilentAttack(object victim, object attacker, int anim_nr, int dir)
{
	if (victim)
	{
		victim->SetTurnForced(-1);	
	}

	if (attacker)
	{
		attacker->SetTurnForced(-1);
		attacker->SetTurnType(0);
		attacker->SetHandAction(false);
		attacker->UpdateAttach();
		attacker->SetXDir();
		attacker->SetComDir(COMD_Stop);
		attacker->SetDir(dir);
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
		if (!this.attacker || !this.attacker->GetAlive())
		{
			return FX_Execute_Kill;
		}

		this.attacker->SetComDir(COMD_Stop);
		!this.victim || this.victim->SetComDir(COMD_Stop);

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

		// You should still finish the animation, 
		// but not strike the victim
		// - if the victim is already down
		// - if the constraint is gone (meaking victim is too far way) 
		if (!this.victim || !this.victim->GetAlive() || this.constraint == nil)
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
		if (this.constraint)
		{
			RemoveEffect(nil, nil, this.constraint);
		}
		Target->FinishSilentAttack(this.victim, this.attacker, this.anim_nr, this.dir);
	},
};
