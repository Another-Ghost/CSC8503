#include "Player.h"
#include "../../Common/Window.h"
#include "../GameTech/TutorialGame.h"

void NCL::CSC8503::Player::OnCollisionBegin(GameObject* otherObject)
{
	if (otherObject->GetName() == "bonus") {
		score += 25;
	}

}

void NCL::CSC8503::Player::Update(float dt)
{
	//time += dt;
	//if (time > cycle) {
	//	time -= cycle;

	//	//
	//	score -= 10;
	//	if (score < 0) {
	//		score = 0;
	//	}
	//}
	if (transform.GetPosition().y < game->GetKillY()) {
		Respawn();
	}

	if ((destinationPos - transform.GetPosition()).Length() < 5) {
		bFinished = true;
		game->OnCharacterFinished(this);
	}

	UpdateMovement(dt);

}

void NCL::CSC8503::Player::Respawn()
{
	transform.SetPosition(Vector3(10, 5, 10));
}

void NCL::CSC8503::Player::UpdateMovement(float dt)
{
	float frameSpeed = 2000 * dt;

	float yaw = -(Window::GetMouse()->GetRelativePosition().x);
	//Quaternion dOrientation = Quaternion::EulerAnglesToQuaternion(0.0, yaw, 0.0);
	physicsObject->AddTorque(Vector3(0, yaw, 0)*10);

	//float scale = 20;
	Vector3 forwardForce = transform.GetRotMatrix() * Vector3(0, 0, -1) * frameSpeed;
	Vector3 rightForce = transform.GetRotMatrix() * Vector3(1, 0, 0) * frameSpeed;

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		//position += Matrix4::Rotation(yaw, Vector3(0, 1, 0)) * Vector3(0, 0, -1) * frameSpeed;
		physicsObject->AddForce(forwardForce);
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		physicsObject->AddForce(-forwardForce);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
		physicsObject->AddForce(rightForce);
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
		physicsObject->AddForce(-rightForce);
	}
}
