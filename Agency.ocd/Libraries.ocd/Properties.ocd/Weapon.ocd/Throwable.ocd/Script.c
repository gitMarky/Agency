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
		target = nil,    // object: aim at this target instead
		precision = 100, // int:  angular precision
		max_strength = 450, // int: max throw strength
		throw_at = nil,     // effect: throw at specific target tracker
		procedure = nil,    // effect: throwing animation
	};
	return _inherited(by, ...);
}

/* --- Internal --- */

func SetThrowableAiming(bool is_aiming)
{
	property_weapon.throwable.aiming = is_aiming;
	// Reset the properties in either case
	// This ensures a clean state :)
	property_weapon.throwable.target = nil;
	property_weapon.throwable.throw_at = nil;
	property_weapon.throwable.procedure = nil;
}

/* --- Usage --- */

func HoldingEnabled() { return true; }


func RejectUse(object user)
{
	var can_use = user->~IsWalking();
	var is_throwing = !!property_weapon.throwable.procedure;
	return !can_use || is_throwing;
}


func ControlUseAltStart(object user, int x, int y)
{
	SetThrowableAiming(true);
	return true;
}


func ControlUseAltHolding(object user, int x, int y)
{
	ThrowAimAt(user, x, y, false);
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
		ThrowAimAt(user, x, y, true);
		ControlUseAltStop(user, x, y);
		return true;
	}
	return false;
}


/* --- Throwing logic --- */


func ThrowAimAt(object user, int x, int y, bool confirm_throw)
{
	SetController(user->GetOwner());

	// Calculation
	var precision_velocity = 100;
	var precision = property_weapon.throwable.precision;
	var angle_aim = Angle(0, 0, x, y, precision);
	//var angle_mod = 45 * precision * user->GetCalcDir(); // Aim 45 degree upwards by default
	var angle_mod = 0;
	var angle = Abs(Normalize(angle_aim - angle_mod, -180 * precision, precision));
	angle /= 3; // Aim 30° if you want to throw forward :O
	angle = Max(angle, 10 * precision);
	angle *= user->GetCalcDir();
	var distance = Distance(0, 0, x, y);
	//var dist_mod = 66 - 41 * BoundBy(distance - 25, 0, 155) / 155;
	var strength = BoundBy(distance * 5, property_weapon.throwable.max_strength / 10, property_weapon.throwable.max_strength);

	var exit_x = user->GetX() + user->GetCalcDir() * 4;
	var exit_y = user->GetY() + Cos(angle, -4, precision);
	var xdir = +Sin(angle, strength, precision);
	var ydir = -Cos(angle, strength, precision);
	
	// Throw boost (throws stronger upwards than downwards)
	if (ydir < 0) ydir = ydir * 13/10;
	if (ydir > 0) ydir = ydir *  8/10;

	// Add own velocity
	xdir += user->GetXDir(precision_velocity) / 2;
	ydir += user->GetYDir(precision_velocity) / 2;
	

	// TODO: Cycle through targets with W/S
	// TODO: Limit throwing distance somewhere
	var target = FindTarget(user, user->GetX() + x, user->GetY() + y);

	if (confirm_throw) // Actually throw the object
	{
		var delay = 20;
		var throw = 650;
		if (property_weapon.throwable.throw_at)
		{
			delay = 30;
			throw = 900;
		}
		property_weapon.throwable.procedure = CreateEffect(FxThrowAnimation, 1, 1, user, delay, throw)->SetData(exit_x, exit_y, xdir, ydir, precision_velocity);
	}
	else // Show and update trajectory preview
	{
		var precision_vdefault = 10;
		xdir = precision_vdefault * xdir / precision_velocity;
		ydir = precision_vdefault * ydir / precision_velocity;

		// Reset the target if there is no selection effect
		if (!property_weapon.throwable.throw_at)
		{
			property_weapon.throwable.target = nil;
		}
		// Assign a target and create selection effect
		if (target && !property_weapon.throwable.target)
		{
			property_weapon.throwable.target = target;
			property_weapon.throwable.throw_at = this->CreateEffect(FxThrowAtTarget, 1, 1, target);
		}
		// Trajectory should be displayed only if there is no marker on a target	
		if (property_weapon.throwable.throw_at)
		{
			Trajectory->Remove(user);
			// Remove the aim effect if you aim too far away
			var angle_t = Angle(GetX(), GetY(), property_weapon.throwable.target->GetX(), property_weapon.throwable.target->GetY(), precision);
			var angle_diff = Abs(Normalize(angle_t - angle_aim, -180 * precision, precision));
			if (angle_diff > 20 * precision)
			{
				RemoveEffect(nil, nil, property_weapon.throwable.throw_at);
			}
		}
		else
		{
			Trajectory->Create(user, exit_x, exit_y, xdir, ydir, nil, nil, true);
		}
	}
}


