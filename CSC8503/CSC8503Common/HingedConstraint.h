#pragma once
#include "Constraint.h"
#include "../../Common/Quaternion.h"

namespace NCL {
	namespace CSC8503 {
		using namespace Maths;
		class GameObject;

		class HingedConstraint : public Constraint
		{
		public:
			HingedConstraint(GameObject* a, GameObject* b) {
				object1 = a;
				object2 = b;
			}

			void UpdateConstraint(float dt) override;

		protected:
			GameObject* object1;
			GameObject* object2;

			//Quaternion q_0;
		};

	}
}

