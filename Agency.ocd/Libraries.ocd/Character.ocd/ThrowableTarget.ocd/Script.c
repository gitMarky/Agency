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


func GetThrowableHitEffects(object projectile)
{
	if (projectile->~IsThrowableWeapon())
	{
		if (projectile->~CausesLethalDamage())
		{
			StickThrowable(projectile);
		}
	}
}

func StickThrowable(object projectile)
{
	// TODO: More distinguished sticking? Hit from front or back makes a difference
	if (projectile->GetMeshMaterial())
	{
		var projectile_dir = Sign(projectile->GetXDir());
		var own_dir = this->~GetCalcDir();
		var hit_from_behind = projectile_dir == own_dir // Looks away from the projectile?
		                   || projectile_dir == 0;      // Be fair, and assume a projectile that comes directly from above as hitting from behind

		// Assume that we are being attached to a humanoid with Clonk skeleton, and that the item can be carried
		var bone = projectile->~GetCarryBone() ?? "main";
		projectile.property_weapon.throwable.mesh_nr = AttachMesh(projectile, "skeleton_head", bone, projectile->~GetThrowableHitTransform(hit_from_behind), AM_DrawBefore);
	}
	if (projectile.Collectible)
	{
		projectile->Enter(this);
	}
}

