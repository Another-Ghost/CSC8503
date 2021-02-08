#include "GJK.h"

#include "../../Common/Plane.h"
#include "../../Common/Maths.h"

#define GJK_MAX_NUM_ITERATIONS 64

//Point NCL::CalculateSearchPoint(const Vector3 & search_dir, GameObject* coll1, GameObject* coll2)
//{
//	Point point;
//	point.b = coll2->GetTransform().GetPosition();
//	point.a = coll1->GetBoundingVolume()->Support(-search_dir, coll1->GetTransform());
//	point.p = point.b - point.a;
//	return point;
//}


bool NCL::GJKCalculation(GameObject* coll1, GameObject* coll2, CollisionDetection::CollisionInfo& collisionInfo)
{
	collisionInfo.a = coll1;
	collisionInfo.b = coll2;

	Vector3* mtv;

	Vector3 coll1Pos = coll1->GetTransform().GetPosition();
	Vector3 coll2Pos = coll2->GetTransform().GetPosition();


	Point a, b, c, d; 
	Vector3 search_dir = coll1Pos - coll2Pos; 

	
	//c = coll2->GetBoundingVolume()->Support(search_dir, coll2->GetTransform()) - coll1->GetBoundingVolume()->Support(-search_dir, coll1->GetTransform());
	
	CalculateSearchPoint(c, search_dir, coll1, coll2);
	search_dir = -c.p; //search in direction of origin

	
	CalculateSearchPoint(b, search_dir, coll1, coll2);

	if (Vector3::Dot(b.p, search_dir) < 0) {
		return false;
	}

	search_dir = Vector3::Cross(Vector3::Cross(c.p - b.p, -b.p), c.p - b.p); 
	if (search_dir == Vector3(0, 0, 0)) { 
		
		search_dir = Vector3::Cross(c.p - b.p, Vector3(1, 0, 0)); 
		if (search_dir == Vector3(0, 0, 0))
			search_dir = Vector3::Cross(c.p - b.p, Vector3(0, 0, -1)); 
	}
	int simp_dim = 2; 

	for (int iterations = 0; iterations < GJK_MAX_NUM_ITERATIONS; iterations++)
	{
		//a.p = coll2->GetBoundingVolume()->Support(search_dir, coll2->GetTransform()) - coll1->GetBoundingVolume()->Support(-search_dir, coll1->GetTransform());
		//Point a;
		CalculateSearchPoint(a, search_dir, coll1, coll2);
		
		if (Vector3::Dot(a.p, search_dir) < 0) {
			return false;
		}

		simp_dim++;
		if (simp_dim == 3) {
			update_simplex3(a, b, c, d, simp_dim, search_dir);
		}
		else if (update_simplex4(a, b, c, d, simp_dim, search_dir)) {
			EPA(a, b, c, d, coll1, coll2, collisionInfo);
			return true;
		}
	}

	return false;
}

void NCL::update_simplex3(Point& a, Point& b, Point& c, Point& d, int& simp_dim, Vector3& search_dir)
{

	Vector3 n = Vector3::Cross(b.p - a.p, c.p - a.p); 
	Vector3 AO = -a.p; 



	simp_dim = 2;
	if (Vector3::Dot(Vector3::Cross(b.p - a.p, n), AO) > 0) { 
		c = a;
		
		search_dir = Vector3::Cross(Vector3::Cross(b.p - a.p, AO), b.p - a.p);
		return;
	}
	if (Vector3::Dot(Vector3::Cross(n, c.p - a.p), AO) > 0) { 
		b = a;
		
		search_dir = Vector3::Cross(Vector3::Cross(c.p - a.p, AO), c.p - a.p);
		return;
	}

	simp_dim = 3;
	if (Vector3::Dot(n, AO) > 0) { 
		d = c;
		c = b;
		b = a;
		
		search_dir = n;
		return;
	}

	d = b;
	b = a;

	search_dir = -n;
	return;
}

bool NCL::update_simplex4(Point& a, Point& b, Point& c, Point& d, int& simp_dim, Vector3& search_dir)
{

	Vector3 ABC = Vector3::Cross(b.p - a.p, c.p - a.p);
	Vector3 ACD = Vector3::Cross(c.p - a.p, d.p - a.p);
	Vector3 ADB = Vector3::Cross(d.p - a.p, b.p - a.p);

	Vector3 AO = -a.p; 
	simp_dim = 3; 


	if (Vector3::Dot(ABC, AO) > 0) { 
		d = c;
		c = b;
		b = a;
		search_dir = ABC;
		return false;
	}

	if (Vector3::Dot(ACD, AO) > 0) { 
		search_dir = ACD;
		return false;
	}
	if (Vector3::Dot(ADB, AO) > 0) { 
		c = d;
		d = b;
		b = a;
		search_dir = ADB;
		return false;
	}

	return true;
}


