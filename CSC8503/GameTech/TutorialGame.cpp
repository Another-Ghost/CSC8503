#include "TutorialGame.h"
#include "../CSC8503Common/GameWorld.h"
#include "../CSC8503Common/PositionConstraint.h"
#include "../CSC8503Common/OrientationConstraint.h"
#include "../CSC8503Common/HingedConstraint.h"
#include "../../Common/Quaternion.h"

#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"

#include "../CSC8503Common/NavigationGrid.h"
#include "../CSC8503Common/BonusObject.h"
#include "../CSC8503Common/Pawn.h"

#include <string>

using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame(){
	world		= new GameWorld();
	renderer	= new GameTechRenderer(*world);
	physics		= new PhysicsSystem(*world);

	/*Debug*/
	physics->SetTutorialGame(this);
	/*Debug*/

	forceMagnitude	= 10.0f;
	useGravity = false;
	inSelectionMode = false;

	Debug::SetRenderer(renderer);

	//InitialiseAssets();

	state = GameState::Start;


	scoringCycle = 1;

	deductionNum = 10;

	killY = -20;
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	auto loadFunc = [](const string& name, OGLMesh** into) {
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	loadFunc("cube.msh"		 , &cubeMesh);
	loadFunc("sphere.msh"	 , &sphereMesh);
	loadFunc("Male1.msh"	 , &charMeshA);
	loadFunc("courier.msh"	 , &charMeshB);
	loadFunc("security.msh"	 , &enemyMesh);
	loadFunc("coin.msh"		 , &bonusMesh);
	loadFunc("capsule.msh"	 , &capsuleMesh);

	/**/
	loadFunc("Cylinder.msh", &cylinderMesh);
	loadFunc("b1.msh", &terminusMesh);

	basicTex	= (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");

	//InitCamera();
	//InitWorld();
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete charMeshA;
	delete charMeshB;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
}

void NCL::CSC8503::TutorialGame::Update(float dt)
{
	switch (state)
	{
	case GameState::Start:
		Debug::Print("Little Fall Buddies", Vector2(20, 30), Debug::BLUE);
		Debug::Print("(1) Scores Race Mode", Vector2(20, 40), Debug::BLUE);
		Debug::Print("(2) Racing Mode", Vector2(20, 50), Debug::BLUE);
		//Debug::Print("Press (Space) to Start", Vector2(20, 50), Debug::BLUE);
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM1)) {
			bRacingMode = false;
			world->ClearAndErase();
			InitialiseAssets();
			Restart();
			state = GameState::Running;
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM2)) {
			world->ClearAndErase();
			bRacingMode = true;
			InitialiseAssets();
			Restart();
			state = GameState::Running;
		}

		break;

	case GameState::Running:
 		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::P)) {
			state = GameState::Pause;
		}

		UpdateGame(dt);
		break;

	case GameState::Pause:
		Debug::Print("Press (P) to Resume", Vector2(20, 40), Debug::BLUE);
		Debug::Print("Press (R) to Restart", Vector2(20, 50), Debug::BLUE);
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::P)) {
			state = GameState::Running;
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::R)) {
			state = GameState::Start;
		}
		break;

	case GameState::End:
		if (bWin) {
			Debug::Print("You Win!", Vector2(20, 30), Debug::BLUE);
		}
		else {
			Debug::Print("You Lose!", Vector2(20, 30), Debug::BLUE);
		}
		if (!bRacingMode) {
			Debug::Print("Your Score:" + std::to_string(player->GetScore()), Vector2(20, 40), Debug::BLUE);
		}
		Debug::Print("Press (R) to Restart", Vector2(20, 50), Debug::BLUE);
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::R)) {
			state = GameState::Start;
		}
		//state = GameState::Running;
		break;
	}
	Debug::FlushRenderables(dt);

	renderer->Render();

}

