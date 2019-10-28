#include "Octan.h"
#include <stack>
using namespace Simplex;


uint Simplex::MyOctant::octantCount = 0;
uint Simplex::MyOctant::maxLevel = 3;
uint Simplex::MyOctant::idealEntityCount = 5;

Simplex::MyOctant::MyOctant(uint maxLvl, uint idealEntityC)
{
	Init();
	
	octantCount = 0;
	maxLevel = maxLvl;
	idealEntityCount = idealEntityC;
	ID = octantCount;

	root = this;
	lChild.clear();

	std::vector<vector3> minMax;
	const uint numberOfObjects = entityManager->GetEntityCount();
	for (uint i = 0; i < numberOfObjects; i++)
	{
		const auto entity = entityManager->GetEntity(i);
		const auto rigidBody = entity->GetRigidBody();

		minMax.push_back(rigidBody->GetMinGlobal());
		minMax.push_back(rigidBody->GetMaxGlobal());
	}
	
	// hack to get the correct dimensions for the big boi Octan (first ever) without having to find the min and max myself
	auto rb = new MyRigidBody(minMax);
	
	vector3 halfWidth = rb->GetHalfWidth();

	float max = halfWidth.x;
	for (uint i = 1; i < 3; i++)
	{
		if(max < halfWidth[i])
		{
			max = halfWidth[i];
		}
	}
	vector3 rbCenter = rb->GetCenterLocal();

	// invalidate the rb. it's unneeded now
	SafeDelete(rb);
	rb = nullptr;
	
	size = max * 2.f;
	center = rbCenter;

	minLocation = center - vector3(max);
	maxLocation = center + vector3(max);

	transform = (glm::translate(IDENTITY_M4, center) * glm::scale(vector3(size)));
	octantCount++;

	ConstructTree(maxLevel);
}

Simplex::MyOctant::MyOctant(vector3 centerV, float sizeF)
{
	Init();
	center = centerV;
	size = sizeF;

	minLocation = center - (vector3(size) / 2.f);
	maxLocation = center + (vector3(size) / 2.f);

	octantCount++;
}

Simplex::MyOctant::MyOctant(const MyOctant& other)
{
	childrenCount = other.childrenCount;
	center = other.center;
	minLocation = other.minLocation;
	maxLocation = other.maxLocation;
	size = other.size;
	ID = other.ID;
	level = other.level;
	parent = other.parent;
	root = other.root;
	lChild = other.lChild;
	meshManager = MeshManager::GetInstance();
	entityManager = MyEntityManager::GetInstance();

	for (uint i = 0; i < 8; i++)
	{
		child[i] = other.child[i];
	}
}

Simplex::MyOctant& Simplex::MyOctant::operator=(const MyOctant& other)
{
	if(this != &other)
	{
		Release();
		Init();
		MyOctant temp(other);
		Swap(temp);
	}

	return *this;
}

Simplex::MyOctant::~MyOctant()
{
	Release();
}

void Simplex::MyOctant::Swap(MyOctant& other)
{
	std::swap(childrenCount, other.childrenCount);
	std::swap(size, other.size);
	std::swap(ID, other.ID);
	std::swap(root, other.root);
	std::swap(lChild, other.lChild);
	std::swap(center, other.center);
	std::swap(minLocation, other.minLocation);
	std::swap(maxLocation, other.maxLocation);

	meshManager = MeshManager::GetInstance();
	entityManager = MyEntityManager::GetInstance();

	std::swap(level, other.level);
	std::swap(parent, other.parent);

	for (uint i = 0; i < 8; i++)
	{
		std::swap(child[i], other.child[i]);
	}
}

float Simplex::MyOctant::GetSize() const
{
	return size;
}

vector3 Simplex::MyOctant::GetCenterGlobal() const
{
	return center;
}

vector3 Simplex::MyOctant::GetMinGlobal() const
{
	return minLocation;
}

vector3 Simplex::MyOctant::GetMaxGlobal() const
{
	return maxLocation;
}

bool Simplex::MyOctant::IsColliding(uint entityIndex) const
{
	const uint entityCount = entityManager->GetEntityCount();
	if(entityIndex > entityCount)
	{
		return false;
	}
	
	// JUST DO AABB
	const auto rb = entityManager->GetEntity(entityIndex)->GetRigidBody();
	
	const vector3 min = rb->GetMinGlobal();
	const vector3 max = rb->GetMaxGlobal();

	if(maxLocation.x < min.x)
	{
		return false;
	}
	if(minLocation.x > max.x)
	{
		return false;
	}

	if(maxLocation.y < min.y)
	{
		return false;
	}
	if(minLocation.y > max.y)
	{
		return false;
	}

	if(maxLocation.z < min.z)
	{
		return false;
	}
	if(minLocation.z > max.z)
	{
		return false;
	}
	
	return true;
}

void Simplex::MyOctant::Display(uint index, vector3 color)
{
	// no recursion for more optimization
	std::stack<MyOctant*> octStack;
	octStack.push(this);
	while(!octStack.empty())
	{
		const auto node = octStack.top();
		if(node->ID == index)
		{
			meshManager->AddWireCubeToRenderList(node->transform, color, RENDER_WIRE);
		}
		octStack.pop();
		for (uint i = 0; i < node->childrenCount; i++)
		{
			octStack.push(node->child[i]);
		}
	}
}

void Simplex::MyOctant::Display(vector3 color)
{
	std::stack<MyOctant*> octStack;
	if(!child[0])
		return;
	octStack.push(child[0]);
	octStack.push(child[1]);
	octStack.push(child[2]);
	octStack.push(child[3]);
	octStack.push(child[4]);
	octStack.push(child[5]);
	octStack.push(child[6]);
	octStack.push(child[7]);
	
	while(!octStack.empty())
	{
		const auto node = octStack.top();

		meshManager->AddWireCubeToRenderList(node->transform, color, RENDER_WIRE);
		
		octStack.pop();
		for (uint i = 0; i < node->childrenCount; i++)
		{
			octStack.push(node->child[i]);
		}
	}
}

