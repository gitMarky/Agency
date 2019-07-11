/**
	Proplist format for animation stuff.
 */

static const Strike_Animation = new Global {
	Source = nil,    // Source ID
	Animation = nil, // Animation name
	Length = 0,      // Animation length (ticks)
	Start = 0,       // Start animation at this tick
	Strike = 0,      // Strike event happens at this tick
	Ending = ANIM_Remove,

	GetStrikeTime = func (int play_time)
	{
		// Calculates the delay, in frames, when the strike event should happen.
		return Max(0, this.Strike - this.Start) * play_time / this.Length;
	},

	GetAnimationLength = func ()
	{
		return this.Length;
	},

	GetAnimationStart = func ()
	{
		return this.Start;
	},

	GetAnimationEnding = func ()
	{
		return this.Ending;
	},

	GetName = func ()
	{
		return this.Animation;
	},

};

static const Strike_Animations = {
	Pickaxe = {
		Prototype = Strike_Animation,
		Source = Clonk,
		Animation = "StrikePickaxe",
		Length = 3400,
		Start = 1500,
		Strike = 3235,
		Ending = ANIM_Loop,
	},
	Throw = {
		Prototype = Strike_Animation,
		Source = Clonk,
		Animation = "ThrowArms",
		Length = 1500,
		Start = 0,
		Strike = 650,
		Ending = ANIM_Remove,
	},
};
