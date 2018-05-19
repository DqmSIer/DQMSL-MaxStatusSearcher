#ifndef RTREE_CLASS
#define RTREE_CLASS

#include <cstring>
#include <vector>
#include <deque>
#include <algorithm>
#include <iterator>

#include "Mbr.hpp"

template <typename T, int D>
class Rtree {
private: /* define */
	static constexpr int MAX_ENTRY_NUMBER = 10;
	static constexpr int MIN_ENTRY_NUMBER = 3;

	struct Node {
	public:
		template <typename ObjectType, class LowerFunc, class UpperFunc, class = void>
		struct ChildAdder {
			static void add(Node *const node, ObjectType *const child, LowerFunc lowerFunc, UpperFunc upperFunc) {}
		};
		template <class LowerFunc, class UpperFunc>
		struct ChildAdder<T, LowerFunc, UpperFunc> {
			static void add(Node *const node, T *const child, LowerFunc lowerFunc, UpperFunc upperFunc) {
				node->mbr.extend(lowerFunc(*child), upperFunc(*child));
				node->child.push_back(reinterpret_cast<void *>(child));
			}
		};
		template <class LowerFunc, class UpperFunc>
		struct ChildAdder<Node, LowerFunc, UpperFunc> {
			static void add(Node *const node, Node *const child, LowerFunc lowerFunc, UpperFunc upperFunc) {
				node->mbr.extend(child->mbr.getLower(), child->mbr.getUpper());
				node->child.push_back(reinterpret_cast<void *>(child));
				child->parent = node;
			}
		};

	public: /* function */
		Node(void) {
			this->child.reserve(MAX_ENTRY_NUMBER);
			this->child.clear();
			this->mbr.clear();
			this->parent = nullptr;
		}
		~Node(void) = default;

		template <typename ObjectType, class LowerFunc, class UpperFunc>
		void addChild(ObjectType *const child, LowerFunc lowerFunc, UpperFunc upperFunc) {
			ChildAdder<ObjectType, LowerFunc, UpperFunc>::add(this, child, lowerFunc, upperFunc);
		}
		void updateMbr(void) {
			this->mbr.clear();
			for (void *const ptr: this->child) {
				Node *const child = static_cast<Node *>(ptr);
				this->mbr.extend(child->mbr.getLower(), child->mbr.getUpper());
			}
		}

	public: /* member */
		std::vector<void *> child;
		Mbr<D> mbr;
		Node *parent;
	};

public: /* function */
	Rtree(void) {
		this->objectPool.clear();
		this->nodePool.clear();
		this->nodePool.emplace_back();
		this->root = &this->nodePool.back();
		this->level = 0;
	}
	~Rtree(void) {
		this->level = -1;
		this->root = nullptr;
		this->nodePool.clear();
		this->nodePool.shrink_to_fit();
		this->objectPool.clear();
		this->objectPool.shrink_to_fit();
	}

	template <typename ObjectType, class LowerFunc, class UpperFunc>
	void pickSeeds(std::deque<ObjectType *> &remainSet, Node *const node, Node *const divNode, LowerFunc lowerFunc, UpperFunc upperFunc) {
		int maxDiff = -1;
		int minIdx[2];

		/* find maximum distance pair */
		for (int i = 0; i < remainSet.size() - 1; i++) {
			ObjectType *objAddr[2];
			Mbr<D> objMbr[2];

			objAddr[0] = remainSet[i];
			for (int j = i + 1; j < remainSet.size(); j++) {
				Mbr<D> mbr;
				int diff;

				objAddr[1] = remainSet[j];
				mbr.set(lowerFunc(*objAddr[0]), upperFunc(*objAddr[0]));
				mbr.extend(lowerFunc(*objAddr[1]), upperFunc(*objAddr[1]));

				objMbr[0].set(lowerFunc(*objAddr[0]), upperFunc(*objAddr[0]));
				objMbr[1].set(lowerFunc(*objAddr[1]), upperFunc(*objAddr[1]));

				diff = std::abs(mbr.getArea() - objMbr[0].getArea() - objMbr[1].getArea());
				if (maxDiff < diff) {
					maxDiff = diff;
					minIdx[0] = i;
					minIdx[1] = j;
				}
			}
		}

		/* form two nodes */
		node->addChild(remainSet[minIdx[0]], lowerFunc, upperFunc);
		divNode->addChild(remainSet[minIdx[1]], lowerFunc, upperFunc);
		/* remove from remaining set */
		remainSet.erase(remainSet.begin() + minIdx[1]);
		remainSet.erase(remainSet.begin() + minIdx[0]);
	}

