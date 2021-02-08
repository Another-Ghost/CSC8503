#pragma once

#include "GameObject.h"
#include "CollisionDetection.h"

namespace NCL {
	struct Point {
		Vector3 p;
		Vector3 a;
		Vector3 b;
	};

	using namespace NCL::Maths;
	using namespace NCL::CSC8503;


	bool GJKCalculation(GameObject* coll1, GameObject* coll2, CollisionDetection::CollisionInfo& collisionInfo);


	void update_simplex3(Point& a, Point& b, Point& c, Point& d, int& simp_dim, Vector3& search_dir);
	bool update_simplex4(Point& a, Point& b, Point& c, Point& d, int& simp_dim, Vector3& search_dir);


	void EPA(Point& a, Point& b, Point& c, Point& d, GameObject* coll1, GameObject* coll2, CollisionDetection::CollisionInfo& collisionInfo);

	void CalculateSearchPoint(Point& point, Vector3& search_dir, GameObject* coll1, GameObject* coll2);
}