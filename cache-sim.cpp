#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <math.h>

using namespace std;
const int cacheLineSize = 32;
const int setAssocCacheSize = 16384;
	
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

void simDirectMapCache(const vector<memInstruct>& memTrace, ofstream& outFile, int cacheSize) {
	int cacheHits = 0;
	int totalAccesses = 0;
	int numLines = cacheSize / cacheLineSize;
	vector<directCacheLine> cache(numLines, {false, 0});
	
	for (const auto& instruction : memTrace) {
		totalAccesses++;
		int cacheIndex = (instruction.address / cacheLineSize) % numLines;
		
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
	int cacheHits = 0;
	int totalAccesses = 0;
	int numLines = setAssocCacheSize / cacheLineSize;
	int numSets = numLines / associativity;
	vector<vector<setCacheLine>> cache(numSets, vector<setCacheLine>(associativity, {false, 0, 0}));
	
	for (const auto& instruction : memTrace) {
		totalAccesses++;
		int setIndex = (instruction.address / cacheLineSize) % numSets;
		auto& currentSet = cache[setIndex];
		auto iter = find_if(currentSet.begin(), currentSet.end(), [&](const setCacheLine& line) {
			return line.valid && line.tag == instruction.address / cacheLineSize;
		});
		
		if (iter != currentSet.end()) {
			cacheHits++;
			for (auto& line : currentSet) {
				line.lru++;
			}
			
			iter->lru = 0;
		}
			
		else {
			auto lruIter = max_element(currentSet.begin(), currentSet.end(), [](const setCacheLine& lineA, const setCacheLine& lineB) {
				return lineA.lru < lineB.lru;
			});
			
			for (auto& line : currentSet) {
				line.lru++;	
			}
			
			lruIter->valid = true;
			lruIter->tag = instruction.address / cacheLineSize;
			lruIter->lru = 0;
		}
	}
	
	outFile << cacheHits << "," << totalAccesses << "; ";
}

void simFullLRUCache(const vector<memInstruct>& memTrace, ofstream& outFile) {
	int cacheHits = 0;
	int totalAccesses = 0;
	int numLines = setAssocCacheSize / cacheLineSize;
	vector<setCacheLine> cache(numLines, {false, 0, 0});
	
	for (const auto& instruction : memTrace) {
		totalAccesses++;
		auto iter = find_if(cache.begin(), cache.end(), [&](const setCacheLine& line) {
			return line.valid && line.tag == instruction.address / cacheLineSize;
		});
		
		if (iter != cache.end()) {
			cacheHits++;
			for (auto& line : cache) {
				line.lru++;
			}
			
			iter->lru = 0;
		}
		
		else {
			auto lruIter = max_element(cache.begin(), cache.end(), [](const setCacheLine& lineA, const setCacheLine& lineB) {
				return lineA.lru < lineB.lru;
			});
			
			for (auto& line : cache) {
				line.lru++;	
			}
			
			lruIter->valid = true;
			lruIter->tag = instruction.address / cacheLineSize;
			lruIter->lru = 0;	
		}
	}
	
	outFile << cacheHits << "," << totalAccesses << "; ";	
}

void updateHotCold(vector<int>& hotCold, int position) {
	int bitToBeUpdated = position + hotCold.size();
	while (bitToBeUpdated > 0) {
		if (bitToBeUpdated % 2 == 0 ) {
			hotCold[(bitToBeUpdated - 1)] = 1;
		}
		
		else {
			hotCold[(bitToBeUpdated - 1)] = 1;
		}
		
		bitToBeUpdated = (bitToBeUpdated - 1) / 2;
	}
}

int findVictim(vector<int>& hotCold) {
	int position = 0;
	while (position < hotCold.size()) {
		if (hotCold[position] == 1) {
			position = 2 * position + 1;
		}
		
		else {
			position = 2 * position + 2;
		}
	}
	
	return (position - hotCold.size());
}

void simHotColdLRUCache(const vector<memInstruct>& memTrace, ofstream& outFile) {
	int cacheHits = 0;
	int totalAccesses = 0;
	int numLines = setAssocCacheSize / cacheLineSize;
	vector<setCacheLine> cache(numLines, {false, 0, 0}); 
	vector<int> hotCold(numLines - 1, 0);
	
	for (const auto& instruction : memTrace) {
		totalAccesses++;
		int cacheIndex = (instruction.address / cacheLineSize) % numLines;
		auto iter = find_if(cache.begin(), cache.end(), [&](const setCacheLine& line) {
			return line.valid && line.tag == instruction.address / cacheLineSize;
		});
		
		if (iter != cache.end()) {
			cacheHits++;
			updateHotCold(hotCold, cacheIndex + hotCold.size());
		}
		
		else {
			int victimIndex = findVictim(hotCold);
			victimIndex %= numLines;
			updateHotCold(hotCold, victimIndex + hotCold.size());
			setCacheLine& victimLine = cache[victimIndex];
			victimLine.valid = true;
			victimLine.tag = instruction.address / cacheLineSize;
		}
	}
	
	outFile << cacheHits << "," << totalAccesses << "; ";
}

void simSetAssocNoAllocCache(const vector<memInstruct>& memTrace, ofstream& outFile, int associativity) {
	int cacheHits = 0;
	int totalAccesses = 0;
	int numLines = setAssocCacheSize / cacheLineSize;
	int numSets = numLines / associativity;
	vector<vector<setCacheLine>> cache(numSets, vector<setCacheLine>(associativity, {false, 0, 0}));

	for (const auto& instruction : memTrace) {
		totalAccesses++;
		int setIndex = instruction.address / cacheLineSize % numSets;
		auto& currentSet = cache[setIndex];
		auto iter = find_if(currentSet.begin(), currentSet.end(), [&](const setCacheLine& line) {
			return line.valid && line.tag == instruction.address / cacheLineSize;
		});
		
		if (iter != currentSet.end()) {
			cacheHits++;
			for (auto& line : currentSet) {
				line.lru++;
			}
			
			iter->lru = 0;
		}
			
		else {
			auto lruIter = max_element(currentSet.begin(), currentSet.end(), [](const setCacheLine& lineA, const setCacheLine& lineB) {
				return lineA.lru < lineB.lru;
			});
			
			if (instruction.op == 'L') {
				for (auto& line : currentSet) { 
					line.lru++;
				}
				
				lruIter->valid = true;
				lruIter->tag = instruction.address / cacheLineSize;
				lruIter->lru = 0;
			}
			
			else if (instruction.op == 'S') {
				continue;
			}
		}
	}
	
	outFile << cacheHits << "," << totalAccesses << "; ";
}

void simSetAssocNextLinePreFetchCache(const vector<memInstruct>& memTrace, ofstream& outFile, int associativity) {
	int cacheHits = 0;
	int totalAccesses = 0;
	int numLines = setAssocCacheSize / cacheLineSize;
	int numSets = numLines / associativity;
	vector<vector<setCacheLine>> cache(numSets, vector<setCacheLine>(associativity, {false, 0, 0}));
	
	for (int i = 0; i < memTrace.size(); ++i) {
		totalAccesses++;
		const auto& instruction = memTrace[i];
		int setIndex = instruction.address / cacheLineSize % numSets;
		auto& currentSet = cache[setIndex];
		auto iter = find_if(currentSet.begin(), currentSet.end(), [&](const setCacheLine& line) {
			return line.valid && line.tag == instruction.address / cacheLineSize;
		});
		
		if (iter != currentSet.end()) {
			cacheHits++;
			for (auto& line : currentSet) {
				line.lru++;
			}
			
			iter->lru = 0;
		}
		
		else {
			auto lruIter = max_element(currentSet.begin(), currentSet.end(), [](const setCacheLine& lineA, const setCacheLine& lineB) {
				return lineA.lru < lineB.lru;
			});
			
			for (auto& line : currentSet) {
				line.lru++;
			}
			
			lruIter->valid = true;
			lruIter->tag = instruction.address / cacheLineSize;
			lruIter->lru = 0;
		}
		
		int preFetchSetIndex = (instruction.address + cacheLineSize) / cacheLineSize % numSets;
		auto& preFetchSet = cache[preFetchSetIndex];
		auto preFetchIter = find_if(preFetchSet.begin(), preFetchSet.end(), [&](const setCacheLine& line) {
			return line.valid && line.tag == (instruction.address + cacheLineSize) / cacheLineSize;
		});
		
		if (preFetchIter != preFetchSet.end()) {
			for (auto& line : preFetchSet) {
				line.lru++;
			}
			
			preFetchIter->lru = 0;
		}
		
		else {
			auto preFetchLruIter = max_element(preFetchSet.begin(), preFetchSet.end(), [](const setCacheLine& lineA, const setCacheLine& lineB) {
				return lineA.lru < lineB.lru;
			});
			
			for (auto& line : preFetchSet) {
				line.lru++;
			}
			
			preFetchLruIter->valid = true;
			preFetchLruIter->tag = (instruction.address + cacheLineSize) / cacheLineSize;
			preFetchLruIter->lru = 0;
		}
	}
	
	outFile << cacheHits << "," << totalAccesses << "; ";
}

void simSetAssociativeOnMissPreFetchCache(const vector<memInstruct>& memTrace, ofstream& outFile, int associativity) {
	int cacheHits = 0;
	int totalAccesses = 0;
	int numLines = setAssocCacheSize / cacheLineSize;
	int numSets = numLines / associativity;
	vector<vector<setCacheLine>> cache(numSets, vector<setCacheLine>(associativity, {false, 0, 0}));
	
	for (int i = 0; i < memTrace.size(); ++i) {
		totalAccesses++;
		const auto& instruction = memTrace[i];
		int setIndex = instruction.address / cacheLineSize % numSets;
		auto& currentSet = cache[setIndex];
		auto iter = find_if(currentSet.begin(), currentSet.end(), [&](const setCacheLine& line) {
			return line.valid && line.tag == (instruction.address) / cacheLineSize;
		});
		
		if (iter != currentSet.end()) {
			cacheHits++;
			for (auto& line : currentSet) {
				line.lru++;
			}
			
			iter->lru = 0;
		}
		
		else {
			auto lruIter = max_element(currentSet.begin(), currentSet.end(), [](const setCacheLine& lineA, const setCacheLine& lineB) {
				return lineA.lru < lineB.lru;
			});
			
			for (auto& line : currentSet) {
				line.lru++;
			}
			
			lruIter->valid = true;
			lruIter->tag = (instruction.address) / cacheLineSize;
			lruIter->lru = 0;
			
			int preFetchSetIndex = ((instruction.address + cacheLineSize) / cacheLineSize) % numSets;
			auto& preFetchSet = cache[preFetchSetIndex];
			auto preFetchIter = find_if(preFetchSet.begin(), preFetchSet.end(), [&](const setCacheLine& line) {
				return line.valid && line.tag == (instruction.address + cacheLineSize) / cacheLineSize;
			});
			
			if (preFetchIter != preFetchSet.end()) {
				for (auto& line : preFetchSet) {
					line.lru++;
				}
				
				preFetchIter->lru = 0;
			}
			
			else {
				auto preFetchLruIter = max_element(preFetchSet.begin(), preFetchSet.end(), [](const setCacheLine& lineA, const setCacheLine& lineB) {
					return lineA.lru < lineB.lru;
				});
				
				for (auto& line : preFetchSet) {
					line.lru++;
				}
				
				preFetchLruIter->valid = true;
				preFetchLruIter->tag = (instruction.address + cacheLineSize) / cacheLineSize;
				preFetchLruIter->lru = 0;
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
	ofstream outFile(argv[2]);
	
	vector<int> directCacheSizes {1024, 4096, 16384, 32768};
	vector<int> associativity {2, 4, 8, 16};
	
	for (int i = 0; i < directCacheSizes.size(); ++i) {
		simDirectMapCache(memTrace, outFile, directCacheSizes[i]);
	}
	
	outFile << endl;
	
	for (int i = 0; i < associativity.size(); ++i) {
		simSetAssocCache(memTrace, outFile, associativity[i]);
	}
	
	outFile << endl;
	
	simFullLRUCache(memTrace, outFile);
	outFile << endl;
	
	simHotColdLRUCache(memTrace, outFile);
	outFile << endl;
	
	for (int i = 0; i < associativity.size(); ++i) {
		simSetAssocNoAllocCache(memTrace, outFile, associativity[i]);
	}
	
	outFile << endl;
	
	for (int i = 0; i < associativity.size(); ++i) {
		simSetAssocNextLinePreFetchCache(memTrace, outFile, associativity[i]);
	}
	
	outFile << endl;
	
	for (int i = 0; i < associativity.size(); ++i) {
		simSetAssociativeOnMissPreFetchCache(memTrace, outFile, associativity[i]);
	}
	
	outFile << endl;
	
	outFile.close();
	return 0;
}
