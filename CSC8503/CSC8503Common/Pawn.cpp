#include "Pawn.h"
#include "../../Common/Maths.h"
#include "../GameTech/TutorialGame.h"
#include "Debug.h"
#include "CollisionDetection.h"
#include "GameWorld.h"

void NCL::CSC8503::Pawn::OnCollisionBegin(GameObject* otherObject)
{
	if (otherObject->GetName() == "bonus") {
		score += 25;
	}
}

void NCL::CSC8503::Pawn::Update(float dt)
{
	if (transform.GetPosition().y < game->GetKillY()) {
		Respawn();
	}
	updateTime += dt;
	if (updateTime >= updateCycle) {
			UpdateBehaviourTree();
		updateTime -= updateCycle;
	}
}

void NCL::CSC8503::Pawn::Respawn()
{
	BehaviourRoot->Reset();
	transform.SetPosition(Vector3(10, 5, 10));
}

void NCL::CSC8503::Pawn::InitBehaviourTree()
{
	BehaviourRoot = new BehaviourSequence("Move");

	//auto f = std::bind(&NCL::CSC8503::Pawn::MoveToDesination, this);
	BehaviourAction* MoveToDest = new BehaviourAction("Move to Destination",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) {
				if (!bFinished) {
					if (navGrid->FindPath(transform.GetPosition(), destinationPos, path)) {
						if (path.PopWaypoint(curWayPoint)) {
							state = Ongoing;
						}
						else {
							curWayPoint = destinationPos;
							state = Success;
							//bFinished = true;
							//game->OnCharacterFinished(this);
							//physicsObject->SetPhysicsType(PhysicsType::Disable);
							//BehaviourRoot->Reset();
						}
					}
					else {
						state = Failure;
					}
				}
			}
			else if (state == Ongoing) {
				if (navGrid->ReachedWayPoint(transform.GetPosition(), curWayPoint)) {
					if (path.PopWaypoint(curWayPoint)) {
						DriveToPosition(curWayPoint);
					}
					else {
						curWayPoint = destinationPos;
						state = Success;
						//bFinished = true;
						//game->OnCharacterFinished(this);
						//physicsObject->SetPhysicsType(PhysicsType::Disable);
						//BehaviourRoot->Reset();
					}
				}
				else {
					DriveToPosition(curWayPoint);
				}
				//Vector3 path.PopWaypoint()
				//physicsObject->AddForce();
			}
			else if (state == Success) {
				//state = Failure;
				bFinished = true;
				game->OnCharacterFinished(this);
				physicsObject->SetPhysicsType(PhysicsType::Disable);
				BehaviourRoot->Reset();
			}

			return state;
		}
	);

	BehaviourAction* moveToBonus = new BehaviourAction("Move to Bonus",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) {
				if (bMoveToBonus == true) {
					if (navGrid->FindPath(transform.GetPosition(), curTargetPos, path)) {
						if (path.PopWaypoint(curWayPoint)) {
							state = Ongoing;
						}
						else {
							state = Success;							
						}
					}
					else {
						state = Failure;
					}
				}
				else {
					state = Failure;
				}
				bMoveToBonus = false;
			}
			else if (state == Ongoing) {
				if (navGrid->ReachedWayPoint(transform.GetPosition(), curWayPoint)) {
					if (path.PopWaypoint(curWayPoint)) {
						DriveToPosition(curWayPoint);
					}
					else {
						state = Success;					
					}
				}
				else {
					DriveToPosition(curWayPoint);
				}
			}
			else if (state == Success) {
				BehaviourRoot->Reset();
			}
			return state;
		}
	);

	BehaviourAction*  AvoidObstacle = new BehaviourAction("Avoid Obstacle",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) {
				if (bAvoidObstacle == true) {
					Vector3 obstacleDir = (curTargetPos - transform.GetPosition()).Normalised();
					Vector3 newDir = Vector3::Cross(Vector3(0, 1, 0), obstacleDir).Normalised();
					if (rand() / 2) {
						newDir = -newDir;
					}
					curWayPoint = transform.GetPosition() + newDir * 10;
					state = Ongoing;
				}
				//else {
				//	state = Failure;
				//}
				bAvoidObstacle = false;
			}
			else if (state == Ongoing) {
				if (navGrid->ReachedWayPoint(transform.GetPosition(), curWayPoint)) {
					state = Success;
				}
				else {
					DriveToPosition(curWayPoint);
				}
			}
			else if (state == Success) {
				state = Failure;
				BehaviourRoot->Reset();
			}
			return state;
		}
	);

	BehaviourSelector* move = new BehaviourSelector("Move");


	move->AddChild(AvoidObstacle);
	move->AddChild(moveToBonus);
	move->AddChild(MoveToDest);

	BehaviourRoot->AddChild(move);

}

