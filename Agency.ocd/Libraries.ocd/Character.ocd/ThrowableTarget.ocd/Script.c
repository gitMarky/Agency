/**
	Marks a character as a target for throwables.
 */


/* --- Functions --- */

func IsThrowableTarget(object projectile)
{
	return this->GetAlive() || this->~IsPacified();
}


func GetThrowablePriority(object projectile)
{
	if (!projectile)
	{
		return 0;
	}

	var priority = 0;
	if (this->GetAlive()) // Prefer "alive" characters over pacified ones
	{
		priority += 10;
	}
	var direction = Sign(this->GetX() - projectile->GetX()); // Projectile has to be thrown in this direction
	if (direction == this->~GetCalcDir()) // Looking away from projectile? Prefer this guy!
	{
		priority += 10;
	}
	return priority;
}


func GetThrowableOffset(int precision)
{
	precision = precision ?? 1;
	return {X = 0, Y = -9 * precision, };
}

