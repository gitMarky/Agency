
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

func UpdateRagdoll()
{
	for (var particle in ragdoll_particles)
	{
		var x = particle->GetRelPos(particle->GetPosX(), GetX());
		var y = particle->GetRelPos(particle->GetPosY(), GetY());
		CreateParticle("Magic", x, y, 0, 0, 1, {}, 1);
	}
}
