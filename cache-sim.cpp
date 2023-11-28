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
			
			if (instruction.op == 'L') {
				lruIt->valid = true;
				lruIt->tag = instruction.address / cacheLineSize;
				lruIt->lru = 0;
				for (auto& line : currentSet) {
					if (&line != &(*lruIt)) {
						line.lru++;
					}
				}
			}
			
			else if (instruction.op == 'S') {
				for (auto& line : currentSet) {
					if (&line != &(*lruIt)) {
						line.lru++;
					}
				}
				
				lruIt->valid = true;
				lruIt->tag = instruction.address / cacheLineSize;
				lruIt->lru = 0;
			}
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
			
			if (instruction.op == 'L') {
				lruIt->valid = true;
				lruIt->tag = instruction.address / cacheLineSize;
				lruIt->lru = 0;
				for (auto& line : cache) {
					if (&line != &(*lruIt)) {
						line.lru++;
					}
				}
			}
			
			else if (instruction.op == 'S') {
				for (auto& line : cache) {
					line.lru++;	
				}
				
				lruIt->valid = true;
				lruIt->tag = instruction.address / cacheLineSize;
				lruIt->lru = 0;
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
			
			for (auto& line : cache) {
				if (&line != &(*it)) {
					line.lru++;
					line.hotCold++;
				}
			}
			
			/*
			if (it->hotCold == 0 && instruction.op == 'L') {
				it->hotCold = (it->tag % numLines) < (numLines / 2) ? 1 : 0;
			}
			
			else if (it->hotCold == 1 && instruction.op == 'L') {
				it->hotCold = (it->tag % numLines) < (numLines / 2) ? 0 : 1;
			}
			*/
			if (instruction.op == 'L') {
				it->hotCold = (it->tag % numLines) < (numLines / 2) ? 0 : 1;
			}
		}
		
		else {
			auto lruIt = max_element(cache.begin(), cache.end(), [](const fullCacheLine& lineA, const fullCacheLine& lineB) {
				return lineA.lru < lineB.lru;
			});
			
			if (instruction.op == 'L') {
				lruIt->valid = true;
				lruIt->tag = instruction.address / cacheLineSize;
				lruIt->lru = 0;
				for (auto& line : cache) {
					line.lru++;
				}
				
				/*
				if (lruIt->hotCold == 0) {
					lruIt->hotCold = (lruIt->tag % numLines) < (numLines / 2) ? 1 : 0;
				}
				
				if (lruIt->hotCold == 1) {
					lruIt->hotCold = (lruIt->tag % numLines) < (numLines / 2) ? 0 : 1;
				}
				*/
				
				lruIt->hotCold = (lruIt->tag % numLines) < (numLines / 2) ? 0 : 1;
			}
			
			else if (instruction.op == 'S') {
				for (auto& line : cache) {
					line.lru++;
				}
				
				auto hotColdIt = max_element(cache.begin(), cache.end(), [](const fullCacheLine& lineA, const fullCacheLine& lineB) {
					return lineA.hotCold < lineB.hotCold;
				});
				
				hotColdIt->valid = true;
				hotColdIt->tag = instruction.address / cacheLineSize;
				hotColdIt->lru = 0;
				
				/*
				if (hotColdIt->hotCold == 0) {
					hotColdIt->hotCold = (hotColdIt->tag % numLines) < (numLines / 2) ? 1 : 0;
				}
				
				if (hotColdIt->hotCold == 1) {
					hotColdIt->hotCold = (hotColdIt->tag % numLines) < (numLines / 2) ? 0 : 1;
				}
				*/
				
				hotColdIt->hotCold = (hotColdIt->tag % numLines) < (numLines / 2) ? 0 : 1;
			}
		}
	}
	
	outFile << cacheHits << "," << totalAccesses << "; ";
}

void simSetAssocNoAllocCache(const vector<memInstruct>& memTrace, ofstream& outFile, int associativity) {
	int numLines = 16384 / cacheLineSize;
	int numSets = numLines / associativity;
	vector<vector<setCacheLine>> cache(numSets, vector<setCacheLine>(associativity, {false, 0, 0}));
	int cacheHits = 0;
	int totalAccesses = 0;
	for (const auto& instruction : memTrace) {
		totalAccesses++;
		int setIndex = instruction.address / cacheLineSize % numSets;
		auto& currentSet = cache[setIndex];
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
			
			if (instruction.op == 'L') {
				lruIt->valid = true;
				lruIt->tag = instruction.address / cacheLineSize;
				lruIt->lru = 0;
				for (auto& line : currentSet) {
					if (&line != &(*lruIt)) {
						line.lru++;
					}
				}
			}
			
			else if (instruction.op == 'S') {
				continue;
			}
		}
	}
	
	outFile << cacheHits << "," << totalAccesses << "; ";
}

