#pragma once
#include "NavigationMap.h"
#include <string>
namespace NCL {
	namespace CSC8503 {


		enum GRIDTYPE {
			Blank,
			Block,
		};

		struct GridNode {
			GridNode* parent;

			GridNode* connected[4];
			int		  costs[4];

			Vector3		position;

			float f;
			float g;

			int type;

			GRIDTYPE gridType;

			GridNode() {
				for (int i = 0; i < 4; ++i) {
					connected[i] = nullptr;
					costs[i] = 0;
				}
				f = 0;
				g = 0;
				type = 0;

				gridType = GRIDTYPE::Blank;

				parent = nullptr;
			}
			~GridNode() {	}
		};

		class NavigationGrid : public NavigationMap	{
		public:
			NavigationGrid();
			NavigationGrid(const std::string&filename);
			~NavigationGrid();

			bool FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) override;
				
			int GetWidth() { return gridWidth; }
			int GetHeight() { return gridHeight; }

			GridNode* GetNodes() { return allNodes; }

			int nodeSize;

			/**/
			bool ReachedWayPoint(Vector3 position, Vector3 wayPoint);

		protected:
			bool		NodeInList(GridNode* n, std::vector<GridNode*>& list) const;
			GridNode*	RemoveBestNode(std::vector<GridNode*>& list) const;
			float		Heuristic(GridNode* hNode, GridNode* endNode) const;

			int gridWidth;
			int gridHeight;

			GridNode* allNodes;

		};
	}
}

