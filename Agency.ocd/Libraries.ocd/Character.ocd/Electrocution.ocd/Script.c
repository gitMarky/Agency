/**
	Marks a character as electrocutable.
 */


func CanBeElectrocuted()
{
	return (this->~IsPacified() || this->GetAlive())
	    && !GetEffect(FxElectrocution.Name, this);
}

/* --- Functions --- */

func Electrocute()
{
	if (!GetEffect(FxElectrocution.Name, this))
	{
		Global->CreateEffect(FxElectrocution, 1, 1, this, this->~IsPacified());
	}
}


local FxElectrocution = new Effect
{
	TimeDeath = 20,
	TimeOver = 100,
	Name = "Electrocution",

	Start = func (int temporary, object victim, bool is_pacified)
	{
		if (!temporary)
		{
			var blend = Anim_Linear(0, 0, 1000, this.TimeDeath / 2, ANIM_Remove);
			var slot = CLONK_ANIM_SLOT_Death - 1;
			this.Horizontal = is_pacified;
			this.Victim = victim;
			if (is_pacified)
			{
				var end = this.Victim->GetAnimationLength("Dead");
				var begin = end - 400;
				this.Victim->PlayAnimation("Dead", slot, Anim_Linear(begin, begin, end, 3, ANIM_Loop), blend);
			}
			else
			{
				this.Victim->PlayAnimation("FallShort", slot, Anim_Linear(600, 600, 0, 3, ANIM_Loop), blend);
			}
			
			// Disable the target
			this.Victim->SetCrewEnabled(false);
			this.Victim->SetComDir(COMD_Stop);
			this.Victim->SetTurnForced(this.Victim->GetDir());
		}
	},

	Timer = func (int time)
	{
		if (time > this.TimeOver)
		{
			return FX_Execute_Kill;
		}
		if (time > this.TimeDeath)
		{
			Smolder();
			if (!Random(12))
			{
				Sparkle();
			}
		}
		else if (time == this.TimeDeath)
		{
			this.Victim->Kill(); // If this were this.Target the effect would vanish, too
			this.Horizontal = true; // Victim is lying down now
		}
		else
		{
			this.Victim->SetComDir(COMD_Stop);
		
			Flash();
			Sparkle();
		}
	},
	
	Flash = func ()
	{
		if (!Random(4))
		{
			this.Victim->CreateParticle("Flash", 0, 0, 0, 0, PV_Random(5, 10), { 
				Prototype = Particles_Flash(30),
				R = 0, G = 200, B = 255,
				Alpha = PV_KeyFrames(0, 0, 16, 250, 4, 1000, 0),
			});
			this.Victim->CreateLight(0, 0, 40, Fx_Light.LGT_Temp, nil, nil, RandomX(1, 3));
		}
		
	},
	
	Smolder = func ()
	{
		var attach = ATTACH_Back;
		if (Random(2))
		{
			attach = ATTACH_Front;
		}
		var grey = RandomX(180, 220);
		var particles = {
			Prototype = Particles_Smoke(),
			Attach = attach,
			Alpha = PV_Linear(150, 0),
			R = grey, G = grey, B = grey,
		};
		this.Victim->CreateParticle("Smoke", PV_Random(-5, +5), this.Victim->GetBottom() - 3, PV_Random(-1, 1), PV_Random(0, -2), PV_Random(30, 60), particles);
	},

	Sparkle = func ()
	{
		var prototype;
		if (Random(2))
		{
			prototype = Particles_ElectroSpark1();
		}
		else
		{
			prototype = Particles_ElectroSpark2();
		}
		var range_x;
		var range_y;
		var shape = this.Victim->GetShape();
		if (this.Horizontal)
		{
			range_x = shape[3] / 2;
			range_y = shape[2];
		}
		else
		{
			range_x = shape[2] / 2;
			range_y = shape[3];
		}
		var particles = 
		{
			Prototype = prototype, 
			Size = PV_Random(2, 6),
			Attach = ATTACH_Front,
			ForceY = PV_Gravity(500),
		OnCollision = PC_Bounce(500),
		};
		var bottom = this.Victim->GetBottom();
		this.Victim->CreateParticle("ElectroSpark", PV_Random(-range_x, +range_x), PV_Random(bottom, bottom - range_y), PV_Random(-3, 3), PV_Random(-15, -5), PV_Random(5, 10), particles);
	},
};
