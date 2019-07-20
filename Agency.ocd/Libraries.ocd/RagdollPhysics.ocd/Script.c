/** 
	Ragdoll Library

	@credits Based on Randrians Rope library
*/

// Some constants for the method.
static const LIB_RAGDOLL_Iterations = 10;
static const LIB_RAGDOLL_Precision = 100;
static const LIB_RAGDOLL_SegmentLength = 5;
static const LIB_RAGDOLL_LandscapeMoveDirs = [[0, 1], [1, 0], [1, 1], [0, 2], [1, 2], [2, 0], [2, 1], [2, 2], [0, 3], [1, 3], [2, 3], [3, 0], [3, 1], [3, 2], [0, 4], 
                                              [1, 4], [2, 4], [3, 3], [4, 0], [4, 1], [4, 2], [0, 5], [1, 5], [2, 5], [3, 4], [3, 5], [4, 3], [4, 4], [5, 0], [5, 1], 
                                              [5, 2], [5, 3], [0, 6], [1, 6], [2, 6], [3, 6], [4, 5], [5, 4], [6, 0], [6, 1], [6, 2], [6, 3], [0, 7], [1, 7], [2, 7], 
                                              [3, 7], [4, 6], [5, 5], [5, 6], [6, 4], [6, 5], [7, 0], [7, 1], [7, 2], [7, 3], [0, 8], [1, 8], [2, 8], [3, 8], [4, 7],
                                              [4, 8], [5, 7], [6, 6], [7, 4], [7, 5], [8, 0], [8, 1], [8, 2], [8, 3], [8, 4], [0, 9], [1, 9], [2, 9], [3, 9], [4, 9],
                                              [5, 8], [6, 7], [7, 6], [7, 7], [8, 5], [9, 0], [9, 1], [9, 2], [9, 3], [9, 4]
                                             ];

// Internal variable of this library.
local lib_ragdoll_length;         // Length of the rope.
local lib_ragdoll_particle_count; // Number of rope particles.
local lib_ragdoll_particles;      // List of rope particles.
local lib_ragdoll_segments;       // List of rope segments.
local lib_ragdoll_objects;        // List of rope objects, first entry is the start of the rope, second entry is the end of the rope; format for entry: [object, bool fixed].
local lib_ragdoll_max_length;     // Maximum rope length.

public func GetRagdollGravity()
{
	return GetGravity() * LIB_RAGDOLL_Precision / 100;
}

/** Starts a rope
* The rope object itself is used as first pole where the rope is connected to.
* @param obj1 The first object
* @param obj2 The second object
*/
protected func StartRope()
{
	lib_ragdoll_objects = [[this, false], [nil, false]];
	lib_ragdoll_length = LIB_RAGDOLL_SegmentLength;

	lib_ragdoll_particle_count = 1;
	lib_ragdoll_segments = [];
	lib_ragdoll_segments[0] = CreateSegment(0, nil);

	lib_ragdoll_particles = [];
	lib_ragdoll_particles[0] = 
	{
		x = GetX() * LIB_RAGDOLL_Precision,
		y = GetY() * LIB_RAGDOLL_Precision,
		oldx = (GetX() + 1) * LIB_RAGDOLL_Precision, 
	    oldy = GetY() * LIB_RAGDOLL_Precision,
	    accx = 0,
	    accy = GetRagdollGravity(),
	    mass = 0
    };
	return;
}