void NCL::CSC8503::Pawn::UpdateBehaviourTree()
{
	//vector<BonusObject*> bonusObjects = game->GetBonusObjects();
	//vector <BonusObject*>::const_iterator first = bonusObjects.begin();
	//vector <BonusObject*>::const_iterator last = bonusObjects.end();
	//for (auto i = first; i != last; ++i) {
	//	//Vector3  (*i)->GetTransform().GetPosition()
	//}
	//NavigationPath tempPath;
	//navGrid->FindPath(transform.GetPosition(), destinationPos, tempPath);
	Vector3 position = transform.GetPosition();
	Vector3 rayDir = Vector3((curWayPoint - position).x, 0, (curWayPoint - position).z).Normalised();
	Ray r(position, rayDir);
	if (game->DebugMode()) {
		Debug::DrawLine(r.GetPosition(), curWayPoint, Debug::BLUE, updateCycle);
	}
	RayCollision collisionInfo;
	if (game && game->GetWorld()) {
		game->GetWorld()->Raycast(r, collisionInfo);
		GameObject* viewObject = (GameObject*)collisionInfo.node;
		
		if (viewObject) {
			float distance = (viewObject->GetTransform().GetPosition() - position).Length();
			if (!game->RacingMode()) {
				if (distance < 20) {
					if (viewObject->GetName() == "bonus") {
						curTargetPos = viewObject->GetTransform().GetPosition();
						bMoveToBonus = true;
						BehaviourRoot->Reset();
						if (game->DebugMode()) {
							std::cout << "Moving to Bonus" << std::endl;
						}
					}
				}
			}
			if (distance < 10) {
				if (viewObject->GetPhysicsObject()->GetPhysicsType() == PhysicsType::Dynamic) {
					curTargetPos = viewObject->GetTransform().GetPosition();
					bAvoidObstacle = true;
					BehaviourRoot->Reset();
					if (game->DebugMode()) {
						std::cout << "Avoiding a Obstacle" << std::endl;
					}
				}
			}
		}
	}
	BehaviourRoot->Execute(1.0f);
}

void NCL::CSC8503::Pawn::DriveToPosition(const Vector3 position)
{
	if (bFinished) {
		return;
	}

	Vector3 moveDir = (position - transform.GetPosition()).Normalised();
	float magnitude = averageVelocity / (updateCycle * physicsObject->GetInverseMass())* moveDir.Length();
	physicsObject->AddForce(moveDir.Normalised() * (position - transform.GetPosition()).Length()*4);

	Vector3 forward = transform.GetRotMatrix()* Vector3(0, 0, -1);
	forward.y = 0;
	moveDir.y = 0;

	int rotDir = 1;
	if (Vector3::Cross(forward, moveDir).y < 0) {
		rotDir = -1;
	}


	float rotMag = RadiansToDegrees(acos(Vector3::Dot(forward, moveDir) / (forward.Length() + moveDir.Length())))*3;
	//float angle = RadiansToDegrees(acos(Vector3::Dot(Vector3(0,0,1), )))
	if (game->DebugMode()) {
		Debug::DrawLine(GetTransform().GetPosition(), GetTransform().GetPosition() + moveDir * 10, Debug::RED, updateCycle);
		Debug::DrawLine(GetTransform().GetPosition(), GetTransform().GetPosition() + forward * 10, Debug::GREEN, updateCycle);
	}

	physicsObject->AddTorque(Vector3(0,rotDir, 0)* rotMag);
}





