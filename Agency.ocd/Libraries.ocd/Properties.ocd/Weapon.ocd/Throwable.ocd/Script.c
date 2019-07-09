/**
	Marks an object as a throwable weapon.
 */

public func IsThrowableWeapon(){ return true; }

/* --- Properties --- */

local property_weapon;

func Construction(object by)
{
	property_weapon = property_weapon ?? {};
	property_weapon.throwable =
	{
		aiming = false,  // bool: aiming or not?
		angle = 0,       // int:  the aim angle, in precision
		strength = 0,    // int:  the strength
		target = nil,    // object: aim at this target instead
		precision = 100, // int:  angular precision
	};
	return _inherited(by, ...);
}

/* --- Internal --- */

func SetThrowableAiming(bool is_aiming)
{
	property_weapon.throwable.aiming = is_aiming;
	// Reset the properties in either case
	// This ensures a clean state :)
	property_weapon.throwable.angle = 0;
	property_weapon.throwable.strength = 0;
	property_weapon.throwable.target = nil;
}

/* --- Usage --- */

func HoldingEnabled() { return true; }


func RejectUse(object user)
{
	var can_use = user->~IsWalking();
	return !can_use;
}


func ControlUseAltStart(object user, int x, int y)
{
	SetThrowableAiming(true);
	return true;
}


func ControlUseAltHolding(object user, int x, int y)
{
	ThrowWeapon(user, x, y, false);
	return true;
}


func ControlUseAltCancel(object user, int x, int y)
{
	return ControlUseAltStop(user, x, y);
}


func ControlUseAltStop(object user, int x, int y)
{
	Trajectory->Remove(user);
	SetThrowableAiming(false);
	return true;
}


func ControlUseStart(object user, int x, int y)
{
	if (property_weapon.throwable.aiming)
	{
		ThrowWeapon(user, x, y, true);
		ControlUseAltStop(user, x, y);
		return true;
	}
	return false;
}

func ThrowWeapon(object user, int x, int y, bool do_throw)
{
	// Calculation
	var precision = property_weapon.throwable.precision;
	var angle = Normalize(Angle(0, 0, x, y, precision), -180 * precision, precision);
	var strength = Distance(0, 0, x, y);

	var exit_x = user->GetX();
	var exit_y = user->GetY();
	var xdir = +Sin(angle, strength, precision);
	var ydir = -Cos(angle, strength, precision);

	if (do_throw) // Actually throw the object
	{
		// Support for stackable objects
		var projectile = this->~TakeObject() ?? this;
		projectile->Exit();
		projectile->SetPosition(exit_x, exit_y);
		projectile->SetR(Random(360));
		projectile->SetSpeed(xdir, ydir);
		projectile->SetRDir(RandomX(-30, 30));
	}
	else // Show and update trajectory preview
	{
		Trajectory->Create(user, exit_x, exit_y, xdir, ydir, nil, nil, true);

		property_weapon.throwable.angle = angle;
		property_weapon.throwable.strength = strength;
		property_weapon.throwable.target = nil;
	}
}