func ThrowWeapon(int exit_x, int exit_y, int xdir, int ydir, int precision_velocity)
{
	// Support for stackable objects
	var projectile = this->~TakeObject() ?? this;
	projectile->Exit();
	projectile->SetPosition(exit_x, exit_y);
	projectile->SetR(Random(360));
	projectile->SetRDir(RandomX(-30, 30));
	projectile->SetController(this->GetController());
	if (property_weapon.throwable.throw_at) // TODO: Re-assign is not possible, has to switch the logic?
	{
		//property_weapon.throwable.throw_at.Target = projectile;
	}
	else
	{
		projectile->SetSpeed(xdir, ydir, precision_velocity);
	}
}

local FxThrowAnimation = new Effect {
	Name = "ThrowAnimation",

	SetData = func (int exit_x, int exit_y, int xdir, int ydir, int precision_velocity)
	{
		this.exit_x = exit_x;
		this.exit_y = exit_y;
		this.xdir = xdir;
		this.ydir = ydir;
		this.precision_velocity = precision_velocity;
		return this;
	},

	Start = func (int temporary, object user, int play_time, int throw)
	{
		if (temporary)
		{
			return 0;
		}
		
		var animation = Strike_Animations.Throw;

		this.user = user;
		this.anim_nr = user->PlayAnimation(animation->GetName(), 
		                                   CLONK_ANIM_SLOT_Arms,
		                                   Anim_Linear(animation->GetAnimationStart(), 0, animation->GetAnimationLength(), play_time, animation->GetAnimationEnding()),
		                                   Anim_Linear(0, 0, 1000, 10));
		                                   
		this.time_throw = throw * play_time / animation->GetAnimationLength();
		this.time_stop = play_time;
	},

	Stop = func (int reason, bool temporary)
	{
		if (!temporary)
		{
			if (this.user)
			{
				this.user->StopAnimation(this.anim_nr);
			}
		}
	},

	Timer = func (int time)
	{
		if (time == this.time_throw)
		{
			this.Target->ThrowWeapon(this.exit_x, this.exit_y, this.xdir, this.ydir, this.precision_velocity);
		}
		if (time >= this.time_stop)
		{
			return FX_Execute_Kill;
		}
	},
};


func FindTarget(object user, int x, int y)
{
	// Convert to local coordinates
	x -= GetX();
	y -= GetY();
	// Criteria
	var area = Find_Or(Find_AtPoint(x, y), Find_Distance(15, x, y));
	var targets = FindObjects(area, Find_NoContainer(), Find_Func("IsThrowableTarget", this), Find_Exclude(user));
	var found = nil;
	var max_priority = 0;
	for (var target in targets)
	{
		// You need to have a clear line to the target
		if (!PathFree(user->GetX(), user->GetY(), target->GetX(), target->GetY()))
		{
			continue;
		}
		var priority = target->~GetThrowablePriority(this);
		if (found == nil || priority > max_priority)
		{
			found = target;
			max_priority = priority;
		}
	}
	return found;
}