void simSetAssocNextLinePreFetchCache(const vector<memInstruct>& memTrace, ofstream& outFile, int associativity) {
	int numLines = 16384 / cacheLineSize;
	int numSets = numLines / associativity;
	vector<vector<setCacheLine>> cache(numSets, vector<setCacheLine>(associativity, {false, 0, 0}));
	int cacheHits = 0;
	int totalAccesses = 0;
	for (int i = 0; i < memTrace.size(); ++i) {
		totalAccesses++;
		const auto& instruction = memTrace[i];
		int setIndex = instruction.address / cacheLineSize % numSets;
		auto& currentSet = cache[setIndex];
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
			
			for (auto& line : currentSet) {
				line.lru++;
			}
			
			lruIt->valid = true;
			lruIt->tag = instruction.address / cacheLineSize;
			lruIt->lru = 0;
		}
		
		auto preFetchIt = currentSet.begin();
		
		if (it != currentSet.end()) {
			preFetchIt = next(it);
		}
		
		if (preFetchIt == currentSet.end()) {
			preFetchIt = currentSet.begin();
		}
		
		preFetchIt->lru = 0;
		for (auto& line : currentSet) {
			if (&line != &(*preFetchIt)) {
				line.lru++;
			}
		}
		
		/*	
		if (preFetchIt != currentSet.end()) {
			preFetchIt->lru = 0;
			for (auto& line : currentSet) {
				if (&line != &(*preFetchIt)) {
					line.lru++;
				}
			}
		}
		
		else {
			auto lruIt = max_element(currentSet.begin(), currentSet.end(), [](const setCacheLine& lineA, const setCacheLine& lineB) {
				return lineA.lru < lineB.lru;
			});
			
			for (auto& line : currentSet) {
				line.lru++;	
			}
			
			lruIt->valid = true;
			lruIt->tag = instruction.address / cacheLineSize;
			lruIt->lru = 0;
		}
		*/
	}
	
	outFile << cacheHits << "," << totalAccesses << "; ";
}

void simSetAssociativeOnMissPreFetchCache(const vector<memInstruct>& memTrace, ofstream& outFile, int associativity) {
	int numLines = 16384 / cacheLineSize;
	int numSets = numLines / associativity;
	vector<vector<setCacheLine>> cache(numSets, vector<setCacheLine>(associativity, {false, 0, 0}));
	int cacheHits = 0;
	int totalAccesses = 0;
	for (int i = 0; i < memTrace.size(); ++i) {
		totalAccesses++;
		const auto& instruction = memTrace[i];
		int setIndex = instruction.address / cacheLineSize % numSets;
		auto& currentSet = cache[setIndex];
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
			
			for (auto& line : currentSet) {
				line.lru++;
			}
			
			lruIt->valid = true;
			lruIt->tag = instruction.address / cacheLineSize;
			lruIt->lru = 0;
			auto preFetchIt = currentSet.begin();
		
			if (it != currentSet.end()) {
				preFetchIt = next(it);
			}
			
			if (preFetchIt == currentSet.end()) {
				preFetchIt = currentSet.begin();
			}
			
			
			preFetchIt->lru = 0;
			for (auto& line : currentSet) {
				if (&line != &(*preFetchIt)) {
					line.lru++;
				}
			}
			/*
			if (preFetchIt != currentSet.end()) {
				preFetchIt->lru = 0;
				for (auto& line : currentSet) {
					if (&line != &(*preFetchIt)) {
						line.lru++;
					}
				}
			}
			
			else {
				auto lruIt = max_element(currentSet.begin(), currentSet.end(), [](const setCacheLine& lineA, const setCacheLine& lineB) {
					return lineA.lru < lineB.lru;
				});
				
				for (auto& line : currentSet) {
					line.lru++;	
				}
				
				lruIt->valid = true;
				lruIt->tag = instruction.address / cacheLineSize;
				lruIt->lru = 0;
			}
			*/
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
	
	outFile.close();
	return 0;
}
