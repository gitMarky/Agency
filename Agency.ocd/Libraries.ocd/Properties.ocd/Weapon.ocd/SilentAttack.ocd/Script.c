/**
	Marks an object act as a silent attack weapon.
 */

public func IsSilentAttackWeapon(){ return true; }


func DoSilentAttack(object victim, object agent)
{
	AssertNotNil(victim);
	AssertNotNil(agent);
	
	var anim_name = "StrikePickaxe";
	var anim_length = agent->GetAnimationLength(anim_name);
	var anim_start = 1500;
	var play_time = 50;

	agent->SetTurnType(1);
	agent->SetHandAction(1);
	agent->UpdateAttach();
	var anim_nr = agent->PlayAnimation(anim_name, CLONK_ANIM_SLOT_Arms, Anim_Linear(anim_start, 0, anim_length, play_time, ANIM_Loop), Anim_Linear(0, 0, 1000, 10));
	var attack = AddEffect("IntSilentAttack", agent, 1, 1, this);
	if (!attack) return false;
	attack.victim = victim;
	attack.x = victim->GetX();
	attack.y = victim->GetY() - 7; // Aim at the head!
	attack.time_strike = (3235 - anim_start) * play_time / anim_length;	// Create an offset, so that the hit matches with the animation
	attack.time_stop = (anim_length + 0) * play_time / anim_length;
	attack.anim = anim_nr;
}


func ResetSilentAttack(object agent, int anim_nr)
{
	agent->SetTurnType(0);
	agent->SetHandAction(false);
	agent->UpdateAttach();
	agent->StopAnimation(anim_nr ?? agent->GetRootAnimation(CLONK_ANIM_SLOT_Arms));
	RemoveEffect("IntSilentAttack", agent);
}


func DoSwing(object victim, object agent, int x, int y)
{
	if (this->~CausesLethalDamage())
	{
		victim->SetKiller(agent->GetOwner());
		victim->Kill();
	}
}


func FxIntSilentAttackTimer(object agent, proplist effect, int time)
{
	if (time >= effect.time_strike && !effect.struck) // Waits three seconds for animation to run (we could have a clonk swing his pick 3 times)
	{
		effect.struck = true;
		DoSwing(effect.victim, agent, effect.x, effect.y);
	}
	
	if (time >= effect.time_stop)
	{
		ResetSilentAttack(agent, effect.anim_nr);
		return FX_Execute_Kill;
	}
}