local FxThrowAtTarget = new Effect
{
	Range = 150,
	Velocity = 150,
	Active = false,

	Start = func (int temporary, object target)
	{
		if (!temporary)
		{
			this.Victim = target;
			CreateSelector();
			CreateSelectorParticle();
		}
	},
	
	Stop = func (int reason, bool temporary)
	{
		if (!temporary)
		{
			if (this.Selector)
			{
				this.Selector->RemoveObject();
			}
		}
	},
	
	Timer = func (int time)
	{
		if (!this.Victim || ObjectDistance(this.Target, this.Victim) > this.Range)
		{
			return FX_Execute_Kill;
		}

		var precision = 10; // Calculate velocity and position in the same precision

		var offset = this.Victim->~GetThrowableOffset(precision);
		var victim_x = this.Victim->GetX(precision) + offset.X;
		var victim_y = this.Victim->GetY(precision) + offset.Y;
		
		if (this.Target->Contained())
		{
		}
		else
		{
			return CalculateVelocity(precision, victim_x, victim_y);
		}
	},

	CalculateVelocity = func (int precision, int victim_x, victim_y)
	{
		// Kill the effect if it hit the landscape or is too slow,
		// so that it does not become a bouncy homing projectile
		if (this.Active && this.Target->GetSpeed() < this.Velocity / 2)
		{
			return FX_Execute_Kill;
		}

		var projectile_x = this.Target->GetX(precision);
		var projectile_y = this.Target->GetY(precision);
		var target_x = victim_x - projectile_x;
		var target_y = victim_y - projectile_y;

		// Hit now (this frame)?
		if (Distance(target_x, target_y) <= this.Velocity)
		{
			if (PathFree(projectile_x / precision, projectile_y / precision, victim_x / precision, victim_y / precision))
			{
				// Add a basic velocity if you just exited the thrower
				if (!this.Active)
				this.Target->SetXDir(target_x, precision);
				// Teleport to the position, cause damage
				this.Target->SetPosition(victim_x / precision, victim_y / precision);
				this.Target->CauseDamage(this.Victim, this.Target->GetController());
				// Do the effects later, so that it can e.g. enter a dead target
				this.Victim->~GetThrowableHitEffects(this.Target);
				// Reset velocity last, so that it can be used as a directional marker during hit effects
				this.Target->SetXDir();
				this.Target->SetYDir();
			}
			else
			{
				// Cannot reach the target, collide with landscape or whatever comes next
			}
			return FX_Execute_Kill; // Either hit or unreachable, kill the effect in both cases
		}
		else
		{
			this.Active = true;
			// Adjust target position by velocity
			target_x += this.Victim->GetXDir(precision);
			target_y += this.Victim->GetYDir(precision);
			// Launch in that direction, homing.
			// The homing part should not be noticeable,
			// unless the target changes direction frequently
			// at a high velocity.
			var angle = Angle(0, 0, target_x, target_y, precision);
			this.Target->SetVelocity(angle, this.Velocity, precision, precision);
			return FX_OK;
		}
	},

	CreateSelector = func ()
	{
		if (this.Selector)
		{
			return;
		}

		var offset = this.Victim->~GetThrowableOffset() ?? { X = 0, Y = 0,};
		this.Selector = CreateObject(Dummy, 0, 0, this.Target->GetController());
		this.Selector->SetPosition(this.Victim->GetX(), this.Victim->GetY());
		this.Selector->SetVertex(0, VTX_X, -offset.X);
		this.Selector->SetVertex(0, VTX_Y, -offset.Y);
		this.Selector.ActMap =
		{
			Attach =
			{
				Name = "Attach",
				Procedure = DFA_ATTACH,
				FacetBase = 1
			}
		};
		this.Selector.Visibility = VIS_Owner;
		this.Selector.Plane = this.Victim.Plane + 10;
		this.Selector->SetAction("Attach", this.Victim);
	},
	
	CreateSelectorParticle = func ()
	{
		if (this.Selector)
		{
			// Draw a nice selector particle on item change.
			var size = 15;
			var selector =
			{
				Size = PV_Step(30, -1, 1, size),
				Attach = ATTACH_Front | ATTACH_MoveRelative,
				Rotation = PV_Step(1, PV_Random(0, 360), 1),
				Alpha = 200,
				R = 255, G = 255, B = 255, Alpha = 100,
			};

			this.Selector->CreateParticle("Selector", 0, 0, 0, 0, 0, selector, 1);
		}
	},
};
