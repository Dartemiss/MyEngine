#include "AABBTree.h"
#include <assert.h>
#include <stack>
#include "debugdraw.h"


AABBTree::AABBTree(unsigned initialSize)
{
	nodeCapacity = initialSize;
	//Resize
	nodes.resize(initialSize);
	for (unsigned nodeIndex = 0; nodeIndex < initialSize; nodeIndex++)
	{
		NodeAABB &node = nodes[nodeIndex];
		node.nextNodeIndex = nodeIndex + 1;
	}
	nodes[initialSize - 1].nextNodeIndex = AABB_NULL_NODE;
}

AABBTree::~AABBTree()
{
	nodes.clear();
	objectNodeIndexMap.clear();
}

unsigned AABBTree::AllocateNode()
{
	// if we have no free tree nodes then grow the pool
	if (nextFreeNodeIndex == AABB_NULL_NODE)
	{
		assert(allocatedNodeCount == nodeCapacity);

		nodeCapacity += growthSize;
		nodes.resize(nodeCapacity);
		nodes[allocatedNodeCount - 1].nextNodeIndex = allocatedNodeCount;
		for (unsigned nodeIndex = allocatedNodeCount; nodeIndex < nodeCapacity; nodeIndex++)
		{
			NodeAABB& node = nodes[nodeIndex];
			node.nextNodeIndex = nodeIndex + 1;
		}
		nodes[nodeCapacity - 1].nextNodeIndex = AABB_NULL_NODE;
		nextFreeNodeIndex = allocatedNodeCount;
	}

	unsigned nodeIndex = nextFreeNodeIndex;
	NodeAABB& allocatedNode = nodes[nodeIndex];
	nextFreeNodeIndex = allocatedNode.nextNodeIndex;
	allocatedNodeCount++;

	return nodeIndex;

}

void AABBTree::DeallocateNode(unsigned nodeIndex)
{
	NodeAABB& deallocateNode = nodes[nodeIndex];
	deallocateNode.nextNodeIndex = nextFreeNodeIndex;
	nextFreeNodeIndex = nodeIndex;
	allocatedNodeCount--;

	return;
}

void AABBTree::Insert(GameObject * go)
{
	assert(go != nullptr);

	unsigned nodeIndex = AllocateNode();
	NodeAABB& node = nodes[nodeIndex];

	//AABB a bit bigger for avoiding little movements
	AABB auxAABB = AABB(go->globalBoundingBox->minPoint - float3(2,2,2), go->globalBoundingBox->maxPoint + float3(2,2,2));
	node.aabb = auxAABB;
	node.go = go;

	InsertLeaf(nodeIndex);
	objectNodeIndexMap[go] = nodeIndex;

	return;
}