	template <typename ObjectType, class LowerFunc, class UpperFunc>
	void pickNext(std::deque<ObjectType *> &remainSet, Node *const node, Node *const divNode, LowerFunc lowerFunc, UpperFunc upperFunc) {
		int minDiff = INT_MAX;
		Node *minNode;
		int minIdx;

		/* choose proper node to assign the remaining object */
		for (int i = 0; i < remainSet.size(); i++) {
			ObjectType *const objAddr = remainSet[i];
			Mbr<D> newMbr[2] = {node->mbr, divNode->mbr};
			int diff[2];

			newMbr[0].extend(lowerFunc(*objAddr), upperFunc(*objAddr));
			newMbr[1].extend(lowerFunc(*objAddr), upperFunc(*objAddr));
			diff[0] = std::abs(newMbr[0].getArea() - node->mbr.getArea());
			diff[1] = std::abs(newMbr[1].getArea() - divNode->mbr.getArea());

			if (minDiff > std::min(diff[0], diff[1])) {
				minIdx = i;
				minDiff = std::min(diff[0], diff[1]);

				if (diff[0] < diff[1]) {
					minNode = node;
				}
				else {
					if (diff[1] < diff[0]) {
						minNode = divNode;
					}
					/* tie */
					else {
						if (node->mbr.getArea() < divNode->mbr.getArea()) {
							minNode = node;
						}
						/* tie */
						else {
							minNode = (node->child.size() < divNode->child.size()) ? node : divNode;
						}
					}
				}
			}
		}

		minNode->addChild(remainSet[minIdx], lowerFunc, upperFunc);
		remainSet.erase(remainSet.begin() + minIdx);
	}

	template <typename ObjectType, class LowerFunc, class UpperFunc>
	Node *split(Node *const node, ObjectType *const object, LowerFunc lowerFunc, UpperFunc upperFunc) {
		std::deque<ObjectType *> remainSet;
		Node *divNode;

		/* allocate new split node */
		this->nodePool.emplace_back();
		divNode = &this->nodePool.back();

		remainSet.resize(MAX_ENTRY_NUMBER + 1);
		remainSet.clear();
		for (void *const ptr: node->child) {
			ObjectType *const obj = static_cast<ObjectType *>(ptr);
			remainSet.push_back(obj);
		}
		remainSet.push_back(object);

		node->mbr.clear();
		node->child.clear();
		divNode->mbr.clear();
		divNode->child.clear();

		/* divide objects into 2 groups */
		this->pickSeeds(remainSet, node, divNode, lowerFunc, upperFunc);
		while (remainSet.empty() == false) {
			if (node->child.size() == MAX_ENTRY_NUMBER - MIN_ENTRY_NUMBER + 1) {
				for (ObjectType *const obj: remainSet) {
					divNode->addChild(obj, lowerFunc, upperFunc);
				}
				break;
			}
			if (divNode->child.size() == MAX_ENTRY_NUMBER - MIN_ENTRY_NUMBER + 1) {
				for (ObjectType *const obj: remainSet) {
					node->addChild(obj, lowerFunc, upperFunc);
				}
				break;
			}

			this->pickNext(remainSet, node, divNode, lowerFunc, upperFunc);
		}

		return divNode;
	}

	/* Internal Function */
	template <class LowerFunc, class UpperFunc>
	Node *chooseLeaf(const int depth, Node *const node, const T &object, LowerFunc lowerFunc, UpperFunc upperFunc) {
		/* Leaf */
		if (depth == this->level) {
			return node;
		}
		else {
			int minArea = INT_MAX;
			int minExtend = INT_MAX;
			Node *minCostNode = nullptr;

			for (void *const ptr: node->child) {
				Node *const child = static_cast<Node *>(ptr);
				int oldArea, newArea, extend;
				Mbr<D> newMbr;

				oldArea = child->mbr.getArea();
				newMbr = child->mbr;
				newMbr.extend(lowerFunc(object), upperFunc(object));
				newArea = newMbr.getArea();

				extend = newArea - oldArea;
				if (extend < minExtend) {
					minExtend = extend;
					minArea = newArea;
					minCostNode = child;
				}
				else if (extend == minExtend) {
					if (newArea < minArea) {
						// minExtend = extend;
						minArea = newArea;
						minCostNode = child;
					}
				}
			}
			return this->chooseLeaf(depth + 1, minCostNode, object, lowerFunc, upperFunc);
		}
	}
	/* External Function */
	template <class LowerFunc, class UpperFunc>
	Node *chooseLeaf(const T &object, LowerFunc lowerFunc, UpperFunc upperFunc) {
		return this->chooseLeaf(0, this->root, object, lowerFunc, upperFunc);
	}

