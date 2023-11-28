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

struct fullCacheLine {
	bool valid;
	unsigned long tag;
	int lru;
	int hotCold;
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
		if (instruction.op == 'L') {
			if (cache[cacheIndex].valid && (cache[cacheIndex].tag == instruction.address / cacheLineSize)) {
				cacheHits++;
			}
			
			else {
				cache[cacheIndex].valid = true;
				cache[cacheIndex].tag = instruction.address / cacheLineSize;
			}
		}
		
		else if (instruction.op == 'S') {
			if (cache[cacheIndex].valid && (cache[cacheIndex].tag == instruction.address / cacheLineSize)) {
				cacheHits++;
			}
			
			cache[cacheIndex].valid = true;
			cache[cacheIndex].tag = instruction.address / cacheLineSize;
		}
	}
	
	outFile << cacheHits << "," << totalAccesses << "; ";
}

void simSetAssocCache(const vector<memInstruct>& memTrace, ofstream& outFile, int associativity) {
	int numLines = 16384 / cacheLineSize;
	int numSets = numLines / associativity;
	vector<vector<setCacheLine>> cache(numSets, vector<setCacheLine>(associativity, {false, 0, 0}));
	int cacheHits = 0;
	int totalAccesses = 0;
	for (const auto& instruction : memTrace) {
		totalAccesses++;
		int setIndex = instruction.address / cacheLineSize % numSets;
		auto& currentSet = cache[setIndex];
		if (instruction.op == 'L') {
			auto it = find_if(currentSet.begin(), currentSet.end(), [&](const setCacheLine& line) {
				return line.valid && line.tag == instruction.address / cacheLineSize;
			});
		
			if (it != currentSet.end()) {
				cacheHits++;
				for (auto& line : currentSet) {
					line.lru++;
				}
				
				it->lru = 0;
			}
			
			else {
				auto lruIt = max_element(currentSet.begin(), currentSet.end(), [](const setCacheLine& lineA, const setCacheLine& lineB) {
					return lineA.lru < lineB.lru;
				});
				lruIt->valid = true;
				lruIt->tag = instruction.address / cacheLineSize;
				lruIt->lru = 0;
				for (auto& line : currentSet) {
					if (&line != &(*lruIt)) {
						line.lru++;
					}
				}
			}
		}
		
		else if (instruction.op == 'S') {
			auto it = find_if(currentSet.begin(), currentSet.end(), [&](const setCacheLine& line) {
				return line.valid && line.tag == instruction.address / cacheLineSize;
			});
		
			if (it != currentSet.end()) {
				cacheHits++;
				for (auto& line : currentSet) {
					line.lru++;
				}
				
				it->lru = 0;
			}
			
			auto lruIt = max_element(currentSet.begin(), currentSet.end(), [](const setCacheLine& lineA, const setCacheLine& lineB) {
				return lineA.lru < lineB.lru;
			});
			lruIt->valid = true;
			lruIt->tag = instruction.address / cacheLineSize;
			lruIt->lru = 0;
		}
	}
	
	outFile << cacheHits << "," << totalAccesses << "; ";
}

void simFullLRUCache(const vector<memInstruct>& memTrace, ofstream& outFile) {
	int numLines = 16384 / cacheLineSize;
	vector<fullCacheLine> cache(numLines, {false, 0, 0, 0});
	int cacheHits = 0;
	int totalAccesses = 0;
	for (const auto& instruction : memTrace) {
		totalAccesses++;
		auto it = find_if(cache.begin(), cache.end(), [&](const fullCacheLine& line) {
			return line.valid && line.tag == instruction.address / cacheLineSize;
		});
		
		if (it != cache.end()) {
			cacheHits++;
			it->lru = 0;
			for (auto& line : cache) {
				if (&line != &(*it)) {
					line.lru++;
				}
			}
		}
		
		else {
			auto lruIt = max_element(cache.begin(), cache.end(), [](const fullCacheLine& lineA, const fullCacheLine& lineB) {
				return lineA.lru < lineB.lru;
			});
			
			lruIt->valid = true;
			lruIt->tag = instruction.address / cacheLineSize;
			lruIt->lru = 0;
			for (auto& line : cache) {
				if (&line != &(*lruIt)) {
					line.lru++;
				}
			}	
		}
	}
	
	outFile << cacheHits << "," << totalAccesses << "; ";	
}