void TutorialGame::UpdateGame(float dt) {
	Debug::Print("Score:" + std::to_string(player->GetScore()), Vector2(60, 10), Debug::BLUE);

	if (!inSelectionMode) {
		world->GetMainCamera()->UpdateCamera(dt);
	}
	UpdateKeys();

	UpdateObjectKeys(dt);

	//enemy->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
	UpdateCharacters(dt);
	//Path finding
	//DisplayPathfinding();

	if (useGravity) {
		Debug::Print("(G)ravity on", Vector2(5, 95));
	}
	else {
		Debug::Print("(G)ravity off", Vector2(5, 95));
	}

	SelectObject();

	/*Physics*/
	if (bDebugMode) {
		MoveSelectedObject();
	}

	if (physics->bPhysics) {
		physics->Update(dt);
	}
	else {
		physics->TestUpdate(dt);
	}


	/*Physics*/

	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera()->SetPosition(camPos);
		world->GetMainCamera()->SetPitch(angles.x);
		world->GetMainCamera()->SetYaw(angles.y);

		//Debug::DrawAxisLines(lockedObject->GetTransform().GetMatrix(), 2.0f);
	}

	if (testStateObject) {
		testStateObject->Update(dt);
	}


	renderer->Update(dt);
	world->UpdateWorld(dt);

	//Debug::FlushRenderables(dt);
	//renderer->Render();
}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
		lockedObject	= nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::C)) {
		physics->bPhysics = !physics->bPhysics;
		std::cout << "Setting bPhysics to " << physics->bPhysics << std::endl;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM0)) {
		bDebugMode = !bDebugMode;
		world->SetDebugMode(bDebugMode);
		std::cout << "Setting DebugMode to " << bDebugMode << std::endl;
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void NCL::CSC8503::TutorialGame::UpdateObjectKeys(float dt)
{
	if (selectionObject) {
		float speed = dt * 5.0f;
		Vector3 pos = selectionObject->GetTransform().GetPosition();
		//if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		//	selectionObject->GetTransform().SetPosition(Vector3(pos.x, pos.y, pos.z + speed));
		//}
		//if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		//	selectionObject->GetTransform().SetPosition(Vector3(pos.x, pos.y, pos.z - speed));
		//}
		//if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
		//	selectionObject->GetTransform().SetPosition(Vector3(pos.x + speed, pos.y, pos.z));
		//}
		//if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
		//	selectionObject->GetTransform().SetPosition(Vector3(pos.x - speed, pos.y, pos.z));
		//}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::Z)) {
			selectionObject->GetTransform().SetPosition(Vector3(pos.x, pos.y + speed, pos.z));
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::X)) {
			selectionObject->GetTransform().SetPosition(Vector3(pos.x, pos.y - speed, pos.z));
		}

	}
}

void NCL::CSC8503::TutorialGame::UpdateCharacters(float dt)
{
	for (auto enemy : enemys) {
		if (enemy) {
			enemy->Update(dt);
		}
	}

	if (player) {
		player->Update(dt);
	}

	curScoringTime += dt;
	if (curScoringTime > scoringCycle) {
		curScoringTime -= scoringCycle;
		for (auto enemy : enemys) {
			if (enemy) {
				if (enemy->Finished()) {}
				enemy->SetScore(enemy->GetScore() - deductionNum);
			}
		}
		if (player) {
			player->SetScore(player->GetScore() - deductionNum);

			if (player->GetScore() == 0) {
				bWin = false;
				state = GameState::End;
			}
		}
	}

}



void TutorialGame::LockedObjectMovement() {
	Matrix4 view		= world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld	= view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();

	Vector3 charForward  = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);
	Vector3 charForward2 = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);

	float force = 100.0f;

	//if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
	//	lockedObject->GetPhysicsObject()->AddForce(-rightAxis * force);
	//}

	//if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
	//	Vector3 worldPos = selectionObject->GetTransform().GetPosition();
	//	lockedObject->GetPhysicsObject()->AddForce(rightAxis * force);
	//}

	//if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
	//	lockedObject->GetPhysicsObject()->AddForce(fwdAxis * force);
	//}

	//if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
	//	lockedObject->GetPhysicsObject()->AddForce(-fwdAxis * force);
	//}

	//if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NEXT)) {
	//	lockedObject->GetPhysicsObject()->AddForce(Vector3(0,-10,0));
	//}
}