#define EPA_TOLERANCE 0.0001
#define EPA_MAX_NUM_FACES 64
#define EPA_MAX_NUM_LOOSE_EDGES 32
#define EPA_MAX_NUM_ITERATIONS 64
void NCL::EPA(Point& a, Point& b, Point& c, Point& d, GameObject* coll1, GameObject* coll2, CollisionDetection::CollisionInfo& collisionInfo)
{
	Point faces[EPA_MAX_NUM_FACES][4]; 

	Vector3 VertexA[3];
	Vector3 VertexB[3];

	faces[0][0] = a;
	faces[0][1] = b;
	faces[0][2] = c;
	faces[0][3].p = (Vector3::Cross(b.p - a.p, c.p - a.p)).Normalised(); 
	faces[1][0] = a;
	faces[1][1] = c;
	faces[1][2] = d;
	faces[1][3].p = (Vector3::Cross(c.p - a.p, d.p - a.p)).Normalised(); 
	faces[2][0] = a;
	faces[2][1] = d;
	faces[2][2] = b;
	faces[2][3].p = (Vector3::Cross(d.p - a.p, b.p - a.p)).Normalised(); 
	faces[3][0] = b;
	faces[3][1] = d;
	faces[3][2] = c;
	faces[3][3].p = (Vector3::Cross(d.p - b.p, c.p - b.p)).Normalised(); 

	int num_faces = 4;
	int closest_face;

	for (int iterations = 0; iterations < EPA_MAX_NUM_ITERATIONS; iterations++) {
		
		float min_dist = Vector3::Dot(faces[0][0].p, faces[0][3].p);
		closest_face = 0;
		for (int i = 1; i < num_faces; i++) {
			float dist = Vector3::Dot(faces[i][0].p, faces[i][3].p);
			if (dist < min_dist) {
				min_dist = dist;
				closest_face = i;
			}
		}


		Vector3 search_dir = faces[closest_face][3].p;

		//Vector3 p = coll2->GetBoundingVolume()->Support(search_dir, coll2->GetTransform()) - coll1->GetBoundingVolume()->Support(-search_dir, coll1->GetTransform());
		Point p;
		CalculateSearchPoint(p, search_dir, coll1, coll2);

		if (Vector3::Dot(p.p, search_dir) - min_dist < EPA_TOLERANCE) {
			
			Plane closestPlane = Plane::PlaneFromTri(faces[closest_face][0].p, faces[closest_face][1].p, faces[closest_face][2].p);
			Vector3 projectionPoint = closestPlane.ProjectPointOntoPlane(Vector3(0, 0, 0));
			float u, v, w;
			Barycentric(faces[closest_face][0].p, faces[closest_face][1].p, faces[closest_face][2].p,
				projectionPoint, u, v, w);
			Vector3 localA = faces[closest_face][0].a * u + faces[closest_face][1].a * v + faces[closest_face][2].a * w;
			Vector3 localB = faces[closest_face][0].b * u + faces[closest_face][1].b * v + faces[closest_face][2].b * w;
			float penetration = (localA - localB).Length();
			Vector3 normal = (localA - localB).Normalised();

			//Convergence (new point is not significantly further from origin)
			//float penetration = Vector3::Dot(p.p, search_dir);
			//Vector3 normal = -faces[closest_face][3].p;
			/*Vector3 localA = coll1->GetTransform().GetInvRotMatrix() * coll1->GetBoundingVolume()->Support(-search_dir, coll1->GetTransform());
			Vector3 localB = coll2->GetTransform().GetInvRotMatrix() * coll2->GetBoundingVolume()->Support(search_dir, coll2->GetTransform());*/

			localA -= coll1->GetTransform().GetPosition();
			localB -= coll2->GetTransform().GetPosition();

			collisionInfo.AddContactPoint(localA, localB, normal, penetration);

			return;
			//return faces[closest_face][3] * Vector3::Dot(p, search_dir); //dot vertex with normal to resolve collision along normal!
		}

		Point loose_edges[EPA_MAX_NUM_LOOSE_EDGES][2]; 
		int num_loose_edges = 0;


		for (int i = 0; i < num_faces; i++)
		{
			if (Vector3::Dot(faces[i][3].p, p.p - faces[i][0].p) > 0) 
			{
			
				for (int j = 0; j < 3; j++)
				{
					Point current_edge[2] = { faces[i][j], faces[i][(j + 1) % 3] };
					bool found_edge = false;
					for (int k = 0; k < num_loose_edges; k++) 
					{
						if (loose_edges[k][1].p == current_edge[0].p && loose_edges[k][0].p == current_edge[1].p) {
							
							loose_edges[k][0] = loose_edges[num_loose_edges - 1][0]; 
							loose_edges[k][1] = loose_edges[num_loose_edges - 1][1]; 
							num_loose_edges--;
							found_edge = true;
							k = num_loose_edges; 
						}
					}

					if (!found_edge) { 
						
						if (num_loose_edges >= EPA_MAX_NUM_LOOSE_EDGES) break;
						loose_edges[num_loose_edges][0] = current_edge[0];
						loose_edges[num_loose_edges][1] = current_edge[1];
						num_loose_edges++;
					}
				}

				
				faces[i][0] = faces[num_faces - 1][0];
				faces[i][1] = faces[num_faces - 1][1];
				faces[i][2] = faces[num_faces - 1][2];
				faces[i][3] = faces[num_faces - 1][3];
				num_faces--;
				i--;
			}
		}

		
		for (int i = 0; i < num_loose_edges; i++)
		{
			
			if (num_faces >= EPA_MAX_NUM_FACES) break;
			faces[num_faces][0] = loose_edges[i][0];
			faces[num_faces][1] = loose_edges[i][1];
			faces[num_faces][2] = p;
			faces[num_faces][3].p = Vector3::Cross(loose_edges[i][0].p - loose_edges[i][1].p, loose_edges[i][0].p - p.p).Normalised();

			
			float bias = 0.000001; 
			if (Vector3::Dot(faces[num_faces][0].p, faces[num_faces][3].p) + bias < 0) {
				Point temp = faces[num_faces][0];
				faces[num_faces][0] = faces[num_faces][1];
				faces[num_faces][1] = temp;
				faces[num_faces][3].p = -faces[num_faces][3].p;
			}
			num_faces++;
		}
	} 
	printf("EPA did not converge\n");
	
	Vector3 search_dir = faces[closest_face][3].p;

	Point p;
	CalculateSearchPoint(p, search_dir, coll1, coll2);
	//Vector3 p = coll2->GetBoundingVolume()->Support(search_dir, coll2->GetTransform()) - coll1->GetBoundingVolume()->Support(-search_dir, coll1->GetTransform());

	Plane closestPlane = Plane::PlaneFromTri(faces[closest_face][0].p, faces[closest_face][1].p, faces[closest_face][2].p);
	Vector3 projectionPoint = closestPlane.ProjectPointOntoPlane(Vector3(0, 0, 0));
	float u, v, w;
	Barycentric(faces[closest_face][0].p, faces[closest_face][1].p, faces[closest_face][2].p,
		projectionPoint, u, v, w);
	Vector3 localA = faces[closest_face][0].a * u + faces[closest_face][1].a * v + faces[closest_face][2].a * w;
	Vector3 localB = faces[closest_face][0].b * u + faces[closest_face][1].b * v + faces[closest_face][2].b * w;
	float penetration = (localA - localB).Length();
	Vector3 normal = (localA - localB).Normalised();

	//float penetration = Vector3::Dot(p.p, search_dir);
	//Vector3 normal = -faces[closest_face][3].p;

	//Vector3 localA = coll1->GetTransform().GetInvRotMatrix() * coll1->GetBoundingVolume()->Support(-search_dir, coll1->GetTransform());
	//Vector3 localB = coll2->GetTransform().GetInvRotMatrix() * coll2->GetBoundingVolume()->Support(search_dir, coll2->GetTransform());
	collisionInfo.AddContactPoint(localA, localB, normal, penetration);

	return;
	//return faces[closest_face][3] * Vector3::Dot(faces[closest_face][0], faces[closest_face][3]);
}

void NCL::CalculateSearchPoint(Point& point, Vector3& search_dir, GameObject* coll1, GameObject* coll2)
{
	point.b = coll2->GetBoundingVolume()->Support(search_dir, coll2->GetTransform());
	point.a = coll1->GetBoundingVolume()->Support(-search_dir, coll1->GetTransform());
	point.p = point.b - point.a;
}
