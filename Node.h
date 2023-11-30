#ifndef _NODE_H_
#define _NODE_H_

#include <vector>

class Node {
	public:
		Node(int number);
		
		Node();
		
		Node* createBST(vector<int> hotCold, int start, int end);
		
		int findVictim(Node* root);
		
		void updateTree(Node* victim);
		
		bool isLeaf(Node* node);
		
		int getData(Node* node);
		
		void updataData(int newHotCold);
		
		Node* getLeftChild(Node* node);
		
		Node* getRightChild(Node* node);
		
	private:
		int data;
		Node* rightChild;
		Node* leftChild;
		Node* parent;
};

#endif