void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}

}

void TutorialGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.1f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	curScoringTime = 0;
	bWin = false;
	remainingPlaces = 2;

	world->ClearAndErase();
	physics->Clear();

	//InitMixedGridWorld(5, 5, 3.5f, 3.5f);

	InitDefaultFloor();

	//BridgeConstraintTest();

	//HingedConstraintTest();
	//testStateObject = AddStateObjectToWorld(Vector3(0, 10, 0));

	//Path finding
	//TestPathfinding();
	InitTestWorld();

}

void NCL::CSC8503::TutorialGame::Restart()
{
	enemys.clear();
	enemyScore.clear();
	finishers.clear();

	InitCamera();
	InitWorld();
}


void NCL::CSC8503::TutorialGame::InitTestWorld()
{


	//Vector3 cubePosition1(1, 5, 0);
	////Vector3 cubeDims(1, 1, 1);
	//AddCubeToWorld(cubePosition1, cubeDims);

	InitMap();
	InitScene();
	
	InitPositionConstraintObstacles();

	
	selectionObject = player;
	lockedObject = player;
	useGravity = true; 
	physics->UseGravity(useGravity);
}


//void NCL::CSC8503::TutorialGame::AddDebugPoint(const Vector3& position, float radius, const Vector4& colour, float time)
//{
//	GameObject* sphere = new GameObject("sphere");
//	
//	SphereVolume* volume = new SphereVolume(radius);
//	sphere->SetBoundingVolume((CollisionVolume*)volume);
//
//	sphere->GetTransform()
//		.SetScale(Vector3(1, 1, 1))
//		.SetPosition(position);
//
//	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
//
//	sphere->GetRenderObject()->SetColour(colour);
//
//	DebugObject* point = new DebugObject(sphere, time);
//
//	debugObjectList.push_back(point);
//
//	//world->AddGameObject(sphere);
//}

//void NCL::CSC8503::TutorialGame::UpdateDebugObject(float dt)
//{
//	vector<DebugObject*>::const_iterator first = debugObjectList.begin();
//	vector<DebugObject*>::const_iterator last = debugObjectList.end();
//
//	for (auto i = first; i != last; ++i) {
//		(*i)->time -= dt;
//		if ((*i)->time < 0) {
//			world->RemoveGameObject((*i)->object);
//			debugObjectList.erase(i);
//		}
//	}
//	//for (auto i : debugObjectList) {
//	//	i->time -= dt;
//	//	if (i->time < 0) {
//	//		world->RemoveGameObject(i->object);
//	//		//debugObjectList.erase(&i);
//	//	}
//	//}
//}

