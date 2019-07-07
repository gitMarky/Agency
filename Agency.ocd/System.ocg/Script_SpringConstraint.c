/**
	Pseudo-physics spring constraint

	Fixes two objects together, as if they were connected by a spring.
*/
static const FxSpringConstraint = new Effect
{
	Start = func (int temporary, int distance, int max_dist)
	{
		if (temporary)
		{
			return;
		}
		this.distance = distance;
		this.max_dist = max_dist;
	},

	SetBodyA = func (object body_a, bool fixed)
	{
		AssertNotNil(body_a);
		this.body_a = body_a;
		this.fixed_a = fixed;
		return this;
	},

	SetBodyB = func (object body_b, bool fixed)
	{
		AssertNotNil(body_b);
		this.body_b = body_b;
		this.fixed_b = fixed;
		return this;
	},

	Timer = func ()
	{
		// One body gone?
		if (!this.body_a || !this.body_b)
		{
			return FX_Execute_Kill;
		}
		// Both ends fixed? Spring constraint does not make sense then
		if (this.fixed_a && this.fixed_b)
		{
			return FX_Execute_Kill;
		}
		// Range exceeded?
		var distance = ObjectDistance(this.body_a, this.body_b);
		if (this.max_dist && distance >= this.max_dist)
		{
			return FX_Execute_Kill;
		}
		var displacement = distance - this.distance;
		// Both ends loose? Assume a fixed point in the middle of the "spring"
		if (!this.fixed_a && !this.fixed_b)
		{
			displacement = (displacement + 1) / 2;
		}

		Constrain(displacement);
	},

	Constrain = func (int displacement)
	{
		var prec_angle = 1000;
		var angle = Angle(this.body_a->GetX(), this.body_a->GetY(), this.body_b->GetX(), this.body_b->GetY(), prec_angle);

		var velocity = 10 * displacement;

		if (!this.fixed_a)
		{
			this.body_a->AddVelocity(angle, velocity, prec_angle);
		}
		if (!this.fixed_b)
		{
			this.body_b->AddVelocity((prec_angle * 180) + angle, velocity, prec_angle);
		}
	},
};
