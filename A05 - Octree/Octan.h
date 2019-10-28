#pragma once

#ifndef __MY_OCTAN_CLASS_H_
#define __MY_OCTAN_CLASS_H_

#include "MyEntityManager.h"

namespace Simplex
{
	class MyOctant
	{
		 // how many Octants made;
		static uint octantCount;

		// what is the max octant level can go to
		static uint maxLevel;

		// how many entities per octant is good to have
		static uint idealEntityCount; 

		uint ID = 0;
		uint level = 0;
		uint childrenCount = 0;
		
		//size of the octant
		float size = 0.f;
		
		// singleton to the mesh manager
		MeshManager* meshManager = nullptr;

		// singleton to the entity manager
		MyEntityManager* entityManager = nullptr;

		vector3 center = vector3(0);
		vector3 minLocation = vector3(0);
		vector3 maxLocation = vector3(0);

		MyOctant* parent = nullptr;

		// up to 8 children of the current octant
		MyOctant* child[8];

		// list of entities under the octant
		std::vector<uint> entityList;

		MyOctant* root = nullptr;

		 // list of nodes that contain objects (for root only)
		std::vector<MyOctant*> lChild;

		matrix4 transform;
		
	public:
		/**
		 * Constructor, will create an Octant containing all the Entity Instances that are within the Entity Manager
		 *
		 * @param maxLevel Sets the maximum level of subdivisions
		 * @param idealEntityCount Sets the ideal number of objects per octant
		 */
		MyOctant(uint maxLevel = 2, uint idealEntityCount = 5);

		/**
		 * Constructor, that will create the Octant based  on supplied center and size params
		 *
		 * @param center Sets the center of a particular Octant
		 * @param size Sets the bounded size of the Octant
		 */
		MyOctant(vector3 center, float size);

		/**
		 * Copy Constructor
		 */
		MyOctant(const MyOctant& other);

		/**
		 * Copy Assignment Operator
		 */
		MyOctant& operator= (const MyOctant& other);

		/**
		 * Destructor
		 */
		~MyOctant();

		/**
		 * Swap objects' contents with the other
		 *
		 * @param other the object that will swap data with the current one
		 */
		void Swap(MyOctant& other);

		/**
		 * Getter for Octant size
		 */
		float GetSize() const;

		/**
		 * Getter for the center of the octant in global space
		 */
		vector3 GetCenterGlobal() const;

		/**
		 * Getter for the Octant's Min in global space
		 */
		vector3 GetMinGlobal() const;

		/**
		 * Getter for the Octant's Max in global space
		 */
		vector3 GetMaxGlobal() const;

		/**
		 * Helper function to check if the Octant is colliding with the entity
		 */
		bool IsColliding(uint entityIndex) const;

		/**
		 * Display the Octant Volume specified by the index including the objects underneath
		 */
		void Display(uint index, vector3 color = C_YELLOW);

		/**
		 * Displays the Octant with the specified color
		 */
		void Display(vector3 color = C_YELLOW);

		/**
		 * Removes the entities for each node
		 */
		void ClearEntityList();

		/**
		 * Allocate 8 smaller Octants in the child pointer
		 */
		void Subdivide();

		/**
		 * Getter for the Octant child bound by the index
		 */
		MyOctant* GetChild(uint childIndex);

		/**
		 * Getter for the Parent Octant if any
		 */
		MyOctant* GetParent();

		/**
		 * Helper that determines if the Octant contains more than the specified number of entities
		 */
		bool ContainsMoreThan(uint entityCount);

		/**
		 * Deletes all the child nodes including their leaves
		 */
		void KillBranches();

		/**
		 * Creates tree using subdivisions according to the max number of levels
		 */
		void ConstructTree(uint maxLevel = 3);

		/**
		 * Helper that traverses the tree to the leafs and sets the entities inside to the appropriate index
		 */
		void AssignIDtoEntity();

		/**
		 * Gets the total number of Octants in the world
		 */
		uint GetOctantCount();

	private:
		/**
		 * Deallocates the member fields
		 */
		void Release();
		
		/**
		 * Allocates the member fields
		 */
		void Init();
		/*
		 * creates the list that contains all the objects
		 */
		void ConstructList();
	};
}

#endif
