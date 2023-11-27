#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <list>

using namespace std;

struct memInstruct {
	char op;
	unsigned long address;
};

struct directCacheLine {
	bool valid;
	unsigned long tag;
};

struct setCacheLine {
	bool valid;
	unsigned long tag;
	int lru;
};

const int cacheLineSize = 32;

void simDirectMapCache(const vector<memInstruct>& memTrace, ofstream& outFile, int cacheSize) {
	int numLines = cacheSize / cacheLineSize;
	vector<directCacheLine> cache(numLines, {false, 0});
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
	
	outFile << cacheHits << "," << totalAccesses << "; ";
}

void simSetAssocCache(const vector<memInstruct>& memTrace, ofstream& outFile, int associativity) {
	int numLines = 16384 / cacheLineSize;
	int sets = numLines / associativity;
	vector<vector<setCacheLine>> cache(sets, vector<setCacheLine>(associativity, {false, 0, 0}));
	int cacheHits = 0;
	int totalAccesses = 0;
	for (const auto& instruction : memTrace) {
		totalAccesses++;
		int setIndex = instruction.address / cacheLineSize % sets;
		auto& set = cache[setIndex];
		auto it = find_if(set.begin(), set.end(), [&](const setCacheLine& line) {
			return line.valid && line.tag == instruction.address / cacheLineSize;
		});
		
		if (it != set.end()) {
			cacheHits++;
			for (auto& line : set) {
				line.lru++;
			}
			
			it->lru = 0;
		}
		
		else {
			auto lruIt = max_element(set.begin(), set.end(), [](const setCacheLine& lineA, const setCacheLine& lineB) {
				return lineA.lru < lineB.lru;
			});
			
			lruIt->valid = true;
			lruIt->tag = instruction.address / cacheLineSize;
			lruIt->lru = 0;
			for (auto& line : set) {
				if (&line != &(*lruIt)) {
					line.lru++;
				}
			}
		}
	}
	
	outFile << cacheHits << "," << totalAccesses << "; ";
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
	ifstream inputFile("trace1.txt");
	if (!(inputFile.is_open())) {
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
	
	 
	simDirectMapCache(memTrace, outFile, 1024);
	simDirectMapCache(memTrace, outFile, 4096);
	simDirectMapCache(memTrace, outFile, 16384);
	simDirectMapCache(memTrace, outFile, 32768);
	outFile << endl;
	
	simSetAssocCache(memTrace, outFile, 2);
	simSetAssocCache(memTrace, outFile, 4);
	simSetAssocCache(memTrace, outFile, 8);
	simSetAssocCache(memTrace, outFile, 16);
	outFile << endl;
	
	simFullAssocCache();
	outFile << endl;
	
	simSetAssocNoAllocCache();
	outFile << endl;
	
	simSetAssocNextLinePreFetchCache();
	simSetAssocNextLinePreFetchCache();
	simSetAssocNextLinePreFetchCache();
	simSetAssocNextLinePreFetchCache();
	outFile << endl;
	
	simSetAssociativeOnMissPreFetchCache();
	simSetAssociativeOnMissPreFetchCache();
	simSetAssociativeOnMissPreFetchCache();
	simSetAssociativeOnMissPreFetchCache();
	outFile << endl;
	
	outFile.close();
	return 0;
}
