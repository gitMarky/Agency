#include Character_Human
#include Clonk_HandDisplay
#include Character_Control
#include Library_HUDAdapter
#include Library_CarryHeavyControl
#include Library_CanClimbLadder

/* --- Properties --- */

local Name = "Player";
local Description = "$Description$";
local MaxEnergy = 50000;
local MaxBreath = 720; // Clonk can breathe for 20 seconds under water.
local JumpSpeed = 400;
local ThrowSpeed = 294;
local ContactIncinerate = 10;

