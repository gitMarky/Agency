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
		var length = Abs(GetAnimationEnd() - GetAnimationBegin());
		var strike = Abs(this.Strike - GetAnimationStart());
		// Calculates the delay, in frames, when the strike event should happen.
		return strike * play_time / Max(1, length);
	},

	GetAnimationLength = func ()
	{
		return this.Length;
	},

	GetAnimationStart = func ()
	{
		return this.Start;
	},

	GetAnimationBegin = func ()
	{
		return this.Begin ?? 0;
	},

	GetAnimationEnd = func ()
	{
		return this.End ?? this.Length;
	},

	GetAnimationEnding = func ()
	{
		return this.Ending;
	},

	GetName = func (dir)
	{
		if (dir == nil)
		{
			return this.Animation;
		}
		else if (GetType(dir) == C4V_String)
		{
			return Format("%s%s", this.Animation, dir);
		}
		else if (GetType(dir) == C4V_Int)
		{
			if (dir == 0)
			{
				return GetName(".L");
			}
			else
			{
				return GetName(".R");
			}
		}
		else
		{
			FatalError("Unsupported type %s, only string and int are supported", GetType(dir));
		}
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
	FastStrike = {
		Prototype = Strike_Animation,
		Source = Clonk,
		Animation = "SwordSlash2",
		Length = 1300,
		Start = 600,
		Strike = 100,
		Begin = 1300, End = 0,
		Ending = ANIM_Remove,
	},
};