	template <class LowerFunc, class UpperFunc>
	void adjust(Node *const leaf, Node *const divLeaf, LowerFunc lowerFunc, UpperFunc upperFunc) {
		auto constexpr lowerGetter = [](const Node &node){ return node.mbr.getLower(); };
		auto constexpr upperGetter = [](const Node &node){ return node.mbr.getUpper(); };
		Node *node = leaf;
		Node *divNode = divLeaf;

		/* fix mbr of leaf to root */
		while (node != this->root) {
			Node *parent = node->parent;
			Node *divParent = nullptr;

			parent->updateMbr();
			if (divNode != nullptr) {
				Node *nodeAddr = &this->nodePool.back();

				if (parent->child.size() < MAX_ENTRY_NUMBER) {
					parent->addChild(divNode, lowerGetter, upperGetter);
				}
				else {
					divParent = this->split(parent, nodeAddr, lowerGetter, upperGetter);
				}
			}

			node = parent;
			divNode = divParent;
		}

		/* root is reached */
		if (divNode != nullptr) {
			this->nodePool.emplace_back();
			this->root = &this->nodePool.back();

			this->root->addChild(node, lowerGetter, upperGetter);
			this->root->addChild(divNode, lowerGetter, upperGetter);
			this->level++;
		}
	}


	/* Internal Function */
	template <class LowerFunc, class UpperFunc>
	void insert(const int depth, Node *const node, const T &object, LowerFunc lowerFunc, UpperFunc upperFunc) {
		Node *leaf = nullptr;
		Node *divLeaf = nullptr;
		T *objAddr;

		this->objectPool.push_back(object);
		objAddr = &this->objectPool.back();

		leaf = this->chooseLeaf(object, lowerFunc, upperFunc);

		if (leaf->child.size() < MAX_ENTRY_NUMBER) {
			leaf->addChild(objAddr, lowerFunc, upperFunc);
		}
		else {
			divLeaf = this->split(leaf, objAddr, lowerFunc, upperFunc);
		}

		this->adjust(leaf, divLeaf, lowerFunc, upperFunc);
	}
	/* External Function */
	template <class LowerFunc, class UpperFunc>
	void insert(const T &object, LowerFunc lowerFunc, UpperFunc upperFunc) {
		this->insert(0, this->root, object, lowerFunc, upperFunc);
	}

	/* Internal Function */
	template <class LowerFunc, class UpperFunc, class ScreenFunc>
	int update(const int depth, Node *const node, const T &object, LowerFunc lowerFunc, UpperFunc upperFunc, ScreenFunc screenFunc) {
		/* Leaf */
		if (depth == this->level) {
			for (void *const ptr: node->child) {
				T *const obj = static_cast<T *>(ptr);
				int result = screenFunc(*obj, object);

				switch (result) {
					case 1: /* update */
						*obj = object;
					case -1: /* skip */
						return result;
					default:
						break;
				}
			}
		}
		else {
			for (void *const ptr: node->child) {
				Node *const child = static_cast<Node *>(ptr);
				if (child->mbr.overlap(lowerFunc(object), upperFunc(object)) == true) {
					int result = this->update(depth + 1, child, object, lowerFunc, upperFunc, screenFunc);

					if (result != 0) {
						return result;
					}
				}
			}
		}
		return 0;
	}
	/* External Function */
	template <class LowerFunc, class UpperFunc, class ScreenFunc>
	int update(const T &object, LowerFunc lowerFunc, UpperFunc upperFunc, ScreenFunc screenFunc) {
		int result = this->update(0, this->root, object, lowerFunc, upperFunc, screenFunc);
		/* result: 0=insert, 1=update, -1=skip */
		if (result == 0) {
			this->insert(object, lowerFunc, upperFunc);
		}
		return result;
	}

	/* Internal Function */
	template <class LowerFunc, class UpperFunc, class ScreenFunc>
	void search(const int depth, Node *const node, std::deque<T> &resultPool, 
		    const T &searchObject, LowerFunc lowerFunc, UpperFunc upperFunc, ScreenFunc screenFunc) {
		/* Leaf */
		if (depth == this->level) {
			for (void *const ptr: node->child) {
				T *const obj = static_cast<T *>(ptr);
				if (screenFunc(*obj, lowerFunc, upperFunc) == true) {
					resultPool.push_back(obj);
				}
			}
		}
		else {
			for (void *const ptr: node->child) {
				Node *const child = static_cast<T *>(ptr);
				if (child->mbr.overlap(lowerFunc(searchObject), upperFunc(searchObject)) == true) {
					this->search(depth + 1, child, resultPool, searchObject, lowerFunc, upperFunc, screenFunc);
				}
			}

		}
	}
	/* External Function */
	template <class LowerFunc, class UpperFunc, class ScreenFunc>
	void search(std::deque<T> &resultPool, const T &searchObject, LowerFunc lowerFunc, UpperFunc upperFunc, ScreenFunc screenFunc) {
		this->search(0, this->root, resultPool, searchObject, lowerFunc, upperFunc, screenFunc);
	}

	std::deque<T> &referObjectPool(void) {
		return this->objectPool;
	}
	std::deque<Node> &referNodePool(void) {
		return this->nodePool;
	}

private: /* member */
	std::deque<T> objectPool;
	std::deque<Node> nodePool;
	Node *root;
	int level;
};

#endif //RTREE_CLASS