/** Connects \a obj1 and \a obj2
* Connects the two objects with a rope. Should be only used, when the path between the objects is free, so that the line doesn't go throught material.
* By default the first objects is set to fixed (no force is applied on it) and the second to loose (the line can affect the position or the position can affect the line's length)
* @param obj1 The first object
* @param obj2 The second object
*/
public func StartRopeConnect(object obj1, object obj2)
{
	lib_ragdoll_length = ObjectDistance(obj1, obj2);
	lib_ragdoll_objects = [[obj1, false], [obj2, true]];
	lib_ragdoll_particle_count = lib_ragdoll_length / LIB_RAGDOLL_SegmentLength;

	var yoff = 0;
	if (lib_ragdoll_particle_count < 2)
	{
		lib_ragdoll_particle_count = 2;
		yoff = 1;
		lib_ragdoll_length = 10;
	}

	lib_ragdoll_segments = [];
	for (var i = 0; i < lib_ragdoll_particle_count; i++)
	{
		var prev = nil;
		if (i > 0) 
		{
			prev = lib_ragdoll_segments[i-1];
		}
		lib_ragdoll_segments[i] = CreateSegment(i, prev);
	}

	lib_ragdoll_particles = [];
	var x, y;
	for (var i = 0; i < lib_ragdoll_particle_count; i++)
	{
		x = obj1->GetX(LIB_RAGDOLL_Precision) * (lib_ragdoll_particle_count - i) / lib_ragdoll_particle_count + obj2->GetX(LIB_RAGDOLL_Precision) * i / lib_ragdoll_particle_count;
		y = obj1->GetY(LIB_RAGDOLL_Precision) * (lib_ragdoll_particle_count - i) / lib_ragdoll_particle_count + obj2->GetY(LIB_RAGDOLL_Precision) * i / lib_ragdoll_particle_count;
		y += yoff * i;
		// Pos, Oldpos, acceleration (gravity), mass.
		lib_ragdoll_particles[i] =
		{
			x = x,
			y = y,
			oldx = x,
			oldy = y,
			accx = 0,
			accy = GetRagdollGravity(),
			mass = 1,
		}; 
	}
	lib_ragdoll_particles[0].accx = 0;
	lib_ragdoll_particles[0].accy = 0;
	lib_ragdoll_particles[0].mass = 0;
	lib_ragdoll_particles[-1].accx = 0;
	lib_ragdoll_particles[-1].accy = 0;
	lib_ragdoll_particles[-1].mass = 1;

	UpdateSegmentOverlays();
	TimeStep();
	return;
}

protected func Destruction()
{
	RemoveRope();
}

public func SetMaxLength(int newlength)
{
	lib_ragdoll_max_length = newlength;
}

public func GetMaxLength()
{
	return lib_ragdoll_max_length;
}

// Removes the rope: All segments are removed. This should be called to clear the rope.
public func RemoveRope()
{
	if (lib_ragdoll_segments)
		for (var segment in lib_ragdoll_segments)
			DeleteSegment(segment);
	return;
}

/** Sets the fixed status of the two targets
* When a target is fixed, it only serves as a fixed starting or ending point for the rope. No force is applied on the object.
* For a non fixed target force is applied or in case of a rope that is set to ConnectLoose the strength with wich the object pulls affects the length of the rope.
* @param fixed_1 whether object1 shall be fixed
* @param fixed_2 whether object2 shall be fixed
*/
public func SetFixed(bool fixed_1, bool fixed_2)
{
	lib_ragdoll_objects[0][1] = !fixed_1;
	lib_ragdoll_objects[1][1] = !fixed_2;
	lib_ragdoll_particles[ 0].mass = lib_ragdoll_objects[0][1];
	lib_ragdoll_particles[-1].mass = lib_ragdoll_objects[1][1];
}


/** Create a new segment
* This function should be \a overloaded so that the rope can create it's own specific objects
* @param index the index of the new segment
* @param previous the previous segment (in case it needs to be notified)
*/
/* To be overloaded for special segment behaviour */
private func CreateSegment(int index, object previous) { }

/** Remove a new segment
* Can be overloaded, when the segments require special deletion behaviour (e.g. notify other segments)
* @param segment the segment to be removed
* @param previous the previous segment (in case it needs to be notified)
*/
private func DeleteSegment(object segment, object previous)
{
	if (segment)
		segment->RemoveObject();
}

/** Callback when the rope has lost it's last segment with \c RemoveSegment
*/
/* When the last segment is removed */
private func RopeRemoved() { }

