#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

struct memInstruct {
	char op;
	unsigned long address;
};

struct cacheLine {
	bool valid;
	unsigned long tag;
};

const int cacheLineSize = 32;

void simDirectMapCache(const vector<memInstruct>& memTrace, ofstream& outFile, int cacheSize) {
	int numLines = cacheSize / cacheLineSize;
	vector<cacheLine> cache(numLines, {false, 0});
	int cacheHits = 0;
	int totalAccesses = 0;
	for (const auto& instruction : memTrace) {
		totalAccesses++;
		int cacheIndex = instruction.address / cacheLineSize % numLines;
		if (cache[cacheIndex].valid && (cache[cacheIndex].tag == instruction.address / cacheLineSize)) {
			cacheHits++;
		}
		
		else {
			cache[cacheIndex].valid = true;
			cache[cacheIndex].tag = instruction.address / cacheLineSize;
		}
	}
	
	outFile << cacheHits << "," << totalAccesses << ";";
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
	while (getline(inputFile, line)) {
		istringstream iss(line);
		char op;
		unsigned long address;
		if (iss >> op >> hex >> address) {
			memTrace.push_back({op, address});
		}
		
		else {
			cout << "Error: invalid input types." << endl;
			return 1;
		}
	}
	
	inputFile.close();
	ofstream outFile("output.txt");
	
	 
	simDirectMapCache(memTrace, outFile, 1028);
	simDirectMapCache(memTrace, outFile, 4*1028);
	simDirectMapCache(memTrace, outFile, 16*1028);
	simDirectMapCache(memTrace, outFile, 32*1028);
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
