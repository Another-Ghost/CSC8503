#include "OrientationConstraint.h"

#include "../../Common/Vector3.h"

#include "Debug.h"

#include "GameObject.h"
#include <vector>

void NCL::CSC8503::OrientationConstraint::UpdateConstraint(float dt)
{
	PhysicsObject* phys1 = object1->GetPhysicsObject();
	PhysicsObject* phys2 = object2->GetPhysicsObject();
	Vector3 AngVelocity1 = phys1->GetAngularVelocity();
	Vector3 AngVelocity2 = phys2->GetAngularVelocity();


	Vector3 a1 = object1->GetTransform().GetRotMatrix() * Vector3(1, 0, 0);
	Vector3 b1 = object1->GetTransform().GetRotMatrix() * Vector3(0, 1, 0);
	Vector3 c1 = object1->GetTransform().GetRotMatrix() * Vector3(0, 0, 1);

	Vector3 a2 = object2->GetTransform().GetRotMatrix() * Vector3(1, 0, 0);
	Vector3 b2 = object2->GetTransform().GetRotMatrix() * Vector3(0, 1, 0);
	Vector3 c2 = object2->GetTransform().GetRotMatrix() * Vector3(0, 0, 1);

	Vector3 strartPoint = object1->GetTransform().GetPosition() + a1*10;// 
	Vector3 endPoint = object1->GetTransform().GetPosition() - a1 * 10;
	Debug::DrawLine(strartPoint, endPoint, Debug::GREEN, 0.01f);

	strartPoint = object2->GetTransform().GetPosition() + a2*10;// 
	endPoint = object2->GetTransform().GetPosition() - a2 * 10;
	Debug::DrawLine(strartPoint, endPoint, Debug::GREEN, 0.01f);


	if (abs(Vector3::Dot(a1, b2)) - 0.00001 < 0 && abs(Vector3::Dot(a1, c2)) - 0.00001 < 0) {
		return;
	}

	float c11 = Vector3::Dot(-Vector3::Cross(b2, a1), AngVelocity1) +
		Vector3::Dot(Vector3::Cross(b2, a1), AngVelocity2);
	float c21 = Vector3::Dot(-Vector3::Cross(c2, a1), AngVelocity1) +
		Vector3::Dot(Vector3::Cross(c2, a1), AngVelocity2);


	float a = Vector3::Dot(Vector3::Cross(b2, a1), phys1->GetInertiaTensor() * Vector3::Cross(b2, a1));
	+Vector3::Dot(Vector3::Cross(b2, a1), phys2->GetInertiaTensor() * Vector3::Cross(b2, a1));

	float b = Vector3::Dot(Vector3::Cross(b2, a1), phys1->GetInertiaTensor() * Vector3::Cross(c2, a1));
	+Vector3::Dot(Vector3::Cross(b2, a1), phys2->GetInertiaTensor() * Vector3::Cross(c2, a1));

	float c = Vector3::Dot(Vector3::Cross(c2, a1), phys1->GetInertiaTensor() * Vector3::Cross(b2, a1));
	+Vector3::Dot(Vector3::Cross(c2, a1), phys2->GetInertiaTensor() * Vector3::Cross(b2, a1));

	float d = Vector3::Dot(Vector3::Cross(c2, a1), phys1->GetInertiaTensor() * Vector3::Cross(c2, a1));
	+Vector3::Dot(Vector3::Cross(c2, a1), phys2->GetInertiaTensor() * Vector3::Cross(c2, a1));


	float JMJ1 = Vector3::Dot(-Vector3::Cross(b2, a1), phys1->GetInertiaTensor() * -Vector3::Cross(b2, a1))
		+Vector3::Dot(Vector3::Cross(b2, a1), phys2->GetInertiaTensor() * Vector3::Cross(b2, a1));
	float JMJ2 = Vector3::Dot(-Vector3::Cross(c2, a1), phys1->GetInertiaTensor() * -Vector3::Cross(c2, a1))
		+Vector3::Dot(Vector3::Cross(c2, a1), phys2->GetInertiaTensor() * Vector3::Cross(c2, a1));


	float biasFactor = 0.01f;
	float bias1 = -(biasFactor / dt) * c11;
	float bias2 = -(biasFactor / dt) * c21;

	float lambda1 = (c11 + bias1) / -JMJ1;
	float lambda2 = (c21 + bias2) / -JMJ2;

	Vector3 impulse1 = -Vector3::Cross(b2, a1) * lambda1 +
		-Vector3::Cross(c2, a1) * lambda2;
	Vector3 impulse2 = Vector3::Cross(b2, a1) * lambda1 +
		Vector3::Cross(c2, a1) * lambda2;

	//phys1->ApplyLinearImpulse(impulse1);
	//phys2->ApplyLinearImpulse(impulse2);
	if (impulse1.Length() > 1000) {
		impulse1.Normalised() * 1000;
		impulse2 = -impulse1;
	}
	phys1->ApplyAngularImpulse(impulse1);
	phys2->ApplyAngularImpulse(impulse2);
}