/** Adds a new Segment to the rope and increases the length of the rope
* @param xoffset x offset of the newly inserted segment
* @param yoffset y offset of the newly inserted segment
*/
/* Adding and removing segments */
public func AddSegment(int xoffset, int yoffset)
{
	lib_ragdoll_segments[lib_ragdoll_particle_count] = CreateSegment(lib_ragdoll_particle_count, lib_ragdoll_segments[lib_ragdoll_particle_count - 1]);
	var oldx = lib_ragdoll_particles[lib_ragdoll_particle_count - 1].oldx;
	var oldy = lib_ragdoll_particles[lib_ragdoll_particle_count - 1].oldy;
	lib_ragdoll_particles[lib_ragdoll_particle_count] =
	{
		x = oldx + xoffset,
		y = oldy + yoffset,
		oldx = oldx,
		oldy = oldy,
		accx = 0,
		accy = GetRagdollGravity(),
		mass = 1,
	};	
	lib_ragdoll_particle_count++;
	lib_ragdoll_length += LIB_RAGDOLL_SegmentLength;
	UpdateSegmentOverlays();
	return;
}


/**
	Callback, when the maximal length of the rope is reached
*/
public func MaxLengthReached() { }


/**
	Returns the length of the last segment (other segments have \c LIB_RAGDOLL_SegmentLength length)
	@return the length of the last segment
*/
public func GetLastLength()
{
	return lib_ragdoll_length * LIB_RAGDOLL_Precision - LIB_RAGDOLL_SegmentLength * LIB_RAGDOLL_Precision * (lib_ragdoll_particle_count - 1);
}


/**
	This is called when a new segment is added, the segments can adjust their appeareance to that
	Should be \b overloaded by the object.
*/
private func UpdateSegmentOverlays() { }


/** 
	Shall display the rope (e.g. rotate the semgents to fit the rope), called every frame
	Should be \b overloaded by the object.
*/
private func UpdateLines() {}

// The procedure of a time step. Should be called with a timercall or an effect!
public func TimeStep()
{
	Verlet();
	SatisfyConstraints();
	ForcesOnObjects();
	UpdateLines();
	return;
}

/** Summs all the fores on the segments
* These are gravity and for connect \a loose mode this is also a straightening of the rope.
* Only called when in connect \a loose mode. NOT USED AT THE MOMENT!
*/
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
	return;
}

/** Verlet integration step
* Moves the particles according to their old position and thus speed.
*/
private func Verlet()
{
	// Copy Position of the objects
	var j = 0;
	for (var i = 0; i < 2; i++ || j--)
		SetParticleToObject(j, i);

	// Verlet
	var start = 1;
	for (var i = start; i < lib_ragdoll_particle_count; i++)
	{
		var part = lib_ragdoll_particles[i];
		var temp_x = part.x;
		var temp_y = part.y;
		// Verlet step, get speed out of distance moved relativ to the last position.
		lib_ragdoll_particles[i].x += part.x - part.oldx + part.accx;
		lib_ragdoll_particles[i].y += part.y - part.oldy + part.accy;
		lib_ragdoll_particles[i].oldx = temp_x;
		lib_ragdoll_particles[i].oldy = temp_y;
		lib_ragdoll_particles[i].friction = 0;
	}
	return;
}

/** Moves a particle to the position of the object
* @param index the index of the particle to be moved
* @param obj_index the index of the object to be moved
*/
public func SetParticleToObject(int index, int obj_index)
{
	var obj = lib_ragdoll_objects[obj_index][0];
	if (!obj) 
		return;

	if (obj->Contained()) 
		obj = obj->Contained();
	lib_ragdoll_particles[index].x = obj->GetX(LIB_RAGDOLL_Precision);
	lib_ragdoll_particles[index].y = obj->GetY(LIB_RAGDOLL_Precision);
	return;
}

