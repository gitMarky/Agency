
global func CauseDamage(object victim, int by_player)
{
	if (GetType(this) != C4V_C4Object && GetType(this) != C4V_Def)
	{
		FatalError("Call from object or definition context only!");
	}
	if (GetType(this) == C4V_C4Object)
	{
		by_player = by_player ?? this->GetController();
	}
	AssertNotNil(by_player);
	AssertNotNil(victim);

	if (this->~CausesLethalDamage())
	{
		victim->SetKiller(by_player);
		victim->Kill();
	}
	else if (this->~CausesStunningDamage())
	{
		victim->SetKiller(by_player);
		victim->Pacify();
	}
}
