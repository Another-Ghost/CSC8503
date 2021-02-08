#pragma once
#include "Constraint.h"


namespace NCL {
	namespace CSC8503 {
		class GameObject;

		class OrientationConstraint : public Constraint
		{
		public:
			OrientationConstraint(GameObject* a, GameObject* b) {
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