// Satisfying the constraints for the particles. The constraints are: Staying at the position of the objects,
// respecting the length to the next particles and staying out of material.
private func SatisfyConstraints()
{
	for (var j = 0; j < LIB_RAGDOLL_Iterations; j++)
	{
		ConstraintObjects();
		ConstraintLength();
		ConstraintLandscape();
	}
	// Apply friction for those who have the notifier for it.
	// Friction just means that the velocity is divided by 2 to simulate a frictional force.
	for (var i = 0; i < lib_ragdoll_particle_count; i++)
	{
		if (!lib_ragdoll_particles[i].friction)
			continue;
		lib_ragdoll_particles[i].oldx = (lib_ragdoll_particles[i].oldx + lib_ragdoll_particles[i].x) / 2;
		lib_ragdoll_particles[i].oldy = (lib_ragdoll_particles[i].oldy + lib_ragdoll_particles[i].y) / 2;
	}
	return;
}

public func ConstraintObjects()
{
	// Copy position of the objects.
	if (lib_ragdoll_length < GetMaxLength())
		for (var i = 0, j = 0; i < 2; i++ || j--)
			SetParticleToObject(j, i);
	return;
}

public func ConstraintLength()
{
	// Satisfy all stick constraints (move the particles to fit the length).
	var normal_restlength = LIB_RAGDOLL_SegmentLength * LIB_RAGDOLL_Precision;
	var restlength, invmass1, invmass2, delta1, delta2, delta_length;
	for (var i = 0; i < lib_ragdoll_particle_count - 1; i++)
	{
		// Keep length, normal length between two points.
		restlength = normal_restlength;
		if (i == lib_ragdoll_particle_count - 2)
			restlength = GetLastLength();
		// Calculate difference.
		delta1 = lib_ragdoll_particles[i + 1].x - lib_ragdoll_particles[i].x;
		delta2 = lib_ragdoll_particles[i + 1].y - lib_ragdoll_particles[i].y;
		delta_length = Sqrt(delta1**2 + delta2**2);
		if (delta_length < restlength)
			continue;			
		// Get coordinates and inverse masses.
		invmass1 = lib_ragdoll_particles[i].mass;
		invmass2 = lib_ragdoll_particles[i + 1].mass;
		delta1 = delta1 * (delta_length - restlength) / (delta_length * (invmass1 + invmass2));
		delta2 = delta2 * (delta_length - restlength) / (delta_length * (invmass1 + invmass2));
		// Set new positions.
		lib_ragdoll_particles[i    ].x += delta1 * invmass1;
		lib_ragdoll_particles[i    ].y += delta2 * invmass1;
		lib_ragdoll_particles[i + 1].x -= delta1 * invmass2;
		lib_ragdoll_particles[i + 1].y -= delta2 * invmass2;
	}
	return;
}

public func ConstraintLandscape()
{
	for (var i = 0; i < lib_ragdoll_particle_count; i++)
	{
		// Don't touch the ground.
		if (GBackSolid(GetPartX(i) - GetX(), GetPartY(i) - GetY()))
		{
			// Moving left?
			var xdir = -1;
			if (lib_ragdoll_particles[i].x < lib_ragdoll_particles[i].oldx)
				xdir = 1;
			var ydir = -1;
			// Moving up?
			if (lib_ragdoll_particles[i].y < lib_ragdoll_particles[i].oldy)
				ydir = 1;
			var found = false;
			// Look for all possible places where the particle could move (from nearest to furthest).
			for (var pos in LIB_RAGDOLL_LandscapeMoveDirs)
			{
				if (!GBackSolid(GetPartX(i) - GetX() + xdir * pos[0], GetPartY(i) - GetY() + ydir * pos[1]))
				{
					// Calculate the new position (if we don't move in a direction don't overwrite the old value).
					if (pos[0])
						lib_ragdoll_particles[i].x = (GetPartX(i) + xdir * pos[0]) * LIB_RAGDOLL_Precision - xdir * LIB_RAGDOLL_Precision / 2 + xdir;
					if (pos[1])
						lib_ragdoll_particles[i].y = (GetPartY(i) + ydir * pos[1]) * LIB_RAGDOLL_Precision - ydir * LIB_RAGDOLL_Precision / 2 + ydir;
					// Notifier for applying friction after the constraints.
					lib_ragdoll_particles[i].friction = 1; 
					found = true;
					break;
				}
			}
			// No possibility to move the particle out? Then reset it. The old position should be valid.
			if (!found)
			{
				lib_ragdoll_particles[i].x = lib_ragdoll_particles[i].oldx;
				lib_ragdoll_particles[i].y = lib_ragdoll_particles[i].oldy;	
			}
		}
	}
	return;
}

