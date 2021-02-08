#pragma once
#include "GameObject.h"
#include "../GameTech/TutorialGame.h"

namespace NCL {
	namespace CSC8503 {
		class BonusObject : public GameObject
		{
		public:
			BonusObject(string name = "", TutorialGame * tutorialGame = nullptr): GameObject(name, tutorialGame){ 
				bonusValue = 25;
			}
			~BonusObject(){}

			void OnCollisionBegin(GameObject* otherObject) override {
				//std::cout << "OnCollisionBegin event occured!\n";
				if (otherObject->GetName() == "player" || otherObject->GetName() == "enemy") {
					renderObject->SetColour(Vector4(1, 0.5, 0, 1));
					game->GetWorld()->RemoveGameObject(this);
					//game->GetWorld()->RemoveGameObject(this, true);									
				}
			}

			int ReturnBonusValue() { return bonusValue; }

			

		protected:
			int bonusValue;
		};
	}
}