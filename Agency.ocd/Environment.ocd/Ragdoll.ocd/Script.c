
#include Library_Ragdoll

func Activate()
{
	// Create a sort-of rope
	var stick_a = CreateRagdollParticle("StickA", 10, 0, false, false);
	var stick_b = CreateRagdollParticle("StickB", 20, 0, false, true);
	var stick_c = CreateRagdollParticle("StickC", 30, 0, false, true);
	var stick_d = CreateRagdollParticle("StickD", 40, 0, false, false);

	AddRagdollParticle(stick_a);
	AddRagdollParticle(stick_b);
	AddRagdollParticle(stick_c);
	AddRagdollParticle(stick_d);

	AddRagdollConstraint(Ragdoll_Constraint_Landscape->Create(stick_a));
	AddRagdollConstraint(Ragdoll_Constraint_Landscape->Create(stick_b));
	AddRagdollConstraint(Ragdoll_Constraint_Landscape->Create(stick_c));
	AddRagdollConstraint(Ragdoll_Constraint_Landscape->Create(stick_d));

	AddRagdollConstraint(Ragdoll_Constraint_Distance->CreateFixedDistance(stick_a, stick_b, 10));
	AddRagdollConstraint(Ragdoll_Constraint_Distance->CreateFixedDistance(stick_b, stick_c, 10));
	AddRagdollConstraint(Ragdoll_Constraint_Distance->CreateFixedDistance(stick_c, stick_d, 10));

	AddTimer(this.RagdollTimeStep, 1);
}


func Humanoid()
{
	var body = CreateRagdollParticle("Body",   0,  0, false, true);
	var legl = CreateRagdollParticle("LegL",  -2, 10, false, true);
	var legr = CreateRagdollParticle("LegR",  +2, 10, false, true);
	var arml = CreateRagdollParticle("ArmL", -10, -7, false, true);
	var armr = CreateRagdollParticle("ArmR", +10, -7, false, true);
	var neck = CreateRagdollParticle("Neck",   0, -7, false, true);

	body->SetMass(40);
	legl->SetMass(10);
	legr->SetMass(10);
	arml->SetMass(5);
	armr->SetMass(5);
	neck->SetMass(10);

	AddRagdollParticle(body);
	AddRagdollParticle(legl);
	AddRagdollParticle(legr);
	AddRagdollParticle(arml);
	AddRagdollParticle(armr);
	AddRagdollParticle(neck);

	AddRagdollConstraint(Ragdoll_Constraint_Landscape->Create(body));
	AddRagdollConstraint(Ragdoll_Constraint_Landscape->Create(legl));
	AddRagdollConstraint(Ragdoll_Constraint_Landscape->Create(legr));
	AddRagdollConstraint(Ragdoll_Constraint_Landscape->Create(arml));
	AddRagdollConstraint(Ragdoll_Constraint_Landscape->Create(armr));
	AddRagdollConstraint(Ragdoll_Constraint_Landscape->Create(neck));

	AddRagdollConstraint(Ragdoll_Constraint_Distance->CreateFixedDistance(body, neck, 7));
	AddRagdollConstraint(Ragdoll_Constraint_Distance->CreateFixedDistance(neck, arml, 10));
	AddRagdollConstraint(Ragdoll_Constraint_Distance->CreateFixedDistance(neck, armr, 10));
	AddRagdollConstraint(Ragdoll_Constraint_Distance->CreateFixedDistance(body, legl, Distance(2, 10)));
	AddRagdollConstraint(Ragdoll_Constraint_Distance->CreateFixedDistance(body, legr, Distance(2, 10)));

	AddTimer(this.RagdollTimeStep, 1);
}

func UpdateRagdoll()
{
	for (var particle in ragdoll_particles)
	{
		var x = particle->GetRelPos(particle->GetPosX(), GetX());
		var y = particle->GetRelPos(particle->GetPosY(), GetY());
		CreateParticle("Magic", x, y, 0, 0, 1, { Size = Max(1, particle->GetMass() / 10), }, 1);
	}
}
