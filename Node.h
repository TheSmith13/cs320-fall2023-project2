#ifndef _NODE_H_
#define _NODE_H_

using namespace std;

#include <vector>
#include <cstddef>

class Node {
	public:
		Node(int number);
		
		Node();
		
		Node* createBST(vector<int> hotCold, int start, int end, Node* parent);
		
		int findVictim(Node* root);
		
		void updateTree(Node* victim);
		
		bool isLeaf(Node* node);
		
		int getData();
		
		void updateData(int newHotCold);
		
		Node* getLeftChild();
		
		Node* getRightChild();
		
		Node* getParent();
		
	private:
		int data;
		Node* rightChild;
		Node* leftChild;
		Node* parent;
};

#endif
