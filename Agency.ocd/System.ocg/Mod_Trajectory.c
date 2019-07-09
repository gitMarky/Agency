#appendto Trajectory

/**
	Adds the trajectory for the given object.

	x: start x position in global coordinates
	y: start y position in global coordinates
	xdir: direction in x
	ydir: direction in y
	color: color of the trajectory particles (default white)
	spacing: spacing between the particles (default 10 pixels)
 */
public func Create(object target, int x, int y, int xdir, int ydir, int color, int spacing, bool impact)
{
	if (this != Trajectory)
	{
		return;
	}

	// Do not create trajectories for script players, this will only cause lag.
	var controller = target->GetController();
	if (controller == NO_OWNER || GetPlayerType(controller) == C4PT_Script)
	{
		return;
	}

	// Delete old trajectory.
	Trajectory->Remove(target);

	// Default values with added precision.
	x *= 10000;
	y *= 10000;
	xdir *= 100;
	ydir *= 100;
	color = color ?? RGBa(255, 255, 255, 100);
	spacing = spacing ?? 5;
	spacing *= 10000;

	// Create new helper object
	var trajectory = CreateObject(Trajectory, target->GetX(), target->GetY(), controller);
	trajectory->SetAction("Attach", target);

	// Particle setup.
	var particles =
	{
		Prototype = Particles_Trajectory(),
		Size = 4,
		R = (color >> 16) & 0xff,
		G = (color >>  8) & 0xff,
		B = (color >>  0) & 0xff,
		Alpha = (color >> 24) & 0xff,
	};

	var frame_step = 0;
	var oldx = x;
	var oldy = y;
	var particle_positions = [];
	while (frame_step < 36 * 100)
	{
		// Update coordinates.
		x += xdir;
		y += ydir;
		ydir += GetGravity();

		if (Distance(x, y, oldx, oldy) >= spacing)
		{
			// Correct for the fact that the trajectory object is attached to the shooting parent.
			var parent = trajectory->GetActionTarget();
			var off_x = -parent->GetVertex(0, VTX_X) - trajectory->GetX();
			var off_y = -parent->GetVertex(0, VTX_Y) - trajectory->GetY();
			PushBack(particle_positions, {X = x / 10000 + off_x, Y = y / 10000 + off_y});
			oldx = x;
			oldy = y;
		}

		if (GBackSolid(x / 10000 - GetX(), y / 10000 - GetY()))
		{
			break;
		}

		frame_step++;
	}

	// Draw the trajectory
	var max = GetLength(particle_positions);
	for (var i = 0; i < max; ++i)
	{
		var offset = max / 4;
		var alpha = particles.Alpha * Max(0, i - offset) / Max(1, max - offset);
		var size = 4; // 2 + 2 * i / max;
		trajectory->CreateParticle("Magic", particle_positions[i].X, particle_positions[i].Y, 0, 0, 0, { Prototype = particles, Size = size, Alpha = alpha});
	}
	// Draw the impact
	/*var interval = 15;
	if (impact && max > 0 && (FrameCounter() % interval) == 0)
	{
		trajectory->CreateParticle("Shockwave", particle_positions[max - 1].X, particle_positions[max - 1].Y, 0, 0, 3 * interval / 4,
		//trajectory->CreateParticle("Magic", particle_positions[max - 1].X, particle_positions[max - 1].Y, 0, 0, 3 * interval / 4,
		{
			Size = PV_Linear(4, 25),
			R = particles.R,
			G = particles.G,
			B = particles.B,
			Rotation = Random(360),
			//BlitMode = GFX_BLIT_Additive,
			Alpha = PV_Linear(particles.Alpha, 0),
		});
	}*/

	return trajectory;
}