void AABBTree::InsertLeaf(unsigned leafNodeIndex)
{
	// make sure we're inserting a new leaf
	assert(nodes[leafNodeIndex].parentNodeIndex == AABB_NULL_NODE);
	assert(nodes[leafNodeIndex].leftNodeIndex == AABB_NULL_NODE);
	assert(nodes[leafNodeIndex].rightNodeIndex == AABB_NULL_NODE);

	
	//if the tree is empty then we make the root the leaf
	if (rootNodeIndex == AABB_NULL_NODE)
	{
		rootNodeIndex = leafNodeIndex;
		return;
	}


	// search for the best place to put the new leaf in the tree
	// we use surface area and depth as search heuristics
	unsigned treeNodeIndex = rootNodeIndex;
	NodeAABB& leafNode = nodes[leafNodeIndex];
	while (!nodes[treeNodeIndex].isLeaf())
	{
		// because of the test in the while loop above we know we are never a leaf inside it
		const NodeAABB& treeNode = nodes[treeNodeIndex];
		unsigned leftNodeIndex = treeNode.leftNodeIndex;
		unsigned rightNodeIndex = treeNode.rightNodeIndex;
		const NodeAABB& leftNode = nodes[leftNodeIndex];
		const NodeAABB& rightNode = nodes[rightNodeIndex];
		
		AABB combinedAabb = MergeAABB(treeNode.aabb,leafNode.aabb);

		float newParentNodeCost = 2.0f * combinedAabb.SurfaceArea();
		float minimumPushDownCost = 2.0f * (combinedAabb.SurfaceArea() - treeNode.aabb.SurfaceArea());

		// use the costs to figure out whether to create a new parent here or descend
		float costLeft;
		float costRight;
		if (leftNode.isLeaf())
		{
			costLeft = MergeAABB(leftNode.aabb, leafNode.aabb).SurfaceArea() + minimumPushDownCost;
		}
		else
		{
			AABB newLeftAabb = MergeAABB(leftNode.aabb, leafNode.aabb);
			costLeft = (newLeftAabb.SurfaceArea() - leftNode.aabb.SurfaceArea()) + minimumPushDownCost;
		}
		if (rightNode.isLeaf())
		{
			costRight = MergeAABB(rightNode.aabb, leafNode.aabb).SurfaceArea() + minimumPushDownCost;
		}
		else
		{
			AABB newRightAabb = MergeAABB(rightNode.aabb, leafNode.aabb);
			costRight = (newRightAabb.SurfaceArea() - rightNode.aabb.SurfaceArea()) + minimumPushDownCost;
		}

		// if the cost of creating a new parent node here is less than descending in either direction then
		// we know we need to create a new parent node, errrr, here and attach the leaf to that
		if (newParentNodeCost < costLeft && newParentNodeCost < costRight)
		{
			break;
		}

		// otherwise descend in the cheapest direction
		if (costLeft < costRight)
		{
			treeNodeIndex = leftNodeIndex;
		}
		else
		{
			treeNodeIndex = rightNodeIndex;
		}
	}

	// the leafs sibling is going to be the node we found above and we are going to create a new
	// parent node and attach the leaf and this item

	unsigned newParentIndex = AllocateNode();
	NodeAABB& newParent = nodes[newParentIndex];
	unsigned leafSiblingIndex = treeNodeIndex;
	NodeAABB& leafSibling = nodes[leafSiblingIndex];
	unsigned oldParentIndex = leafSibling.parentNodeIndex;
	newParent.parentNodeIndex = oldParentIndex;
	newParent.aabb = MergeAABB(leafNode.aabb, leafSibling.aabb); // the new parents aabb is the leaf aabb combined with it's siblings aabb
	newParent.leftNodeIndex = leafSiblingIndex;
	newParent.rightNodeIndex = leafNodeIndex;
	nodes[leafNodeIndex].parentNodeIndex = newParentIndex;
	//TODO: this was the source of the error, leafNode didnt change nodes[X] if x is 9, wonder why
	//leafNode.parentNodeIndex = newParentIndex;
	leafSibling.parentNodeIndex = newParentIndex;

	if (oldParentIndex == AABB_NULL_NODE)
	{
		// the old parent was the root and so this is now the root
		rootNodeIndex = newParentIndex;
	}
	else
	{
		// the old parent was not the root and so we need to patch the left or right index to
		// point to the new node
		NodeAABB& oldParent = nodes[oldParentIndex];
		if (oldParent.leftNodeIndex == leafSiblingIndex)
		{
			oldParent.leftNodeIndex = newParentIndex;
		}
		else
		{
			oldParent.rightNodeIndex = newParentIndex;
		}
	}

	// finally we need to walk back up the tree fixing heights and areas
	//treeNodeIndex = leafNode.parentNodeIndex;
	treeNodeIndex = nodes[leafNodeIndex].parentNodeIndex;
	FixUpwardsTree(treeNodeIndex);

	return;
}

