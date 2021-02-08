#pragma once
#include "GameObject.h"

#include "../CSC8503Common/BehaviourAction.h"
#include "BehaviourSequence.h"
#include "BehaviourSelector.h"

#include "NavigationPath.h"
#include "NavigationGrid.h"
//#include "BonusObject.h"

namespace NCL {
	namespace CSC8503 {
		
		
		class Pawn : public GameObject
		{
		public:
			Pawn(NavigationGrid* navGrid_, GameObject* player_, Vector3 destination = Vector3(80, 5, 80), string name = "", TutorialGame* tutorialGame = nullptr) :GameObject(name, tutorialGame) {	
				navGrid = navGrid_;
				destinationPos = destination;

				player = player_;

				updateCycle = 1/10.0;
				updateTime = 0;
				averageVelocity = 10;

				score = 1000;
				bFinished = false;
				InitBehaviourTree();

				bMoveToBonus = false;
				bAvoidObstacle = false;

			}

			void OnCollisionBegin(GameObject* otherObject) override;

			void Update(float dt);

			void SetScore(const int s) { score = s;  if (score < 0) { score = 0; } }
			int GetScore() { return score; }

			bool Finished() { return bFinished; }

			void Respawn();

		protected:

			float updateCycle;
			float updateTime;

			int score;
			
			bool bFinished;


		/*Behaviour Tree*/
			BehaviourNodeWithChildren* BehaviourRoot;

			void InitBehaviourTree();

			void UpdateBehaviourTree();
			//BehaviourState MoveToDesination(float dt, BehaviourState state);

			void DriveToPosition(const Vector3 position);
			
			bool bMoveToBonus;
			bool bAvoidObstacle;

			GameObject* player;
			
		/*Navigation*/
			NavigationGrid* navGrid;

			NavigationPath path;

			Vector3 curWayPoint;

			float averageVelocity;

			//vector<BonusObject*> bonusObjects;

			Vector3 destinationPos;

			Vector3 curTargetPos;

		
		};


	}
}
