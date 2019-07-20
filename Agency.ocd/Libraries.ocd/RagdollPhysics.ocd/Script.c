/** 
	Ragdoll Library

	@credits Based on Randrians Rope library
*/

// Some constants for the method.
static const LIB_RAGDOLL_Iterations = 10;
static const LIB_RAGDOLL_Precision = 100;
static const LIB_RAGDOLL_LandscapeMoveDirs = [[0, 1], [1, 0], [1, 1], [0, 2], [1, 2], [2, 0], [2, 1], [2, 2], [0, 3], [1, 3], [2, 3], [3, 0], [3, 1], [3, 2], [0, 4], 
                                              [1, 4], [2, 4], [3, 3], [4, 0], [4, 1], [4, 2], [0, 5], [1, 5], [2, 5], [3, 4], [3, 5], [4, 3], [4, 4], [5, 0], [5, 1], 
                                              [5, 2], [5, 3], [0, 6], [1, 6], [2, 6], [3, 6], [4, 5], [5, 4], [6, 0], [6, 1], [6, 2], [6, 3], [0, 7], [1, 7], [2, 7], 
                                              [3, 7], [4, 6], [5, 5], [5, 6], [6, 4], [6, 5], [7, 0], [7, 1], [7, 2], [7, 3], [0, 8], [1, 8], [2, 8], [3, 8], [4, 7],
                                              [4, 8], [5, 7], [6, 6], [7, 4], [7, 5], [8, 0], [8, 1], [8, 2], [8, 3], [8, 4], [0, 9], [1, 9], [2, 9], [3, 9], [4, 9],
                                              [5, 8], [6, 7], [7, 6], [7, 7], [8, 5], [9, 0], [9, 1], [9, 2], [9, 3], [9, 4]
                                             ];

// Internal variable of this library.
local ragdoll_particles = [];
local ragdoll_constraints = [];

static const Ragdoll_Particle = new Global
{
	// --- Properties
	x = 0,
	y = 0,
	oldx = 0,
    oldy = 0,
    accx = 0,
    accy = 0,
    mass = 1,
    friction = 0,
	name = nil, // Name
    children = [], // Array: Indices of the child particles in the array

	// --- Getters
	
	GetName =     func () { return name; },
	GetMass =     func () { return mass; },
	GetFriction = func () { return friction; },

    GetPosX = func () { return x; },
    GetOldX = func () { return oldx;},
    GetAccX = func () { return accx;}, 

    GetPosY = func () { return y; },
    GetOldY = func () { return oldy;},
    GetAccY = func () { return accy;}, 
    
    GetObjX = func () { return GetObjPos(x); },
    GetObjY = func () { return GetObjPos(y); },
    GetObjPos = func (int pos) { return (pos + LIB_RAGDOLL_Precision / 2) / LIB_RAGDOLL_Precision; },
    GetRelPos = func (int pos, int ref) { return GetObjPos(pos) - ref; },

    // --- Setters

	SetName =     func (int value) { name = value; },
	SetMass =     func (int value) { friction = value; },
	ApplyFriction = func () { friction = 1; },
	ResetFriction = func () { friction = 0; },

    SetPosX = func (int value) { x = value; },
    SetOldX = func (int value) { oldx = value;},
    SetAccX = func (int value) { accx = value;}, 

    SetPosY = func (int value) { y = value; },
    SetOldY = func (int value) { oldy = value;},
    SetAccY = func (int value) { accy = value;}, 
};

static const Ragdoll_Constraint_Object = new Global
{
	Target = nil,
	Particle = nil,
	
	Create = func (object target, proplist particle)
	{
		return new Ragdoll_Constraint_Object {
			Target = target->ObjectNumber(),
			Particle = particle,
		};
	},

	Resolve = func (int index, int obj_index)
	{
		var target = Object(Target);
		if (target->Contained()) 
		{
			target = target->Contained();
		}
		Particle->SetPosX(target->GetX(LIB_RAGDOLL_Precision));
		Particle->SetPosY(target->GetY(LIB_RAGDOLL_Precision));
	},
};

