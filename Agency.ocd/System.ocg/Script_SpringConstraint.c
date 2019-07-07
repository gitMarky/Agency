/**
	Pseudo-physics spring constraint

	Fixes two objects together, as if they were connected by a spring.
*/
static const FxSpringConstraint = new Effect
{
	Start = func (int temporary, int distance, int max_dist, int max_velocity)
	{
		if (temporary)
		{
			return;
		}
		this.prec_distance = 10;
		this.prec_angle = 1000;
		this.distance = distance * this.prec_distance;
		this.max_dist = max_dist * this.prec_distance;
		this.max_velocity = max_velocity ?? this.max_dist / 5;
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

		// Range exceeded? => Stop!
		var range = ObjectDistance(this.body_a, this.body_b) * this.prec_distance;
		if (this.max_dist && range >= this.max_dist)
		{
			return FX_Execute_Kill;
		}

		// Calculate displacement, by predicting the next position
		var x_body_a = this.body_a->GetX(this.prec_distance) + this.body_a->GetXDir(this.prec_distance);
		var y_body_a = this.body_a->GetY(this.prec_distance) + this.body_a->GetYDir(this.prec_distance);
		var x_body_b = this.body_b->GetX(this.prec_distance) + this.body_b->GetXDir(this.prec_distance);
		var y_body_b = this.body_b->GetY(this.prec_distance) + this.body_b->GetYDir(this.prec_distance);
		var distance = Distance(x_body_a, y_body_a, x_body_b, y_body_b);
		var angle = Angle(x_body_a, y_body_a, x_body_b, y_body_b, this.prec_angle);

		var displacement = distance - this.distance;

		// Both ends loose? Assume a fixed point in the middle of the "spring"
		if (!this.fixed_a && !this.fixed_b)
		{
			displacement = (displacement + 1) / 2;
		}

		Constrain(angle, displacement);
	},

	Constrain = func (int angle, int velocity)
	{
		velocity = BoundBy(velocity, -this.max_velocity, this.max_velocity);
		if (!this.fixed_a)
		{
			this.body_a->AddVelocity(angle, velocity, this.prec_angle, this.prec_distance);
		}
		if (!this.fixed_b)
		{
			this.body_b->AddVelocity((this.prec_angle * 180) + angle, velocity, this.prec_angle, this.prec_distance);
		}
	},
};
