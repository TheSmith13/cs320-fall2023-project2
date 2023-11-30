#include "Node.h"

Node::Node(int number) {
	data = number;
	rightChild = leftChild = parent = NULL;
}

Node::Node() {
	data = 0;
	rightChild = leftChild = parent = NULL;
}

int Node::getData() {
	return data;
}

void Node::updataData(int newHotCold) {
	data = newHotCold;
}

Node* Node::getLeftChild() {
	return leftChild;
}

Node* Node::getRightChild() {
	return rightChild;
}

Node* Node::getParent() {
	return parent;
}

bool Node::isLeaf(Node* node) {
	if ((node->getLeftChild() == NULL) && (node->getRightChild() == NULL)) {
		return true;
	}
	
	else {
		return false;
	}
}

Node* Node::createBST(vector<int> hotCold, int start, int end) {
	if (start > end) {
		return NULL;
	}
	
	int mid = (start + end) / 2;
	Node* root = new Node(hotCold[mid]);
	root->leftChild = createBST(hotCold, start, mid - 1);
	root->rightChild = createBST(hotCold, mid + 1, end);
	return root;
}

void Node::updateTree(Node* victim) {
	
}
	
int Node::findVictim(Node* root) {
	if (root->isLeaf(root)) {
		if (root->getData() == 0) {
			updateTree(root);
			return 0;
		}
		
		else if (root->getData() == 1) {
			updateTree(root);
			return 1;
		}
	}
	
	else {
		if (root->getData() == 0) {
			findVictim(root->getRightChild());
		}
		
		else if (root->getData() == 1) {
			findVictim(root->getLeftChild());
		}
	}
}
