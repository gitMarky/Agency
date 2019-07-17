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
	
	var silent_attack = this->~GetSilentAttackAnimation() ??
	{
		Animation = Strike_Animations.Pickaxe,
		Direction = nil,
		TurnType = 1,
		HandAction = 1,
		Delay = 50,
		DistMin = 12,
		DistMax = 15
	};

	var animation = silent_attack.Animation;

	var anim_name = animation->GetName(silent_attack.Direction);
	var anim_start = animation->GetAnimationStart();
	var anim_begin = animation->GetAnimationBegin();
	var anim_end = animation->GetAnimationEnd();
	var anim_ending = animation->GetAnimationEnding();
	var play_time = silent_attack.Delay ?? 30;
	var distance_min = silent_attack.DistMin ?? 10;
	var distance_max = silent_attack.DistMax ?? (distance_min + 1);

	attacker->SetTurnForced(attacker->GetDir());
	if (silent_attack.TurnType)
	{
		attacker->SetTurnType(silent_attack.TurnType);
	}
	if (silent_attack.HandAction)
	{
		attacker->SetHandAction(silent_attack.HandAction);
		attacker->UpdateAttach();
	}

	victim->SetTurnForced(victim->GetDir());
	var anim_nr = attacker->PlayAnimation(anim_name, CLONK_ANIM_SLOT_Arms, Anim_Linear(anim_start, anim_begin, anim_end, play_time, anim_ending), Anim_Linear(0, 0, 1000, 10));

	var attack = CreateEffect(FxSilentAttack, 1, 1);
	if (attack)
	{
		attack.dir = attacker->GetDir();
		attack.attacker = attacker;
		attack.victim = victim;
		attack.time_strike = animation->GetStrikeTime(play_time); // Create an offset, so that the hit matches with the animation
		attack.time_stop = play_time;
		attack.anim = anim_nr;
		attack.constraint = CreateEffect(FxSpringConstraint, 1, 1, distance_min, distance_max)->SetBodyA(attacker)->SetBodyB(victim, true);
		attack.definition = animation;
	}
	
	// Update for better visuals
	attack.victim_plane = { During = attacker.Plane - attacker->GetCalcDir(), After = victim.Plane };
}


func DoSilentAttack(object victim, object attacker, proplist strike_animation)
{
	CauseDamage(victim, attacker->GetController());

	if (this.BreakOnMelee)
	{
		var x = victim->GetX();
		var y = victim->GetY();
		if (strike_animation)
		{
			var offset = strike_animation->GetStrikePosition();
			x += offset.X;
			y += offset.Y;
		}
		this->~EffectsOnBreak(x - GetX(), y - GetY(), attacker);
	}
}


func FinishSilentAttack(object victim, object attacker, int anim_nr, int dir, bool remove_item)
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

	if (this.BreakOnMelee)
	{
		this->Break(true);
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
		if (this.victim)
		{
			this.victim->SetComDir(COMD_Stop);
			if (this.victim_plane)
			{
				this.victim.Plane = this.victim_plane.During;
			}	
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
			Target->DoSilentAttack(this.victim, this.attacker, this.definition);
		}
	},

	Stop = func ()
	{
		if (this.constraint)
		{
			RemoveEffect(nil, nil, this.constraint);
		}

		if (this.victim && this.victim_plane)
		{
			this.victim.Plane = this.victim_plane.After;
		}

		Target->FinishSilentAttack(this.victim, this.attacker, this.anim_nr, this.dir);
	},
};