static const Ragdoll_Constraint_Distance = new Global
{
	ParticleA = nil,
	ParticleB = nil,
	MinDistance = nil,
	MaxDistance = nil,

	Create = func (proplist particle_a, proplist particle_b, int min_dist, int max_dist)
	{
		return new Ragdoll_Constraint_Distance
		{
			ParticleA = particle_a,
			ParticleB = particle_b,
			MinDistance = min_dist,
			MaxDistance = max_dist,
		};
	},

	CreateFixedDistance = func (proplist particle_a, proplist particle_b, int dist, bool precise)
	{
		if (!precise)
		{
			dist *= LIB_RAGDOLL_Precision;
		}
		var variance = LIB_RAGDOLL_Precision / 2;
		return Create(particle_a, particle_b, dist - variance, dist + variance);
	},
	
	Resolve = func ()
	{
		// Calculate difference.
		var delta_length = Distance(ParticleA->GetPosX(), ParticleA->GetPosY(), ParticleB->GetPosX(), ParticleB->GetPosY());
		
		if (MinDistance != nil && delta_length < MinDistance)
		{
			SetDistance(MinDistance, delta_length);
		}
		if (MaxDistance != nil && delta_length > MaxDistance)
		{
			SetDistance(MaxDistance, delta_length);
		}
	},

	SetDistance = func (int target_length, int delta_length)
	{
		// Prevent division by zero
		if (delta_length == 0)
		{
			delta_length = 1;
		}
		// Get coordinates and inverse masses.
		var delta_x = ParticleB->GetPosX() - ParticleA->GetPosX();
		var delta_y = ParticleB->GetPosY() - ParticleA->GetPosY();
		var invmassA = ParticleA->GetMass();
		var invmassB = ParticleB->GetMass();
		var diff = (delta_length - target_length);
		var mass = Max(1, invmassA + invmassB);
		delta_x = delta_x * diff / (delta_length * mass);
		delta_y = delta_y * diff / (delta_length * mass);
		// Set new positions.
		ParticleA->SetPosX(ParticleA->GetPosX() + delta_x * invmassA);
		ParticleA->SetPosY(ParticleA->GetPosY() + delta_y * invmassA);
		ParticleB->SetPosX(ParticleB->GetPosX() - delta_x * invmassB);
		ParticleB->SetPosY(ParticleB->GetPosY() - delta_y * invmassB);
	},
};

static const Ragdoll_Constraint_Landscape = new Global
{
	Particle = nil,
	
	Create = func (proplist particle)
	{
		return new Ragdoll_Constraint_Landscape
		{
			Particle = particle,
		};
	},
	
	Resolve = func ()
	{
		if (GBackSolid(Particle->GetObjX(), Particle->GetObjY()))
		{
			// Moving left?
			var move_x = -1;
			if (Particle->GetPosX() < Particle->GetOldX())
			{
				move_x = 1;
			}
			// Moving up?
			var move_y = -1;
			if (Particle->GetPosY() < Particle->GetOldY())
			{
				move_y = 1;
			}
			// Look for all possible places where the particle could move (from nearest to furthest).
			move_x *= LIB_RAGDOLL_Precision;
			move_y *= LIB_RAGDOLL_Precision;
			for (var pos in LIB_RAGDOLL_LandscapeMoveDirs)
			{
				var new_x = Particle->GetPosX() + move_x * pos[0];
				var new_y = Particle->GetPosY() + move_y * pos[1];
				if (GBackSolid(Particle->GetObjPos(new_x), Particle->GetObjPos(new_y)))
				{
					continue;
				}

				// Calculate the new position (if we don't move in a direction don't overwrite the old value).
				if (pos[0])
				{
					Particle->SetPosX(new_x);
				}
				if (pos[1])
				{
					Particle->SetPosY(new_y);
				}
				// Notifier for applying friction after the constraints.
				Particle->ApplyFriction();
				break;
			}
		}
	},
};

func GetRagdollGravity()
{
	return GetGravity() * LIB_RAGDOLL_Precision / 100;
}

func CreateRagdollParticle(string name, int x, int y, bool precise, bool gravity)
{
	var precision = 1;
	if (precise)
	{
		precision = LIB_RAGDOLL_Precision;
	}
	var part_x = GetX(LIB_RAGDOLL_Precision) + (x * LIB_RAGDOLL_Precision / precision);
	var part_y = GetY(LIB_RAGDOLL_Precision) + (y * LIB_RAGDOLL_Precision / precision);
	var particle = new Ragdoll_Particle {};
	particle->SetName(name);
	particle->SetPosX(part_x);
	particle->SetOldX(part_x);
	particle->SetPosY(part_y);
	particle->SetOldY(part_y);
	if (gravity)
	{
		particle->SetAccY(GetRagdollGravity());
	}
	return particle;
}

func AddRagdollParticle(proplist particle)
{
	RemoveHoles(ragdoll_particles);
	PushBack(ragdoll_particles, particle);
}

func AddRagdollConstraint(proplist constraint)
{
	PushBack(ragdoll_constraints, constraint);
}

func GetRagdollParticle(string name)
{
	for (var particle in ragdoll_particles)
	{
		if (particle->GetName() == name)
		{
			return particle;
		}
	}
	return nil;
}