/*
A single function to add a large immoveable cube to the bottom of our world
*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject("floor");

	Vector3 floorSize	= Vector3(50, 2, 50);
	AABBVolume* volume	= new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));
	//floor->GetPhysicsObject()->SeteElasticity(0.5f);


	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	floor->GetPhysicsObject()->SetPhysicsType(PhysicsType::Static);

	world->AddGameObject(floor);

	return floor;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject("sphere");

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetRenderObject()->SetColour(Vector4(0, 1, 1, 1));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	return sphere;
}


GameObject* TutorialGame::AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass) {
	GameObject* capsule = new GameObject("capsule");

	CapsuleVolume* volume = new CapsuleVolume(halfHeight, radius);
	capsule->SetBoundingVolume((CollisionVolume*)volume);

	capsule->GetTransform()
		.SetScale(Vector3(radius* 2, halfHeight, radius * 2))
		.SetPosition(position);

	capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, basicTex, basicShader));
	capsule->SetPhysicsObject(new PhysicsObject(&capsule->GetTransform(), capsule->GetBoundingVolume()));

	capsule->GetRenderObject()->SetColour(Vector4(0, 1, 1, 1));

	capsule->GetPhysicsObject()->SetInverseMass(inverseMass);
	capsule->GetPhysicsObject()->InitCubeInertia(); //Is this OK?

	world->AddGameObject(capsule);

	return capsule;

}

GameObject* NCL::CSC8503::TutorialGame::AddCylinderToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass)
{
	GameObject* cylinder = new GameObject("cylinder");

	CylinderVolume* volume = new CylinderVolume(halfHeight, radius);
	cylinder->SetBoundingVolume((CollisionVolume*)volume);

	cylinder->GetTransform()
		.SetScale(Vector3(radius * 2, halfHeight, radius * 2))
		.SetPosition(position);

	cylinder->SetRenderObject(new RenderObject(&cylinder->GetTransform(), cylinderMesh, basicTex, basicShader));
	cylinder->SetPhysicsObject(new PhysicsObject(&cylinder->GetTransform(), cylinder->GetBoundingVolume()));

	cylinder->GetRenderObject()->SetColour(Vector4(0, 1, 1, 1));

	cylinder->GetPhysicsObject()->SetInverseMass(inverseMass);
	cylinder->GetPhysicsObject()->InitCubeInertia(); //Is this OK?

	world->AddGameObject(cylinder);

	return cylinder;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, bool bStatic, float elasticity) {
	GameObject* cube = new GameObject("cube");

	AABBVolume* volume = new AABBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	if (!bStatic) {
		cube->GetRenderObject()->SetColour(Vector4(0, 1, 1, 1));
	}

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();
	cube->GetPhysicsObject()->SetElasticity(elasticity);

	if (bStatic) {
		cube->GetPhysicsObject()->SetPhysicsType(PhysicsType::Static);
	}
	//if (bStatic) {
	//	cube->GetPhysicsObject()->SetInverseMass(0);
	//}

	world->AddGameObject(cube);

	return cube;
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	/*Capsule*/
	float capsuleHalfHeight = 2.0f;
	float capsuleRadius = 1.0f;
	/*Capsule*/

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			//if (rand() % 2) {
			//	AddCubeToWorld(position, cubeDims);
			//}
			//else {
			//	AddSphereToWorld(position, sphereRadius);
			//}
			switch (rand() % 3)
			{
			case 0:
				AddCubeToWorld(position, cubeDims);
				break;
			case 1:
				AddSphereToWorld(position, sphereRadius);
				break;
			case 2:
				AddCapsuleToWorld(position, capsuleHalfHeight, capsuleRadius);
				break;
			}
		}
	}
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(45, -4, 45));
}

void NCL::CSC8503::TutorialGame::InitPositionConstraintObstacles()
{
	float halfheight = 2;
	float radius = 1;

	float invCubeMass = 2; // how heavy the middle pieces are
	int numLinks = 5;
	float maxDistance = 4; // constraint distance
	float cubeDistance = 2; // distance between links

	Vector3 startPos = Vector3(50, 3, 10);

	GameObject* start = AddCapsuleToWorld(startPos + Vector3(0, 0, 0)
		, halfheight, radius, 0);
	GameObject* end = AddCapsuleToWorld(startPos + Vector3(0, 0, (numLinks + 1)
		* cubeDistance), halfheight, radius, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCapsuleToWorld(startPos + Vector3(0, 0, (i + 1) *
			cubeDistance), halfheight, radius, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous,
			block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}
	PositionConstraint* constraint = new PositionConstraint(previous,
		end, maxDistance);
	world->AddConstraint(constraint);

}

void TutorialGame::BridgeConstraintTest() {
	Vector3 cubeSize = Vector3(2, 2, 4);

	float invCubeMass = 10; // how heavy the middle pieces are
	int numLinks = 10;
	float maxDistance = 30; // constraint distance
	float cubeDistance = 20; // distance between links

	Vector3 startPos = Vector3(-50, 100, 50);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0)
		, cubeSize, 0);
	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2)
		* cubeDistance, 0, 0), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) *
			cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous,
			block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}
	PositionConstraint* constraint = new PositionConstraint(previous,
		end, maxDistance);
	world->AddConstraint(constraint);
}