void simHotColdLRUCache(const vector<memInstruct>& memTrace, ofstream& outFile) {
	int numLines = 16384 / cacheLineSize;
	vector<fullCacheLine> cache(numLines, {false, 0, 0, 0});
	int cacheHits = 0;
	int totalAccesses = 0;
	for (const auto& instruction : memTrace) {
		totalAccesses++;
		auto it = find_if(cache.begin(), cache.end(), [&](const fullCacheLine& line) {
			return line.valid && line.tag == instruction.address / cacheLineSize;
		});
		
		if (it != cache.end()) {
			cacheHits++;
			it->lru = 0;
			it->hotCold = 0;
			for (auto& line : cache) {
				if (&line != &(*it)) {
					line.lru++;
					line.hotCold++;
				}
			}
		}
		
		else {
			auto lruIt = max_element(cache.begin(), cache.end(), [](const fullCacheLine& lineA, const fullCacheLine& lineB) {
				return lineA.lru < lineB.lru;
			});
			
			
			lruIt = max_element(cache.begin(), cache.end(), [](const fullCacheLine& lineA, const fullCacheLine& lineB) {
				return lineA.hotCold < lineB.hotCold;
			});
			
			
			lruIt->valid = true;
			lruIt->tag = instruction.address / cacheLineSize;
			lruIt->lru = 0;
			lruIt->hotCold = 0;
			for (auto& line : cache) {
				line.lru++;
				line.hotCold++;
			}
		}
	}
	
	outFile << cacheHits << "," << totalAccesses << "; ";
}

void simSetAssocNoAllocCache(const vector<memInstruct>& memTrace, ofstream& outFile, int associativity) {
	int numLines = 16384 / cacheLineSize;
	int sets = numLines / associativity;
	vector<vector<setCacheLine>> cache(sets, vector<setCacheLine>(associativity, {false, 0, 0}));
	int cacheHits = 0;
	int totalAccesses = 0;
	for (const auto& instruction : memTrace) {
		totalAccesses++;
		int setIndex = instruction.address / cacheLineSize % sets;
		auto& currentSet = cache[setIndex];
		auto it = find_if(currentSet.begin(), currentSet.end(), [&](const setCacheLine& line) {
			return line.valid && line.tag == instruction.address / cacheLineSize;
		});
		
		if (it != currentSet.end()) {
			cacheHits++;
			it->lru = 0;
		}
		
		else if (instruction.op == 'S') {
		
		}
		
		else {
			auto lruIt = max_element(currentSet.begin(), currentSet.end(), [](const setCacheLine& lineA, const setCacheLine& lineB) {
				return lineA.lru < lineB.lru;
			});
			
			lruIt->valid = true;
			lruIt->tag = instruction.address / cacheLineSize;
			lruIt->lru = 0;
			for (auto& line : currentSet) {
				if (&line != &(*lruIt)) {
					line.lru++;
				}
			}
		}
	}
	
	outFile << cacheHits << "," << totalAccesses << "; ";
}

