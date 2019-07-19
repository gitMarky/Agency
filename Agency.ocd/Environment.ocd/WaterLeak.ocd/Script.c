
#include Property_ElectricHazard

/* --- Properties --- */

local Name = "$Name$";
local Description = "$Description$";
local Visibility = VIS_Editor; // Only visible in Editor Mode
local Area = [-15, -15, 30, 20]; // Area to check

/* --- Engine Callbacks --- */

func Construction(object by)
{
	_inherited(by, ...);
	// Shift down until you hit the ground
	for (var y = 0; y < 10; ++y)
	{
		if (!GBackSolid(0, 1))
		{
			MovePosition(0, 1);
		}
	}
	// Update shape
	ScheduleCall(this, this.SetAreaRect, 1, 0, GetID().Area);
}


func Definition(id type)
{
	// EditorProps
	if (!type.EditorProps)
	{
		type.EditorProps = {};
	}

	// Area to check for clonks
	type.EditorProps.Area = 
	{ 
		Name = "$Area$",
		Type = "rect",
		Value=
		{
			Rect = type.Area
		},
		ValueKey = "Area",
		Color = RGB(0, 150, 255),
		Relative = true,
		Set = "SetAreaRect",
		SetRoot = false
	};
}


/* --- Functionality --- */

func EffectsOnElectricCurrent(object source)
{
	Message("Zap!!");
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
	return Find_InRect(Area[0], Area[1], Area[2], Area[3]);
}


func Find_Target()
{
	return Find_And(Find_Func("CanBeElectrocuted"), Find_NoContainer());
}


/* --- Setter functions for EditorProps --- */

func SetAreaRect(array new_area)
{
	Area = new_area;
	SetShape(Area[0], Area[1], Area[2], Area[3]);
	return true;
}

/* --- Scenario Saving --- */

func SaveScenarioObject(props)
{
	if (_inherited(props, ...))
	{
		props->AddCall("Area", this, "SetAreaRect", Area);
		return true;
	}
	else
	{
		return false;
	}
}
