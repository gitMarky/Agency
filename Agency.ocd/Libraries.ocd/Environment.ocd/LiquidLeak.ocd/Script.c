
/* --- Properties --- */

local LeakArea = [-15, -15, 30, 20]; // LeakArea the can possibly hold water
local LeakAreaBounds = []; // LeakArea left and right boundaries, adjusted to actual water output
local LeakAreaPoints = []; // Array of points where water was inserted
local LeakAreaActive = nil; // LeakArea that is checked for victims
local LeakDelay = 0;
local LeakMaterial = nil; // This material is leaked

local SprayTime = 90;       // Spray effect, only if the strength is > 0 there will be an effect
local SprayStrength = 75;   // Strength of the water output
local SprayAngle = 0;      // Angle
local SprayVariance = 10;  // Angle variance
local SprayPrecision = 10; // Angle precision
local SprayX = 0;          // Offset, local
local SprayY = 0;          // Offset, local
local SprayAmount = 5;     // Amount of particles per spray
local SprayTexture = nil;  // Particles use the color of this texture

/* --- Engine Callbacks --- */

func Construction(object by)
{
	_inherited(by, ...);
	AdjustPosition();
	ScheduleCall(this, this.SetLeakAreaRect, 1, 0, GetID().LeakArea);
}


func Definition(id type)
{
	// EditorProps
	if (!type.EditorProps)
	{
		type.EditorProps = {};
	}

	// LeakArea to check for water
	type.EditorProps.LeakArea = 
	{ 
		Name = "$LeakArea$",
		Type = "rect",
		Value =
		{
			Rect = type.LeakArea
		},
		ValueKey = "LeakArea",
		Color = RGB(0, 150, 255),
		Relative = true,
		Set = "SetLeakAreaRect",
		SetRoot = false
	};
	// LeakArea to check for victims, cannot be set
	type.EditorProps.LeakAreaActive = 
	{ 
		Name = "$LeakAreaActive$",
		Type = "rect",
		Value =
		{
			Rect = type.LeakAreaActive
		},
		ValueKey = "LeakAreaActive",
		Color = RGB(255, 180, 0),
		Relative = true,
		SetRoot = false
	};
}


/* --- Functionality --- */

func Find_InZone()
{
	return Find_InRect(LeakAreaActive[0], LeakAreaActive[1], LeakAreaActive[2], LeakAreaActive[3]);
}


/* --- Liquid output --- */

func Activate()
{
	if (SprayTime > 0)
	{
		AddTimer(this.SprayLiquid, 1);
		LeakDelay = 15 * Max(1, SprayStrength) / GetGravity();
	}
	AddTimer(this.LeakLiquid, 5);
}


func AdjustPosition()
{
	// Shift down until there is ground.
	// The leak has to be exactly 1 pixel
	// above the ground for the water logic
	// to work correctly.
	for (var y = 0; !GBackSolid(0, 1) && y < 50; ++y)
	{
		MovePosition(0, 1);
	}
}


func ResetLiquid()
{
	LeakAreaBounds[0] = 0;
	LeakAreaBounds[1] = 0;
}


func LeakLiquid()
{
	if (LeakDelay > 0)
	{
		return;
	}
	// Fill the area with liquid!
	var list = {};
	var cancel = false;
	var max_points = LeakArea[2];
	if (InsertMaterial(LeakMaterial, 0, 0, 0, 0, list))
	{
		var left = LeakArea[0];
		var right = left + LeakArea[2];
		var top = LeakArea[1];
		var bottom = top + LeakArea[3];

		list.X -= GetX();
		list.Y -= GetY();
		if (Inside(list.X, left, right) && Inside(list.Y, top, bottom))
		{
			// Expand the bounds
			if (list.X < LeakAreaBounds[0])
			{
				LeakAreaBounds[0] = list.X;
			}
			if (list.X > LeakAreaBounds[1])
			{
				LeakAreaBounds[1] = list.X;
			}
			// Save for later
			PushBack(LeakAreaPoints, { X = list.X, Y = list.Y });
			if (GetLength(LeakAreaPoints) >= max_points)
			{
				cancel = true;
			}
		}
		else
		{
			cancel = true;
		}
	}
	else
	{
		cancel = true;
	}
	if (cancel)
	{
		RemoveTimer(this.LeakLiquid);
		// Shrink the bounds a bit, looks better (but keep a minimum area). Then set the active area
		LeakAreaBounds[0] = Min(-5, LeakAreaBounds[0] + 2);
		LeakAreaBounds[1] = Max(+5, LeakAreaBounds[1] - 2);
		LeakAreaActive = [LeakAreaBounds[0], LeakArea[1], LeakAreaBounds[1] - LeakAreaBounds[0], LeakArea[3]];
	}
}


func SprayLiquid()
{
	if (SprayTime > 0)
	{
		var lifetime = 20 + 20 * Max(1, SprayStrength) / GetGravity();
		var water = Particles_Colored({
			OnCollision = PC_Bounce(100),
			CollisionVertex = 500,
			Size = PV_KeyFrames(0, 0, Max(1, SprayAmount), 500, 2),
			ForceX = PV_Wind(PV_KeyFrames(0, 0, 0, 200, PV_Random(15, 30))),
			ForceY = PV_Gravity(PV_KeyFrames(0, 0, 1000, 500, 900, 800, 500)),
			Stretch = PV_KeyFrames(0, 0, 1000, 60, PV_Speed(1000), 300, 1000),
			Rotation = PV_Direction(),
			Alpha = PV_KeyFrames(0, 0, 128, 800, 50, 1000, 0),
		}, GetAverageTextureColor(SprayTexture));
		for (var i = 0; i < SprayAmount; ++i)
		{
			var angle = SprayAngle + RandomX(-SprayVariance, +SprayVariance);
			CreateParticle("Magic", SprayX, SprayY, Sin(angle, SprayStrength, SprayPrecision), -Cos(angle, SprayStrength, SprayPrecision), lifetime, water);
		}
		SprayTime -= 1;
		LeakDelay -= 1;
		SprayStrength = Min(SprayStrength, SprayTime);
	}
	if (SprayTime <= 0)
	{
		RemoveTimer(this.SprayLiquid);
	}
}


/* --- Setter functions for EditorProps --- */

func SetLeakAreaRect(array new_area)
{
	LeakArea = new_area;
	LeakAreaBounds = [0, 0];
	SetShape(LeakArea[0], LeakArea[1], LeakArea[2], LeakArea[3]);
	return true;
}


func SetLeakAreaActive(array new_area)
{
	LeakAreaActive = new_area;
	return true;
}


/* --- Scenario Saving --- */

func SaveScenarioObject(props)
{
	if (_inherited(props, ...))
	{
		props->AddCall("LeakArea", this, "SetLeakAreaRect", LeakArea);
		props->AddCall("LeakAreaActive", this, "SetLeakAreaActive", LeakAreaActive);
		return true;
	}
	else
	{
		return false;
	}
}
