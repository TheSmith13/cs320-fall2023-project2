#include "Node.h"

Node::Node(int number) {
	data = number;
	rightChild = leftChild = parent = NULL;
}

Node::Node() {
	data = 0;
	rightChild = leftChild = parent = NULL;
}

Node* Node::createBST(vector<int> hotCold, int start, int end) {
	if (start > end) {
		return NULL;
	}
	
	int mid = (start + end) / 2;
	Node* root = new Node(hotCold[mid]);
	root->left = createBST(hotCold, start, mid - 1);
	root->right = createBST(hotCold, mid + 1, end);
	return root;
}
		
int Node::findVictim(Node* root) {
	if (root.isLeaf()) {
		if (root.getData == 0) {
			updateTree(root);
			return 
		}
		
		else if (root.getData == 1) {
			updateTree(root);
			return
		}
	}
	
	else {
		if (root.getData == 0) {
			findVictim(root.getRightChild);
		}
		
		else if (root.getData == 1) {
			findVictim(root.getLeftChild);
		}
	}
}

void Node::updateTree(Node* victim) {
	
}

bool Node::isLeaf(Node* node) {
	if ((node.getLeftChild == NULL) && (node.getRightChild == NULL)) {
		return true;
	}
	
	else {
		return false;
	}
}

int Node::getData(Node* node) {
	return data;
}

void Node::updataData(int newHotCold) {
	data = newHotCold;
}

Node* Node::getLeftChild(Node* node) {
	return leftChild;
}

Node* Node::getRightChild(Node* node) {
	return rightChild;
}
