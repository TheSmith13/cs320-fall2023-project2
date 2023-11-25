#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

struct memInstruct {
	char op;
	unsigned long address;
};

struct cacheLine {
	bool valid;
	unsigned long tag;
};

void simDirectMapCache() {

}

void simSetAssocCache() {

}

void simFullAssocCache() {

}

void simSetAssocNoAllocCache() {

}

void simSetAssocNextLinePreFetchCache() {

}

void simSetAssociativeOnMissPreFetchCache() {

}

int main() {
	ifstream inputFile("input_trace.txt");
	if (!inputFile.is_open()) {
		cout << "Error: unable to open input file." << endl;
		return 1;
	}
	
	vector<memInstruct> memTrace;
	string line;
	while (getline(inputFile, line) {
		istringstream iss(line);
		char op;
		unsigned long address;
		if (iss >> op >> hex >> address) {
			memTrace.push_back({operation, address});
		}
		
		else {
			cout << "Error: invalid input types." << endl;
			return 1;
		}
	}
	
	inputFile.close();
	ofStream outFile("output.txt");
	
	simDirectMapCache();
	simDirectMapCache();
	simDirectMapCache();
	simDirectMapCache();
	cout << endl;
	
	simSetAssocCache();
	simSetAssocCache();
	simSetAssocCache();
	simSetAssocCache();
	cout << endl;
	
	simFullAssocCache();
	cout << endl;
	
	simSetAssocNoAllocCache();
	cout << endl;
	
	simSetAssocNextLinePreFetchCache();
	simSetAssocNextLinePreFetchCache();
	simSetAssocNextLinePreFetchCache();
	simSetAssocNextLinePreFetchCache();
	cout << endl;
	
	simSetAssociativeOnMissPreFetchCache();
	simSetAssociativeOnMissPreFetchCache();
	simSetAssociativeOnMissPreFetchCache();
	simSetAssociativeOnMissPreFetchCache();
	cout << endl;
	
	outFile.close();
	return 0;
}
