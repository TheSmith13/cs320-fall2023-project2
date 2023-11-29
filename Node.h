


class Node {
	public:
		BSTNode(int number);
		
		BSTNode();
		
		Node* createBST(vector<int> hotCold, int start, int end);
		
		Node* findVictim(Node* root);
		
		int getData(Node* node);
		
	private:
		int data;
		BSTNode* rightChild;
		BSTNode* leftChild;
		BSTNode* parent;
}