void AABBTree::FixUpwardsTree(unsigned treeNodeIndex)
{
	while (treeNodeIndex != AABB_NULL_NODE)
	{
		NodeAABB& treeNode = nodes[treeNodeIndex];

		// every node should be a parent
		assert(treeNode.leftNodeIndex != AABB_NULL_NODE && treeNode.rightNodeIndex != AABB_NULL_NODE);

		// fix height and area
		const NodeAABB& leftNode = nodes[treeNode.leftNodeIndex];
		const NodeAABB& rightNode = nodes[treeNode.rightNodeIndex];
		treeNode.aabb = MergeAABB(rightNode.aabb, leftNode.aabb);

		treeNodeIndex = treeNode.parentNodeIndex;


		if (treeNode.parentNodeIndex == treeNode.leftNodeIndex || treeNode.parentNodeIndex == treeNode.rightNodeIndex || treeNode.leftNodeIndex == treeNode.rightNodeIndex)
		{
			return;
		}
	}

	return;

}

void AABBTree::Remove(GameObject* go)
{
	unsigned nodeIndex = objectNodeIndexMap[go];
	RemoveLeaf(nodeIndex);
	DeallocateNode(nodeIndex);
	objectNodeIndexMap.erase(go);

	return;
}

void AABBTree::RemoveLeaf(unsigned leafNodeIndex)
{
	// if the leaf is the root then we can just clear the root pointer and return
	if (leafNodeIndex == rootNodeIndex)
	{
		rootNodeIndex = AABB_NULL_NODE;
		return;
	}

	NodeAABB& leafNode = nodes[leafNodeIndex];
	unsigned parentNodeIndex = leafNode.parentNodeIndex;
	const NodeAABB& parentNode = nodes[parentNodeIndex];
	unsigned grandParentNodeIndex = parentNode.parentNodeIndex;
	unsigned siblingNodeIndex = parentNode.leftNodeIndex == leafNodeIndex ? parentNode.rightNodeIndex : parentNode.leftNodeIndex;
	assert(siblingNodeIndex != AABB_NULL_NODE); // we must have a sibling
	NodeAABB& siblingNode = nodes[siblingNodeIndex];

	if (grandParentNodeIndex != AABB_NULL_NODE)
	{
		// if we have a grand parent (i.e. the parent is not the root) then destroy the parent and connect the sibling to the grandparent in its
		// place
		NodeAABB& grandParentNode = nodes[grandParentNodeIndex];
		if (grandParentNode.leftNodeIndex == parentNodeIndex)
		{
			grandParentNode.leftNodeIndex = siblingNodeIndex;
		}
		else
		{
			grandParentNode.rightNodeIndex = siblingNodeIndex;
		}
		siblingNode.parentNodeIndex = grandParentNodeIndex;
		DeallocateNode(parentNodeIndex);

		FixUpwardsTree(grandParentNodeIndex);
	}
	else
	{
		// if we have no grandparent then the parent is the root and so our sibling becomes the root and has it's parent removed
		rootNodeIndex = siblingNodeIndex;
		siblingNode.parentNodeIndex = AABB_NULL_NODE;
		DeallocateNode(parentNodeIndex);
	}

	leafNode.parentNodeIndex = AABB_NULL_NODE;

	return;
}

void AABBTree::UpdateObject(GameObject * go)
{
	unsigned nodeIndex = objectNodeIndexMap[go];
	UpdateLeaf(nodeIndex, *go->globalBoundingBox);

	return;
}

void AABBTree::GetIntersection(std::set<GameObject*>& intersectionGO, AABB * bbox)
{
	//DFS simulating recursivity using stack

	std::stack<unsigned> stack;
	stack.push(rootNodeIndex);
	while(!stack.empty())
	{
		unsigned nodeIndex = stack.top();
		stack.pop();
		
		if(nodeIndex == AABB_NULL_NODE)
			continue;

		const NodeAABB& node = nodes[nodeIndex];
		if(bbox->Intersects(node.aabb))
		{
			if(node.isLeaf())
			{
				intersectionGO.insert(node.go);
			}
			else
			{
				stack.push(node.leftNodeIndex);
				stack.push(node.rightNodeIndex);
			}
		}
	}

	return;
}

