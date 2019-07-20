
#include Library_LiquidLeak

/* --- Properties --- */

local Name = "$Name$";
local Description = "$Description$";
local Visibility = VIS_Editor;


/* --- Engine Callbacks --- */

func Definition(id type)
{
	_inherited(type, ...);
	type.LeakMaterial = Material("Oil");
	type.SprayTexture = "oil";
}

