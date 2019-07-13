/**
	Marks an object as pacifyable.
 */


/* --- Functions --- */

func IsPacified()
{
	return GetEffect(FxPacification.Name, this);
}

func WakeUp()
{
	var pacified = IsPacified();
	if (pacified)
	{
		RemoveEffect(nil, nil, pacified);
		return true;
	}
	return false;
}

func Pacify()
{
	var pacified = IsPacified();
	if (pacified == nil)
	{
		CreateEffect(FxPacification, 1, 6);
		return true;
	}
	return false;
}

func GetAlive()
{
	if (this->~IsPacified())
	{
		return false;
	}
	else
	{
		return inherited(...); 
	}
}

func GetOCF()
{
	var ocf = inherited(...);

	// Remove alive flag if pacified
	if (this->IsPacified() && (ocf & OCF_Alive))
	{
		return ocf - OCF_Alive;
	}
	else
	{
		return ocf;
	}
}

/* --- Functionality --- */

local FxPacification = new Effect
{
	Name = "Pacification",

	Start = func (int temporary)
	{
		if (!temporary)
		{
			this.Target->~OnPacified();
			this.Particle = -1;
			this.Particles = 10;
		}
		return FX_OK;
	},

	Damage = func (int health_change, int cause, int by_player)
	{
		// Do not die, but do not heal either
		return 0;
	},

	// Status

	Stop = func (int reason, bool temporary)
	{
		if (temporary)
		{
			return;
		}
		this.Target->~OnWakeUp();
	},
	
	Timer = func (int time)
	{
		if (time < 30)
		{
			return FX_OK;
		}
		this.Particle = (this.Particle + 1) % this.Particles;
		var angle = 36 * this.Particle;
		var radius = 5;
		var lifetime = (this.Interval * this.Particles + 2) / 2;
		SpawnParticle(angle - 90, radius, lifetime);
		SpawnParticle(angle + 90, radius, lifetime);
	},

	SpawnParticle = func (int angle, int radius, int lifetime)
	{
		this.Target->CreateParticle("SphereSpark", Sin(-angle, radius), Cos(-angle, radius) - 3, 0, 0, lifetime, {Size = 2, Alpha = PV_Linear(255, 0), Attach = ATTACH_Front | ATTACH_MoveRelative});
	},
};


func OnPacified()
{
	// Cancel commands and such
	SetCommand("None");
	SetComDir(COMD_Stop);
	// Add animation
	CreateEffect(FxPacificationAnimation, 300, 1);
}

func OnWakeUp()
{
	// Remove animations
	RemoveEffect(FxPacificationAnimation.Name, this);
	// Get up!
	this->~DoKneel();

	// Temp: Remove icon
	Message("", this);
}


/* --- Better death animation --- */

func StartDead()
{
	StartDeathAnimation();
}

func StartDeathAnimation(int animation_slot)
{
	animation_slot = animation_slot ?? CLONK_ANIM_SLOT_Death;

	// Save animation slots
	var pacified = IsPacified();
	if (pacified)
	{
		pacified.animations = [];
	}

	// Blend death animation with other animations, except for the death slot
	var merged_animations = false;	
	for (var slot = 0; slot < animation_slot; ++slot)
	{
		if (GetRootAnimation(slot) == nil) continue;
		var anim = OverlayDeathAnimation(slot);
		merged_animations = true;

		if (pacified)
		{
			PushBack(pacified.animations, anim);
		}
	}

	// Force the death animation if there were no other animations active
	if (!merged_animations)
	{
		var anim = OverlayDeathAnimation(animation_slot);
		if (pacified)
		{
			PushBack(pacified.animations, anim);
		}
	}

	// Update carried items
	this->~UpdateAttach();
	// Set proper turn type
	this->~SetTurnType(1);
}

// Merges the animation in an animation slot with the death animation
// More variation is possible if we considered adding a random value for the length,
// or if we set the parameters according to current speed, etc.
func OverlayDeathAnimation(int slot, string animation)
{
	animation = animation ?? "Dead";
	return PlayAnimation(animation, slot, Anim_Linear(0, 0, GetAnimationLength(animation), 20, ANIM_Hold), Anim_Linear(0, 0, 1000, 10, ANIM_Hold));
}

/* --- Pacification animation --- */

local FxPacificationAnimation = new Effect
{
	Name = "FxPacificationAnimation",
	ActionName = "Pacified",

	Timer = func ()
	{
		// Force the animation while the Clonk is on the ground.
		// You can still hit them with rocks and the like (which looks funny)
		// Or fling them in the air from explosions...
		// But the Clonk will then return to the death animation
		// Lets at least get some feedback for this :D
		if (this.Target->GetAction() != this.ActionName
		&& (this.Target->GetContact(-1) || this.Target->InLiquid()))
		{
			RemoveExistingAnimation();
			this.Target->SetAction(this.ActionName);
			this.Target->StartDeathAnimation(CLONK_ANIM_SLOT_Death - 1);
		}
	},

	Destruction = func (int reason)
	{
		if (FX_Call_Normal == reason)
		{
			RemoveExistingAnimation();
		}
	},

	RemoveExistingAnimation = func ()
	{
		var pacified = this.Target->~IsPacified();
		if (pacified && pacified.animations) for (var anim in pacified.animations)
		{
			this.Target->StopAnimation(anim);
		}
	},
};

