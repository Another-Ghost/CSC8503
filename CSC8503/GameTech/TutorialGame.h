#pragma once
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "../GameTech/StateGameObject.h"
#include "../CSC8503Common/NavigationGrid.h"

#include "../CSC8503Common/Player.h"

#include "../CSC8503Common/PushdownState.h"
#include "../CSC8503Common/PushdownMachine.h"

namespace NCL {
	namespace CSC8503 {

		enum class GameState {
			Start,
			Running,
			Pause,
			End,
		};

		class BonusObject;
		class Pawn;

		class TutorialGame {
		public:
			TutorialGame();
			~TutorialGame();

			virtual void Update(float dt);

			virtual void UpdateGame(float dt);

		protected:
			void UpdateKeys();


			void UpdateObjectKeys(float dt);

			void UpdateCharacters(float dt);


			void InitialiseAssets();

			void InitCamera();

			void InitWorld();

			void Restart();
			//void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			void InitDefaultFloor();

			void InitPositionConstraintObstacles();

			void BridgeConstraintTest();

			void HingedConstraintTest();

			void InitTestWorld();

			//void InitDebugObjects();
			
		//public:
		//	void AddDebugPoint(const Vector3& position, float radius = 0.1,const Vector4& colour = Vector4(1, 0, 0, 1), float time = 1.0f);
		//protected:
		//	void UpdateDebugObject(float dt);

			//struct DebugObject {
			//	DebugObject(GameObject* object_, float time_):object(object_), time(time_){}

			//	GameObject* object;
			//	float time;
			//};

			//vector<DebugObject*> debugObjectList;
			


	
			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();

			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f, bool bStatic = false, float elasticity = 0.8f);
			
			/**/
			GameObject* AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass = 10.0f);
			GameObject* AddCylinderToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass = 10.0f);


			Player* AddPlayerToWorld(const Vector3& position);
			Pawn* AddEnemyToWorld(const Vector3& position);
			BonusObject* AddBonusToWorld(const Vector3& position);
			GameObject* AddTerminusToWorld(const Vector3& position);

			GameTechRenderer*	renderer;
			PhysicsSystem*		physics;
			GameWorld*			world;


			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;

			OGLMesh*	capsuleMesh = nullptr;
			OGLMesh*	cubeMesh	= nullptr;
			OGLMesh*	sphereMesh	= nullptr;
			OGLTexture* basicTex	= nullptr;
			OGLShader*	basicShader = nullptr;

			//Coursework Meshes
			OGLMesh*	charMeshA	= nullptr;
			OGLMesh*	charMeshB	= nullptr;
			OGLMesh*	enemyMesh	= nullptr;
			OGLMesh*	bonusMesh	= nullptr;

			/**/
			OGLMesh* cylinderMesh = nullptr;
			OGLMesh* terminusMesh = nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 35, -10);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}
			
			//StateMachine
			StateGameObject * AddStateObjectToWorld (const Vector3 & position);
			StateGameObject * testStateObject;

			/*Path finding*/
		protected:
			//vector<Vector3> testNodes;
			//void TestPathfinding();

			void InitMap();

			void InitScene();

			NavigationGrid* navGrid;
			/*Path finding*/


			void DisplayPathfinding();

			/*Game Scene*/
		public:
			//vector<BonusObject*> GetBonusObjects() { return bonusObjects; }

		protected:
			//vector<BonusObject*> bonusObjects;
			Player* player;

			vector<Pawn*> enemys;

			
			/*Game Scene*/


			/*Game State*/
		public:
			GameWorld* GetWorld() { return world; }

			void OnCharacterFinished(GameObject* ch);

			bool RacingMode() { return bRacingMode; }

			float GetKillY() { return killY; }

			bool DebugMode() { return bDebugMode; }

		protected:
			Vector3 DestinationPosition;

			int playerScore;
			vector<int> enemyScore;		

			GameState state;

			float scoringCycle;
			float curScoringTime;
			float deductionNum;

			bool bWin;

			int remainingPlaces;
			vector<GameObject*> finishers;

			bool bRacingMode; //Whoever reaches the finish line first wins the game

			float killY;
			/*Game State*/

			bool bDebugMode;
		};
	}
}