void NCL::CSC8503::TutorialGame::HingedConstraintTest()
{
	Vector3 cubeSize = Vector3(4, 4, 4);

	float invCubeMass = 5; // how heavy the middle pieces are
	int numLinks = 2;
	float maxDistance = 30; // constraint distance
	float cubeDistance = 5; // distance between links

	Vector3 startPos = Vector3(-20, 5, 20);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0)
		, cubeSize, 0);
	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2)
		* cubeDistance, 20, 0), cubeSize, invCubeMass);
	end->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(45, 45, 45));

	GameObject* previous = start;

	//for (int i = 0; i < numLinks; ++i) {
	//	GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) *
	//		cubeDistance, 0, 0), cubeSize, invCubeMass);
	//	OrientationConstraint* constraint = new OrientationConstraint(previous,
	//		block);
	//	world->AddConstraint(constraint);
	//	previous = block;
	//}
	//OrientationConstraint* constraint = new OrientationConstraint(start,
	//	end);
	HingedConstraint* constraint = new HingedConstraint(start, end);
	world->AddConstraint(constraint);
}


//void TutorialGame::InitGameExamples() {
//	AddPlayerToWorld(Vector3(10, 5, 10));
//	AddEnemyToWorld(Vector3(50, 5, 10));
//	
//	//AddBonusToWorld(Vector3(80, 5, 80));
//}

Player* TutorialGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize = 3.0f;
	float inverseMass = 0.2f;

	player = new Player(DestinationPosition, "player", this);

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.85f, 0.3f) * meshSize);

	player->SetBoundingVolume((CollisionVolume*)volume);

	player->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	//if (rand() % 2) {
	//	player->SetRenderObject(new RenderObject(&player->GetTransform(), charMeshA, nullptr, basicShader));
	//}
	//else {
		player->SetRenderObject(new RenderObject(&player->GetTransform(), charMeshB, nullptr, basicShader));
	//}

	player->GetRenderObject()->SetColour(Vector4(0, 0.5, 0, 1));

	player->SetPhysicsObject(new PhysicsObject(&player->GetTransform(), player->GetBoundingVolume(), PhysicsType::Pawn));

	player->GetPhysicsObject()->SetInverseMass(inverseMass);
	player->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(player);

	//lockedObject = character;

	return player;
}

Pawn* TutorialGame::AddEnemyToWorld(const Vector3& position) {
	float meshSize		= 3.0f;
	float inverseMass	= 0.5f;

	Pawn* enemy = new Pawn(navGrid, player, DestinationPosition, "enemy", this);

	CapsuleVolume* volume = new CapsuleVolume(1.0 * meshSize, 0.5*meshSize);
	enemy->SetBoundingVolume((CollisionVolume*)volume);

	enemy->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	enemy->SetRenderObject(new RenderObject(&enemy->GetTransform(), enemyMesh, nullptr, basicShader));
	enemy->SetPhysicsObject(new PhysicsObject(&enemy->GetTransform(), enemy->GetBoundingVolume(), PhysicsType::Pawn));

	enemy->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));

	enemy->GetPhysicsObject()->SetInverseMass(inverseMass);
	enemy->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(enemy);

	enemys.push_back(enemy);

	return enemy;
}

BonusObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
	BonusObject* apple = new BonusObject("bonus", this);

	SphereVolume* volume = new SphereVolume(1.0f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(0.5, 0.5, 0.5))
		.SetPosition(position);

	//apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume(), PhysicsType::Disable));

	apple->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));

	apple->GetPhysicsObject()->SetInverseMass(0);
	apple->GetPhysicsObject()->InitSphereInertia();

	//GetPhysicsObject()->SetPhysicsType(PhysicsType::Disable);

	world->AddGameObject(apple);

	return apple;
}

GameObject* NCL::CSC8503::TutorialGame::AddTerminusToWorld(const Vector3& position)
{
	GameObject* apple = new GameObject("terminus", this);

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(1, 1, 1))
		.SetPosition(position);

	//apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), terminusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume(), PhysicsType::Disable));

	apple->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));

	apple->GetPhysicsObject()->SetInverseMass(0);
	apple->GetPhysicsObject()->InitSphereInertia();

	//GetPhysicsObject()->SetPhysicsType(PhysicsType::Disable);

	world->AddGameObject(apple);

	return apple;
}

StateGameObject* NCL::CSC8503::TutorialGame::AddStateObjectToWorld(const Vector3& position)
{
	StateGameObject* apple = new StateGameObject();

	SphereVolume* volume = new SphereVolume(0.25f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(0.25, 0.25, 0.25))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

//void NCL::CSC8503::TutorialGame::TestPathfinding()
//{
//		NavigationGrid grid("TestGrid1.txt");
//
//		NavigationPath outPath;
//
//		Vector3 startPos(80, 0, 10);
//		Vector3 endPos(80, 0, 80);
//
//		bool found = grid.FindPath(startPos, endPos, outPath);
//
//		Vector3 pos;
//		while (outPath.PopWaypoint(pos)) {
//			testNodes.push_back(pos);
//		}
//}

void NCL::CSC8503::TutorialGame::InitMap()
{
	navGrid = new NavigationGrid("TestGrid1.txt");
	GridNode* nodes = navGrid->GetNodes();
	int gridWidth = navGrid->GetWidth();
	int gridHeight = navGrid->GetHeight();
	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {
			GridNode& n = nodes[(gridWidth * y) + x];
			if (n.gridType == GRIDTYPE::Block) {
				AddCubeToWorld(n.position, Vector3(0.5, 0.5, 0.5)* navGrid->nodeSize, 0, true, 0.6);
				//AddCubeToWorld(Vector3(x * 2, 1, y * 2), Vector3(1, 1, 1), 0, true);
			}
		}
	}
}

void NCL::CSC8503::TutorialGame::InitScene()
{
	DestinationPosition = Vector3(80, 5, 80);
	AddTerminusToWorld(Vector3(DestinationPosition.x + 3, -5, DestinationPosition.z + 3));

	if(!bRacingMode){
	BonusObject* bonusObj;
	//bonusObj = AddBonusToWorld(Vector3(80, 5, 80));
	//bonusObjects.push_back(bonusObj);

	bonusObj = AddBonusToWorld(Vector3(70, 3, 50));

	bonusObj = AddBonusToWorld(Vector3(40, 3, 30));

	bonusObj = AddBonusToWorld(Vector3(30, 3, 40));

	bonusObj = AddBonusToWorld(Vector3(20, 3, 40));

	bonusObj = AddBonusToWorld(Vector3(60, 3, 50));

	bonusObj = AddBonusToWorld(Vector3(30, 3, 80));
	}

	Vector3 cubePosition(20, 5, 20);
	Vector3 cubeDims(1, 1, 1);
	AddCubeToWorld(cubePosition, cubeDims);

	cubePosition = Vector3(25, 5, 25);
	//Vector3 cubeDims(1, 1, 1);
	AddCubeToWorld(cubePosition, cubeDims);

	Vector3 spherePositon(30, 5, 10);
	float sphereRadius = 1.0f;
	AddSphereToWorld(spherePositon, sphereRadius);

	spherePositon = Vector3(80, 5, 40);
	//float sphereRadius = 1.0f;
	AddSphereToWorld(spherePositon, sphereRadius);

	Vector3 capsulePosition(10, 5, 30);
	float capsuleHalfHeight = 2.0f;
	float capsuleRadius = 1.0f;
	AddCapsuleToWorld(capsulePosition, capsuleHalfHeight, capsuleRadius);

	capsulePosition = Vector3(10, 5, 40);
	//float capsuleHalfHeight = 2.0f;
	//float capsuleRadius = 1.0f;
	AddCapsuleToWorld(capsulePosition, capsuleHalfHeight, capsuleRadius);

	Vector3 cylinderPosition(30, 5, 30);
	float cylinderHalfHeight = 1.0f;
	float cylinderRadius = 1.0f;
	AddCylinderToWorld(cylinderPosition, cylinderHalfHeight, cylinderRadius);

	cylinderPosition = Vector3(20, 5, 30);
	//float cylinderHalfHeight = 1.0f;
	//float cylinderRadius = 1.0f;
	AddCylinderToWorld(cylinderPosition, cylinderHalfHeight, cylinderRadius);

	AddPlayerToWorld(Vector3(35, 5, 15));
	//AddEnemyToWorld(Vector3(20, 5, 10));
	AddEnemyToWorld(Vector3(70, 5, 20));
	AddEnemyToWorld(Vector3(10, 5, 25));
}

