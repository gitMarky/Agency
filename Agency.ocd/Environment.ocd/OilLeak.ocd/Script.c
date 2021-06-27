
#include Library_LiquidLeak

/* --- Properties --- */

local Name = "$Name$";
local Description = "$Description$";
local Visibility = VIS_Editor;


/* --- Engine Callbacks --- */

func Construction(object by)
{
	_inherited(by, ...);
	this.LeakMaterial = Material("Oil");
	this.SprayTexture = "oil";
}