void Simplex::MyOctant::ClearEntityList()
{
	for (uint i = 0; i < childrenCount; i++)
	{
		child[i]->ClearEntityList();
	}
	entityList.clear();
}

void Simplex::MyOctant::Subdivide()
{
	if(level >= maxLevel)
		return;

	if(childrenCount != 0)
		return;

	childrenCount = 8;

	const float subdivSize = size / 4.f;
	const float childSize = subdivSize * 2.f;
	
	vector3 childCenter = center - vector3(subdivSize);

	// Bottom Back Left 
	child[0] = new MyOctant(childCenter, childSize);
	child[0]->transform = (glm::translate(IDENTITY_M4, childCenter) * glm::scale(vector3(childSize)));
	
	// Bottom Back Right
	childCenter.x += childSize;
	child[1] = new MyOctant(childCenter, childSize);
	child[1]->transform = (glm::translate(IDENTITY_M4, childCenter) * glm::scale(vector3(childSize)));
	
	// Bottom Front Right
	childCenter.z += childSize;
	child[2] = new MyOctant(childCenter, childSize);
	child[2]->transform = (glm::translate(IDENTITY_M4, childCenter) * glm::scale(vector3(childSize)));
	
	// Bottom Front Left
	childCenter.x -= childSize;
	child[3] = new MyOctant(childCenter, childSize);
	child[3]->transform = (glm::translate(IDENTITY_M4, childCenter) * glm::scale(vector3(childSize)));
	
	// Top Front Left
	childCenter.y += childSize;
	child[4] = new MyOctant(childCenter, childSize);
	child[4]->transform = (glm::translate(IDENTITY_M4, childCenter) * glm::scale(vector3(childSize)));
	
	// Top Front Right
	childCenter.z -= childSize;
	child[5] = new MyOctant(childCenter, childSize);
	child[5]->transform = (glm::translate(IDENTITY_M4, childCenter) * glm::scale(vector3(childSize)));
	
	// Top Back Right
	childCenter.x += childSize;
	child[6] = new MyOctant(childCenter, childSize);
	child[6]->transform  = (glm::translate(IDENTITY_M4, childCenter) * glm::scale(vector3(childSize)));
	
	// Top Back Left
	childCenter.z += childSize;
	child[7] = new MyOctant(childCenter, childSize);
	child[7]->transform = (glm::translate(IDENTITY_M4, childCenter) * glm::scale(vector3(childSize)));
	
	for (auto& mem : child)
	{
		mem->root = root;
		mem->parent = this;
		mem->level = level + 1;
		if(mem->ContainsMoreThan(idealEntityCount))
		{
			mem->Subdivide();
		}
	}
}

MyOctant* Simplex::MyOctant::GetChild(uint childIndex)
{
	return childIndex > 7 ? nullptr : this->child[childIndex]; 
}

MyOctant* Simplex::MyOctant::GetParent()
{
	return parent;
}

bool Simplex::MyOctant::ContainsMoreThan(uint entityCount)
{
	uint count = 0;
	const uint localEntityCount = entityManager->GetEntityCount();
	for (uint i = 0; i < localEntityCount; i++)
	{
		if(IsColliding(i))
		{
			++count;
			if(count > entityCount)
			{
				return true;
			};
		}
	}
	return false;
}

void Simplex::MyOctant::KillBranches()
{
	for (uint i = 0; i < childrenCount; i++)
	{
		child[i]->KillBranches();
		delete child[i];
		child[i] = nullptr;
	}
	childrenCount = 0;
}

void Simplex::MyOctant::ConstructTree(uint maxLvl)
{
	if(level != 0)
	{
		return;
	}

	entityList.clear();
	lChild.clear();
	
	maxLevel = maxLvl;
	octantCount = 1;

	KillBranches();
	
	if(ContainsMoreThan(idealEntityCount))
	{
		Subdivide();
	}
	entityManager->ClearDimensionSetAll();
	AssignIDtoEntity();
	ConstructList();
}

void Simplex::MyOctant::AssignIDtoEntity()
{
	for (uint i = 0; i < childrenCount; i++)
	{
		child[i]->AssignIDtoEntity();
	}
	if(childrenCount == 0)
	{
		const uint entityCount = entityManager->GetEntityCount();
		for (uint i = 0; i < entityCount; i++)
		{
			if(IsColliding(i))
			{
				entityList.push_back(i);
				entityManager->AddDimension(i, ID);
			}
		}
	}
}

uint Simplex::MyOctant::GetOctantCount()
{
	return octantCount;
}

void Simplex::MyOctant::Release()
{
	if(level == 0)
	{
		KillBranches();
	}
	childrenCount = 0;
	size = 0.f;
	entityList.clear();
	lChild.clear();
}

void Simplex::MyOctant::Init()
{
	meshManager = MeshManager::GetInstance();
	entityManager = MyEntityManager::GetInstance();
	
	childrenCount = 0;
	size = 0.f;
	ID = octantCount;
	level = 0;

	center = vector3(0.f);
	minLocation = vector3(0.f);
	maxLocation = vector3(0.f);

	root = nullptr;
	parent = nullptr;

	for (auto ch : child)
	{
		ch = nullptr;
	}
}

void Simplex::MyOctant::ConstructList()
{
	for (uint i = 0; i < childrenCount; i++)
	{
		child[i]->ConstructList();
	}

	if(!entityList.empty())
	{
		root->lChild.push_back(this);
	}
}