void simSetAssocNextLinePreFetchCache(const vector<memInstruct>& memTrace, ofstream& outFile, int associativity) {
	int numLines = 16384 / cacheLineSize;
	int sets = numLines / associativity;
	vector<vector<setCacheLine>> cache(sets, vector<setCacheLine>(associativity, {false, 0, 0}));
	int cacheHits = 0;
	int totalAccesses = 0;
	for (const auto& instruction : memTrace) {
		totalAccesses++;
		int setIndex = instruction.address / cacheLineSize % sets;
		auto& set = cache[setIndex];
		int nextLineIndex = (instruction.address / cacheLineSize + 1) % sets;
		int nextLineTag = nextLineIndex / associativity;
		auto preFetchIt = find_if(set.begin(), set.end(), [&](const setCacheLine& line) {
			return line.valid && line.tag == instruction.address / cacheLineSize;
		});
		
		if (preFetchIt == set.end()) {
			auto lruIt = max_element(set.begin(), set.end(), [](const setCacheLine& lineA, const setCacheLine& lineB) {
				return lineA.lru < lineB.lru;
			});
			
			lruIt->valid = true;
			lruIt->tag = instruction.address / cacheLineSize;
			lruIt->lru = 0;
		}
		
		else {
			preFetchIt->lru = 0;
		}
		
		auto it = find_if(set.begin(), set.end(), [&](const setCacheLine& line) {
			return line.valid && line.tag == instruction.address / cacheLineSize;
		});
		
		if (it != set.end()) {
			cacheHits++;
			it->lru = 0;
		}
		
		else {
			auto lruIt = max_element(set.begin(), set.end(), [](const setCacheLine& lineA, const setCacheLine& lineB) {
				return lineA.lru < lineB.lru;
			});
			
			lruIt->valid = true;
			lruIt->tag = instruction.address / cacheLineSize;
			lruIt->lru = 0;
		}
		
		for (auto& line : set) {
			if (&line != &(*it)) {
				line.lru++;
			}
		}
	}
	
	outFile << cacheHits << "," << totalAccesses << "; ";
}

void simSetAssociativeOnMissPreFetchCache(const vector<memInstruct>& memTrace, ofstream& outFile, int associativity) {
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
			it->lru = 0;
		}
		
		else {
			auto lruIt = max_element(set.begin(), set.end(), [](const setCacheLine& lineA, const setCacheLine& lineB) {
				return lineA.lru < lineB.lru;
			});
			
			lruIt->valid = true;
			lruIt->tag = instruction.address / cacheLineSize;
			lruIt->lru = 0;
			int nextLineIndex = (instruction.address / cacheLineSize + 1) % sets;
			int nextLineTag = nextLineIndex / associativity;
			auto preFetchIt = find_if(set.begin(), set.end(), [&](const setCacheLine& line) {
				return line.valid && line.tag == instruction.address / cacheLineSize;
			});
		
			if (preFetchIt == set.end()) {
				auto lruIt = max_element(set.begin(), set.end(), [](const setCacheLine& lineA, const setCacheLine& lineB) {
					return lineA.lru < lineB.lru;
				});
				
				lruIt->valid = true;
				lruIt->tag = instruction.address / cacheLineSize;
				lruIt->lru = 0;
			}
			
			else {
				preFetchIt->lru = 0;
			}
		}
		
		for (auto& line : set) {
			if (&line != &(*it)) {
				line.lru++;
			}
		}
	}
	
	outFile << cacheHits << "," << totalAccesses << "; ";
}

int main(int argc, char *argv[]) {
	string instructFile(argv[1]);
	ifstream inputFile(instructFile);
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
	
	/*
	simFullLRUCache(memTrace, outFile);
	outFile << endl;
	
	simHotColdLRUCache(memTrace, outFile);
	outFile << endl;
	
	simSetAssocNoAllocCache(memTrace, outFile, 2);
	simSetAssocNoAllocCache(memTrace, outFile, 4);
	simSetAssocNoAllocCache(memTrace, outFile, 8);
	simSetAssocNoAllocCache(memTrace, outFile, 16);
	outFile << endl;
	
	simSetAssocNextLinePreFetchCache(memTrace, outFile, 2);
	simSetAssocNextLinePreFetchCache(memTrace, outFile, 4);
	simSetAssocNextLinePreFetchCache(memTrace, outFile, 8);
	simSetAssocNextLinePreFetchCache(memTrace, outFile, 16);
	outFile << endl;
	
	simSetAssociativeOnMissPreFetchCache(memTrace, outFile, 2);
	simSetAssociativeOnMissPreFetchCache(memTrace, outFile, 4);
	simSetAssociativeOnMissPreFetchCache(memTrace, outFile, 8);
	simSetAssociativeOnMissPreFetchCache(memTrace, outFile, 16);
	outFile << endl;
	*/
	
	outFile.close();
	return 0;
}