//void NCL::CSC8503::TutorialGame::DisplayPathfinding()
//{
//	for (int i = 1; i < testNodes.size(); ++i) {
//		Vector3 a = testNodes[i - 1];
//		Vector3 b = testNodes[i];
//
//		Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
//	}
//}

void NCL::CSC8503::TutorialGame::OnCharacterFinished(GameObject* ch)
{
	if (bRacingMode) {
		if (player->Finished()) {
			bWin = true;
		}
		else {
			bWin = false;
		}
		state = GameState::End;
		return;
	}

	remainingPlaces--;
	finishers.push_back(ch);
	int score = 0;
	if (remainingPlaces == 0) {
		if (player->Finished()) {
			score = player->GetScore();
			for (auto finisher : finishers) {
				if (finisher->GetName() == "enemy") {
					Pawn* enemy = (Pawn*)finisher;
					if (enemy->GetScore() > score) {
						bWin = false;
						break;
					}
				}
			}
			bWin = true;
		}
		else {
			bWin = false;
		}
		state = GameState::End;
	}
}

/*

Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be 
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around. 

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		renderer->DrawString("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
				lockedObject	= nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;
				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));

				if (bDebugMode) {
					Debug::DrawLine(ray.GetPosition(), closestCollision.collidedAt, Debug::RED, 60.0f);
				}

				return true;
			}
			else {
				return false;
			}
		}
	}
	else {
		renderer->DrawString("Press Q to change to select mode!", Vector2(5, 85));
	}

	if (lockedObject) {
		renderer->DrawString("Press L to unlock object!", Vector2(5, 80));
	}

	else if(selectionObject){
		renderer->DrawString("Press L to lock selected object object!", Vector2(5, 80));
		renderer->DrawString("Selected object type: " + 
			std::to_string((int)selectionObject->GetBoundingVolume()->type), Vector2(5, 75));
	}

	if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
		if (selectionObject) {
			if (lockedObject == selectionObject) {
				lockedObject = nullptr;
			}
			else {
				lockedObject = selectionObject;
			}
		}

	}

	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/
void TutorialGame::MoveSelectedObject() {
	renderer->DrawString("Click Force: " + std::to_string(forceMagnitude),
		Vector2(10, 20)); // Draw debug text at 10 ,20
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject) {
		return;// we haven ¡¯t selected anything !
	}
	// Push the selected object !
	if (Window::GetMouse() -> ButtonPressed(NCL::MouseButtons::RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(
			*world -> GetMainCamera());
		RayCollision closestCollision;
		if (world -> Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject() ->
					//AddForce(ray.GetDirection() * forceMagnitude);
					AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}
}
