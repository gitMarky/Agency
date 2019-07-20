
#include Library_LiquidLeak
#include Property_ElectricHazard

/* --- Properties --- */

local Name = "$Name$";
local Description = "$Description$";
local Visibility = VIS_Editor; // Only visible in Editor Mode

public func CanBeElectrified(){  return LeakAreaActive != nil; }


/* --- Engine Callbacks --- */

func Definition(id type)
{
	_inherited(type, ...);
	type.LeakMaterial = Material("Water");
	type.SprayTexture = "water";
}


/* --- Functionality --- */

func EffectsOnElectricCurrent(object source)
{
	var particles = 
	{
		Prototype = Particles_ElectroSpark1(), 
		Size = PV_Random(2, 6),
		ForceY = PV_Gravity(500),
		OnCollision = PC_Bounce(500),
	};
	var pv_ydir = PV_Random(-5, -2);
	var pv_lifetime = PV_Random(5, 10);
	var pos = LeakAreaPoints[Random(GetLength(LeakAreaPoints))];
	if (pos)
	{
		CreateParticle("ElectroSpark", pos.X, pos.Y - 1, PV_Random(-3, 3), pv_ydir, pv_lifetime, particles);
	}
	if (source && !Random(3))
	{
		var x = source->GetX() - GetX();
		var y = -1;
		for (; GBackSolid(x, y) && y > -3; --y);
		var xdl = LeakAreaBounds[0] - x;
		var xdr = LeakAreaBounds[0] + LeakAreaBounds[1] - x;
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


func Find_Target()
{
	return Find_And(Find_Func("CanBeElectrocuted"), Find_NoContainer());
}

