
#include Property_ElectricHazard

/* --- Properties --- */

local Name = "$Name$";
local Description = "$Description$";
local Visibility = VIS_Editor; // Only visible in Editor Mode

local Area = [-15, -15, 30, 20]; // Area the can possibly hold water
local AreaBounds = []; // Area left and right boundaries, adjusted to actual water output
local AreaPoints = []; // Array of points where water was inserted
local AreaActive = nil; // Area that is checked for victims

public func CanBeElectrified(){  return AreaActive != nil; }

/* --- Engine Callbacks --- */

func Construction(object by)
{
	_inherited(by, ...);
	AdjustPosition();
	ScheduleCall(this, this.SetAreaRect, 1, 0, GetID().Area);
}


func Definition(id type)
{
	// EditorProps
	if (!type.EditorProps)
	{
		type.EditorProps = {};
	}

	// Area to check for water
	type.EditorProps.Area = 
	{ 
		Name = "$Area$",
		Type = "rect",
		Value =
		{
			Rect = type.Area
		},
		ValueKey = "Area",
		Color = RGB(0, 150, 255),
		Relative = true,
		Set = "SetAreaRect",
		SetRoot = false
	};
	// Area to check for victims, cannot be set
	type.EditorProps.AreaActive = 
	{ 
		Name = "$AreaActive$",
		Type = "rect",
		Value =
		{
			Rect = type.AreaActive
		},
		ValueKey = "AreaActive",
		Color = RGB(255, 180, 0),
		Relative = true,
		SetRoot = false
	};
}


/* --- Functionality --- */

func EffectsOnElectricCurrent(object source)
{
	var pv_ydir = PV_Random(-5, -2);
	var pv_lifetime = PV_Random(5, 10);
	var pos = AreaPoints[Random(GetLength(AreaPoints))];
	if (pos)
	{
		var particles = 
		{
			Prototype = Particles_ElectroSpark1(), 
			Size = PV_Random(2, 6),
			ForceY = PV_Gravity(500),
			OnCollision = PC_Bounce(500),
		};
		CreateParticle("ElectroSpark", pos.X, pos.Y - 1, PV_Random(-3, 3), pv_ydir, pv_lifetime, particles);
	}
	if (source && !Random(3))
	{
		var x = source->GetX() - GetX();
		var y = -1;
		for (; GBackSolid(x, y) && y > -3; --y);
		var xdl = AreaBounds[0] - x;
		var xdr = AreaBounds[0] + AreaBounds[1] - x;
		CreateParticle("ElectroSpark", x, y, PV_Random(xdl, xdr), pv_ydir, pv_lifetime, particles);
	}
}


func TriggerOnElectricCurrent(object source)
{
	var targets = FindObjects(Find_InZone(), Find_Target());

	for (var target in targets)
	{
		if (target->GetContact(-1) & CNAT_Bottom)
		{
			target->~Electrocute();
			
			// TODO: Callback to source: Switch off elecriticy, etc?
		}
	}
}


func Find_InZone()
{
	return Find_InRect(AreaActive[0], AreaActive[1], AreaActive[2], AreaActive[3]);
}


func Find_Target()
{
	return Find_And(Find_Func("CanBeElectrocuted"), Find_NoContainer());
}


/* --- Water output --- */


func Activate()
{
	AddTimer(this.LeakWater, 5);	
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


func ResetWater()
{
	AreaBounds[0] = 0;
	AreaBounds[1] = 0;
}


func LeakWater()
{
	// Fill the area with water!
	var list = {};
	var cancel = false;
	var max_points = Area[2];
	if (InsertMaterial(Material("Water"), 0, 0, 0, 0, list))
	{
		var left = Area[0];
		var right = left + Area[2];
		var top = Area[1];
		var bottom = top + Area[3];

		list.X -= GetX();
		list.Y -= GetY();
		if (Inside(list.X, left, right) && Inside(list.Y, top, bottom))
		{
			// Expand the bounds
			if (list.X < AreaBounds[0])
			{
				AreaBounds[0] = list.X;
			}
			if (list.X > AreaBounds[1])
			{
				AreaBounds[1] = list.X;
			}
			// Save for later
			PushBack(AreaPoints, { X = list.X, Y = list.Y });
			if (GetLength(AreaPoints) >= max_points)
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
		RemoveTimer(this.LeakWater);
		// Shrink the bounds a bit, looks better (but keep a minimum area). Then set the active area
		AreaBounds[0] = Min(-5, AreaBounds[0] + 2);
		AreaBounds[1] = Max(+5, AreaBounds[1] - 2);
		AreaActive = [AreaBounds[0], Area[1], AreaBounds[1] - AreaBounds[0], Area[3]];
	}
}


/* --- Setter functions for EditorProps --- */

func SetAreaRect(array new_area)
{
	Area = new_area;
	AreaBounds = [0, 0];
	SetShape(Area[0], Area[1], Area[2], Area[3]);
	return true;
}


func SetAreaActive(array new_area)
{
	AreaActive = new_area;
	return true;
}


/* --- Scenario Saving --- */

func SaveScenarioObject(props)
{
	if (_inherited(props, ...))
	{
		props->AddCall("Area", this, "SetAreaRect", Area);
		props->AddCall("AreaActive", this, "SetAreaActive", AreaActive);
		return true;
	}
	else
	{
		return false;
	}
}
