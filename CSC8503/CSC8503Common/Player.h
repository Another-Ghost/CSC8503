#pragma once
#include "GameObject.h"

namespace NCL {
	namespace CSC8503 {
		class Player : public GameObject
		{
		public:
			Player(Vector3 destPos = Vector3(80,5,80), string name = "", TutorialGame* tutorialGame = nullptr) :GameObject(name, tutorialGame){
				score = 1000;
				float cycle = 1;
				float time = 0;
				bFinished = false;
				destinationPos = destPos;
			}

			void OnCollisionBegin(GameObject* otherObject) override;

			void SetScore(const int s) { score = s;  if (score < 0) { score = 0; } }
			int GetScore() { return score; }

			void Update(float dt) override;

			bool Finished() { return bFinished; }

			void Respawn();

		protected:
			void UpdateMovement(float dt);

			int score;
			bool bFinished;

			float cycle;
			float time;
			
			Vector3 destinationPos;
		};
	}
}