// Returns the length of the rope.
public func GetLineLength()
{
	var length_vertex = 0;
	for (var i = 1; i < lib_ragdoll_particle_count; i++)
		length_vertex += Distance(lib_ragdoll_particles[i].x, lib_ragdoll_particles[i].y, lib_ragdoll_particles[i - 1].x, lib_ragdoll_particles[i - 1].y);
	return length_vertex;
}

public func LengthAutoTryCount() { return 5; }

/** Applies the forces on the objects (only non-fixed ones) or adjust the length then the object pulls
*/
public func ForcesOnObjects()
{
	if (!lib_ragdoll_length)
		return;

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
}

public func PullObjects()
{
	return !lib_ragdoll_length == GetMaxLength();
}

public func ObjContact(obj)
{
	if (obj->Contained()) 
		obj = obj->Contained();
	if (obj->GetContact(-1)) 
		return true;
	return false;
}


/*-- Vector Operations --*/

// Addition of two vectors.
public func Vec_Add(array x, array y) { return [x[0] + y[0], x[1] + y[1]]; }

// Subtraction of two vectors.
public func Vec_Sub(array x, array y) { return [x[0] - y[0], x[1] - y[1]]; }

// Multiplication of a vector by a number.
public func Vec_Mul(array x, int i) { return [x[0] * i, x[1] * i]; }

// Division of a vector by a number.
public func Vec_Div(array x, int i) { return [x[0] / i, x[1] / i]; }

// Dot product of two vectors.
public func Vec_Dot(array x, array y) { return x[0] * y[0] + x[1] * y[1]; }

// Length of a vector.
public func Vec_Length(array x) { return Sqrt(x[0]**2 + x[1]**2); }

// Angle between two vectors.
public func Vec_Angle(array x, array y) { return Angle(x[0], x[1], y[0], y[1]); }

// Normalizes a vector to the given precision.
public func Vec_Normalize(array x, int precision) { return Vec_Div(Vec_Mul(x, precision), Vec_Length(x)); }

// Gives the the rounded x coordinate of particles index.
public func GetPartX(int index) { return (lib_ragdoll_particles[index].x + LIB_RAGDOLL_Precision / 2) / LIB_RAGDOLL_Precision; }

// Gives the the rounded y coordinate of particles index.
public func GetPartY(int index) { return (lib_ragdoll_particles[index].y + LIB_RAGDOLL_Precision / 2) / LIB_RAGDOLL_Precision; }


/*-- Helper Functions --*/

// Helper function which creates the test array variable to test the landscape to move out particles.
private func LogArray()
{
	// Helper function which creates the test array. 
	var test_array = [];
	for (var dist = 1; dist < 20; dist++)
		for (var x = 0; x < 20; x++)
			for (var y = 0; y < 20; y++)
				if (Distance(0, 0, x, y) == dist)
					PushBack(test_array, [x, y]);
	return Log("%v", test_array);
}

// Helper function which gives the speed of the particles.
func LogSpeed()
{
	// Helperfunction for Debugpurpose
	var speed_array = [];
	for (var i = 0; i < lib_ragdoll_particle_count; i++)
		PushBack(speed_array, Distance(lib_ragdoll_particles[i].x - lib_ragdoll_particles[i].oldx, lib_ragdoll_particles[i].y - lib_ragdoll_particles[i].oldy));
	return Log("%v", speed_array);
}
