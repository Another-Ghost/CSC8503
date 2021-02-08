#include "HingedConstraint.h"

#include "../../Common/Vector3.h"
#include "GameObject.h"
#include <vector>

void NCL::CSC8503::HingedConstraint::UpdateConstraint(float dt)
{


	Quaternion q_a = object1->GetTransform().GetOrientation();
	Quaternion q_b = object2->GetTransform().GetOrientation();

	Transform transformA = object1->GetTransform();
	Transform transformB = object2->GetTransform();
	Vector3 a0(1,0,0);
	Vector3 b0 = transformB.GetInvRotMatrix() * transformA.GetRotMatrix() * a0;
	Vector3 h;
	if ((a0 + b0).Length() > 0.00001) {
		h = (a0 + b0) / (a0 + b0).Length();
	}
	else {
		h = Vector3::Cross(a0, Vector3(1, 1, 1).Normalised());
	}

	Quaternion q_0(Vector3::Cross(a0, h), Vector3::Dot(a0, h));

	Vector3 a(Vector3(0, 1, 0));
	Vector3 b(Vector3(0, 0, 1));

	//Quaternion q_rw = q_b * q_a.Conjugate();

	//Quaternion q_r = q_a.Conjugate() * q_b;

	PhysicsObject* phys1 = object1->GetPhysicsObject();
	PhysicsObject* phys2 = object2->GetPhysicsObject();
	Vector3 AngVelocity1 = phys1->GetAngularVelocity();
	Vector3 AngVelocity2 = phys2->GetAngularVelocity();
	//float A[3][4] = { 0,1,0,0, 
	//					0, 0, 1, 0,
	//					0, 0, 0, 1 };

	//float A_T[4][3] = { 0,0,0,
	//	1, 0, 0,
	//	0, 1, 0,
	//	0, 0, 1
	//};

	float a_1[16] = { 0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1,
		0, 0, 0, 0 };
	Matrix4 A(a_1);

	float a_t[16] = { 0,0,0,0,
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0 };
	Matrix4 A_T(a_t);


	Vector3 j11 = Matrix3((A * q_a.Conjugate() * q_b * q_0.Conjugate() * A.Transpose()) * -0.5) * a;
	Vector3 j21 = -j11;


	//Vector3 j12 = Matrix3((A * q_b.Conjugate() * q_a * q_0 * A.Transpose()) * -0.5) * b;
	Vector3 j12 = Matrix3((A * q_a.Conjugate() * q_b * q_0.Conjugate() * A.Transpose()) * -0.5) * b;
	Vector3 j22 = -j12;

	float jv1 = Vector3::Dot(j11, AngVelocity1) + Vector3::Dot(j12, AngVelocity2);

	float jv2 = Vector3::Dot(j21, AngVelocity1) + Vector3::Dot(j22, AngVelocity2);


	float JMJ1 = Vector3::Dot(j11, phys1->GetInertiaTensor() * j11)
		+ Vector3::Dot(j12, phys1->GetInertiaTensor() * j12);
	float JMJ2 = Vector3::Dot(j21, phys2->GetInertiaTensor() * j21)
		+ Vector3::Dot(j22, phys1->GetInertiaTensor() * j22);

	if (fabs(JMJ2) < 0.00001) {
		return;
	}

	float biasFactor = 0.01f;
	float bias1 = -(biasFactor / dt) * jv1;
	float bias2 = -(biasFactor / dt) * jv2;
	float lambda1;
	if (fabs(JMJ1) > 0.00001) {
		lambda1 = (jv1 + bias1) / -JMJ1;
	}
	else {
		lambda1 = 0;
	}

	float lambda2 = (jv2 + bias2) / -JMJ2;


	Vector3 impulse1 = j11 * lambda1 +
		j12 * lambda2;
	Vector3 impulse2 = j21 * lambda1 +
		j22 * lambda2;

	phys1->ApplyAngularImpulse(impulse1);
	phys2->ApplyAngularImpulse(impulse2);
	//Vector3 relativePos = objectA->GetTransform().GetPosition() -
//	objectB->GetTransform().GetPosition();
//Vector3 offsetDir = relativePos.Normalised();
//Vector3 forwardDir = (objectB->GetTransform().GetRotMatrix() * Vector3(0, 0, -1)).Normalised();



////if (Vector3::Dot(forwardDir, offsetDir) + FLT_EPSILON < 1.0f) {
//	PhysicsObject* phys1 = object1->GetPhysicsObject();
//	PhysicsObject* phys2 = object2->GetPhysicsObject();
//	Vector3 AngVelocity1 = phys1->GetAngularVelocity();
//	Vector3 AngVelocity2 = phys2->GetAngularVelocity();
//	//	Vector3 relativeAngVelocity = physA->GetAngularVelocity() -
//	//		physB->GetAngularVelocity();	
//	//}
//	//Vector3 n, p, sa, sb;
//
//	//vector<Vector3> J = { -n, n, Vector3::Cross(-(p - sa), n), 
//	//	Vector3::Cross((p - sb), n) };
//	//Vector3 n = Vector3(0, 1, 0);
//
//	//Vector3 JA = Vector3::Cross((AngVelocity1 - AngVelocity2), n);
//	//Vector3 JB = Vector3::Cross((AngVelocity2 - AngVelocity1), n);
//
//	//float cd = Vector3::Dot(JA, AngVelocity1) + Vector3::Dot(JB, AngVelocity2);
//	//Vector3 AngVelocityC = Vector3(0, cd, 0);
//
//
//
//	//Vector3 interA = Vector3(Vector3::Dot(JA, phys1->GetInertiaTensor().GetColumn(0)),
//	//	Vector3::Dot(JA, phys1->GetInertiaTensor().GetColumn(1)),
//	//	Vector3::Dot(JA, phys1->GetInertiaTensor().GetColumn(2)));
//	//Vector3 interB = Vector3(Vector3::Dot(JB, phys2->GetInertiaTensor().GetColumn(0)),
//	//	Vector3::Dot(JB, phys2->GetInertiaTensor().GetColumn(1)),
//	//	Vector3::Dot(JB, phys2->GetInertiaTensor().GetColumn(2)));
//	//float inertia = Vector3::Dot(interA, JA) + Vector3::Dot(interB, JB);
//
//	//float lambda = -cd / inertia;
//
//
//	Vector3 a1 = object1->GetTransform().GetRotMatrix().GetColumn(0);
//	Vector3 b1 = object1->GetTransform().GetRotMatrix().GetColumn(1);
//	Vector3 c1 = object1->GetTransform().GetRotMatrix().GetColumn(2);
//
//	Vector3 a2 = object2->GetTransform().GetRotMatrix().GetColumn(0);
//	Vector3 b2 = object2->GetTransform().GetRotMatrix().GetColumn(1);
//	Vector3 c2 = object2->GetTransform().GetRotMatrix().GetColumn(2);
//
//	float c11 = Vector3::Dot(-Vector3::Cross(b2, a1), AngVelocity1) +
//		Vector3::Dot(Vector3::Cross(b2, a1), AngVelocity2);
//	float c21 = Vector3::Dot(-Vector3::Cross(c2, a1), AngVelocity1) +
//		Vector3::Dot(Vector3::Cross(c2, a1), AngVelocity2);
//
//
//	float a = Vector3::Dot(Vector3::Cross(b2, a1), phys1->GetInertiaTensor() * Vector3::Cross(b2, a1));
//		+ Vector3::Dot(Vector3::Cross(b2, a1), phys2->GetInertiaTensor() * Vector3::Cross(b2, a1));
//
//	float b = Vector3::Dot(Vector3::Cross(b2, a1), phys1->GetInertiaTensor() * Vector3::Cross(c2, a1));
//		+ Vector3::Dot(Vector3::Cross(b2, a1), phys2->GetInertiaTensor() * Vector3::Cross(c2, a1));
//
//	float c = Vector3::Dot(Vector3::Cross(c2, a1), phys1->GetInertiaTensor() * Vector3::Cross(b2, a1));
//		+ Vector3::Dot(Vector3::Cross(c2, a1), phys2->GetInertiaTensor() * Vector3::Cross(b2, a1));
//
//	float d = Vector3::Dot(Vector3::Cross(c2, a1), phys1->GetInertiaTensor() * Vector3::Cross(c2, a1));
//		+ Vector3::Dot(Vector3::Cross(c2, a1), phys2->GetInertiaTensor() * Vector3::Cross(c2, a1));
//
//
//	float JMJ1 = Vector3::Dot(Vector3::Cross(b2, a1), phys1->GetInertiaTensor() * (-Vector3::Cross(b2, a1)));
//		+ Vector3::Dot(Vector3::Cross(b2, a1), phys2->GetInertiaTensor() * Vector3::Cross(b2, a1));
//		float JMJ2 = Vector3::Dot(Vector3::Cross(c2, a1), phys1->GetInertiaTensor() * Vector3::Cross(b2, a1));
//		+Vector3::Dot(Vector3::Cross(c2, a1), phys2->GetInertiaTensor() * Vector3::Cross(b2, a1));
//
//
//	float biasFactor = 0.01f;
//	float bias1 = -(biasFactor / dt) * c11;
//	float bias2 = -(biasFactor / dt) * c21;
//
//	float lambda1 = (c11 + bias1) / JMJ1;
//	float lambda2 = (c21 + bias2) / JMJ2;
//
//	Vector3 impulse1 = -Vector3::Cross(b2, a1) * lambda1 +
//		Vector3::Cross(b2, a1) * lambda2;
//	Vector3 impulse2 = -(-Vector3::Cross(b2, a1) * lambda1 +
//		Vector3::Cross(b2, a1) * lambda2);
//
//	phys1->ApplyLinearImpulse(impulse1);
//	phys2->ApplyLinearImpulse(impulse2);
}