/* --- Calculation --- */

// The procedure of a time step. Should be called with a timercall or an effect!
func RagdollTimeStep()
{
	RagdollVerlet();
	RagdollSatisfyConstraints();
	RagdollFriction();
	RagdollForcesOnObjects();

	this->~UpdateRagdoll();
}


/**
	Verlet integration step
	Moves the particles according to their old position and thus speed.
*/
func RagdollVerlet()
{
	// Copy Position of the objects
	//var j = 0;
	//for (var i = 0; i < 2; i++ || j--)
	//{
	//	SetParticleToObject(j, i);
	//}

	// Verlet
	for (var particle in ragdoll_particles)
	{
		// Verlet step, get speed out of distance moved relative to the last position.
		var pos_x = particle->GetPosX();
		var pos_y = particle->GetPosY();
		var vel_x = pos_x - particle->GetOldX();
		var vel_y = pos_y - particle->GetOldY();
		
		// Update
		particle->SetPosX(pos_x + vel_x + particle->GetAccX());
		particle->SetPosY(pos_y + vel_y + particle->GetAccY());
		particle->SetOldX(pos_x);
		particle->SetOldY(pos_y);
		particle->ResetFriction();
	}
}


/**
	Satisfying the constraints for the particles. The constraints are:
	- Staying at the position of the objects,
	- respecting the length to the next particles,
	- staying out of material.
*/
func RagdollSatisfyConstraints()
{
	for (var j = 0; j < LIB_RAGDOLL_Iterations; ++j)
	{
		for (var constraint in ragdoll_constraints)
		{
			constraint->Resolve();
		}
	}
}


/**
	Apply friction for those who have the notifier for it.
	Friction just means that the velocity is divided by 2 to simulate a frictional force.
*/
func RagdollFriction()
{
	for (var particle in ragdoll_particles)
	{
		if (particle->GetFriction())
		{
			particle->SetOldX((particle->GetOldX() + particle->GetPosX()) / 2);
			particle->SetOldY((particle->GetOldY() + particle->GetPosY()) / 2);
		}
	}
}


/**
	Applies the forces on the objects (only non-fixed ones) or adjust the length then the object pulls
*/
func RagdollForcesOnObjects()
{
/*
	var speed = Distance(lib_ragdoll_particles[-1].x, lib_ragdoll_particles[-1].y, lib_ragdoll_particles[-1].oldx ,lib_ragdoll_particles[-1].oldy);
	if (lib_ragdoll_length == GetMaxLength())
	{
		if (ObjContact(lib_ragdoll_objects[1][0]))
			speed = 40;
		else
			speed = 100;
	}
	
	var j = 0;
	if (PullObjects())
	{
		for (var i = 0; i < 2; i++)
		{
			if (i == 1)
			{
				j = lib_ragdoll_particle_count - 1;
			}
			var obj = lib_ragdoll_objects[i][0];
	
			if (obj == nil || !lib_ragdoll_objects[i][1])
			{ 
				continue;
			}
	
			if (obj->Contained())
			{
				obj = obj->Contained();
			}
	
			if (obj->GetAction() == "Walk" || obj->GetAction() == "Scale" || obj->GetAction() == "Hangle" || obj->GetAction() == "Climb")
			{
				obj->SetAction("Jump");
			}

			obj->SetXDir(lib_ragdoll_particles[j].x - lib_ragdoll_particles[j].oldx, LIB_RAGDOLL_Precision);
			obj->SetYDir(lib_ragdoll_particles[j].y - lib_ragdoll_particles[j].oldy, LIB_RAGDOLL_Precision);
		}
	}
	return;
*/
}


/**
	Sums all the forces on the segments
	These are gravity and for connect \a loose mode this is also a straightening of the rope.
	Only called when in connect \a loose mode. NOT USED AT THE MOMENT!
*/
/*
public func AccumulateForces()
{
	for (var i = 1; i < lib_ragdoll_particle_count; i++)
	{
		var fx = 0, fy = 0, angle;
		if (i < lib_ragdoll_particle_count - 2)
		{
			angle = Angle(lib_ragdoll_particles[i].x, lib_ragdoll_particles[i].y, lib_ragdoll_particles[i + 1].x, lib_ragdoll_particles[i + 1].y);
			fx = Sin(angle, 5 * LIB_RAGDOLL_Precision);
			fy = -Cos(angle, 5 * LIB_RAGDOLL_Precision);
		}
		lib_ragdoll_particles[i].accx = fx;
		lib_ragdoll_particles[i].accy = fy + GetRagdollGravity();
	}
}
*/