void AABBTree::GetIntersection(std::set<GameObject*>& intersectionGO, const LineSegment * ray)
{
	//DFS simulating recursivity using stack

	std::stack<unsigned> stack;
	stack.push(rootNodeIndex);
	while (!stack.empty())
	{
		unsigned nodeIndex = stack.top();
		stack.pop();

		if (nodeIndex == AABB_NULL_NODE)
			continue;

		const NodeAABB& node = nodes[nodeIndex];
		if (ray->Intersects(node.aabb))
		{
			if (node.isLeaf())
			{
				intersectionGO.insert(node.go);
			}
			else
			{
				stack.push(node.leftNodeIndex);
				stack.push(node.rightNodeIndex);
			}
		}
	}

	return;
}



void AABBTree::Draw() const
{

	if (rootNodeIndex == AABB_NULL_NODE)
		return;

	std::stack<NodeAABB> nodeStack;
	nodeStack.push(nodes[rootNodeIndex]);
	while (!nodeStack.empty())
	{
		NodeAABB node = nodeStack.top();
		nodeStack.pop();

		if (node.parentNodeIndex != AABB_NULL_NODE)
		{
			NodeAABB parent = nodes[node.parentNodeIndex];
			dd::line(node.aabb.CenterPoint(), parent.aabb.CenterPoint(), float3(1.0f, 0.0f, 0.0f));
		}
		if (node.leftNodeIndex != AABB_NULL_NODE)
			nodeStack.push(nodes[node.leftNodeIndex]);
		if (node.rightNodeIndex != AABB_NULL_NODE)
			nodeStack.push(nodes[node.rightNodeIndex]);

		dd::aabb(node.aabb.minPoint, node.aabb.maxPoint, float3(1.0f, 0.0f, 0.0f));

	}
	
	return;
}

void AABBTree::UpdateLeaf(unsigned leafNodeIndex, const AABB & newAaab)
{
	NodeAABB& node = nodes[leafNodeIndex];

	// if the node contains the new aabb then we just leave things
	if (node.aabb.Contains(newAaab)) return;

	RemoveLeaf(leafNodeIndex);
	//AABB a bit bigger for avoiding little movements
	node.aabb = AABB(newAaab.minPoint - float3(2, 2, 2), newAaab.maxPoint + float3(2, 2, 2));
	InsertLeaf(leafNodeIndex);
	
	return;
}

bool AABBTree::ValidNodeLeaf(unsigned leafNodeIndex)
{
	NodeAABB& leafNode = nodes[leafNodeIndex];
	//assert(leafNode.isLeaf());

	std::stack<NodeAABB> nodeStack;
	nodeStack.push(nodes[rootNodeIndex]);
	int counter = 0;
	std::stack<unsigned>nodeIndexes; 
	nodeIndexes.push(rootNodeIndex);
	while (!nodeStack.empty())
	{
		NodeAABB node = nodeStack.top();
		nodeStack.pop();



		if (node.leftNodeIndex == leafNodeIndex)
		{
			++counter;
			if (leafNode.parentNodeIndex != nodeIndexes.top())
				return false;

		}

		if (node.rightNodeIndex == leafNodeIndex)
		{
			++counter;
			if (leafNode.parentNodeIndex != nodeIndexes.top())
				return false;
		}

		nodeIndexes.pop();

		if (node.leftNodeIndex != AABB_NULL_NODE)
		{
			nodeStack.push(nodes[node.leftNodeIndex]);
			nodeIndexes.push(node.leftNodeIndex);
		}
		if (node.rightNodeIndex != AABB_NULL_NODE)
		{
			nodeStack.push(nodes[node.rightNodeIndex]);
			nodeIndexes.push(node.rightNodeIndex);
		}


	}

	if (counter > 1)
		return false;

	return true;
}

AABB AABBTree::MergeAABB(const AABB &first, const AABB &second) const
{
	return AABB(float3(Min(first.minPoint, second.minPoint)), float3(Max(first.maxPoint, second.maxPoint)));
}


