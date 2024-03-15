#include <iostream>
#include <fstream>
#include <unordered_map>
#include <math.h>
#include <set>
#include <algorithm>

typedef struct {
    char symbol;
    double probability;
    double comp_prob;
} SymbolProb;

SymbolProb sProb;
uint32_t modelPointer = 0;
uint32_t hits = 0,misses = 0,alphabetSize = 0,k = 0,bufferSize = 5;
double totalBits = 0,threshold = 0,smoothing = 0;
std::unordered_map<std::string,int> posOfSequences;
std::string charactersRead = "";
bool repeatModelStopped = false;

int getSizeOfAlphabet(std::ifstream& stream) {
    std::set<char> alphabet; 
    char buffer[bufferSize + 1];
    stream.read(buffer,bufferSize);
    int bufferLen = 0;
    while((bufferLen = stream.gcount())) {
        for(int i = 0; i < bufferLen; i++)
            alphabet.insert(buffer[i]);

        stream.read(buffer,bufferSize);
    }

    return alphabet.size();
}

void repeatPredict(char symbolAtModelPoint,char symbolToBePredicted) {
    bool hit = symbolAtModelPoint == symbolToBePredicted;
    hits += hit;
    misses += !hit;

    sProb.probability = (hits + smoothing) / (hits + misses + 2 * smoothing);
    sProb.comp_prob = (1 - sProb.probability) / (alphabetSize - 1);
    sProb.symbol = symbolToBePredicted;
    totalBits += -log2(sProb.probability);
}

void fallbackPredict(char symbol) {
    const size_t fallbackWindowSize = 200;
    size_t len = std::min<size_t>(charactersRead.size(),fallbackWindowSize);

    std::unordered_map<char,int> charCount;
    for(char c : charactersRead.substr(charactersRead.size() - len,len)) {
        charCount[c]++;
    }

    double totalBitsForSymbol = 0;
    double dLen = static_cast<double>(len);
    for(const auto& pair : charCount) {
        double division = pair.second / dLen;
        totalBitsForSymbol += -division * log2(division);
    }

    sProb.probability = std::pow(2,-totalBitsForSymbol);
    sProb.comp_prob = (1 - sProb.probability) / (alphabetSize - 1);
    sProb.symbol = symbol;
    totalBits += totalBitsForSymbol;
}

bool shouldStopRepeatModel() {
    if((hits + misses) == 0)
        return false;
    
    double x = (static_cast<double>(hits) / (hits + misses));
    return !repeatModelStopped && x < threshold;
}

void processString(int startPos) {
    if(charactersRead.length() < k) {
        return;
    }

    std::string kmer = "";
    for(int windowPointer = startPos; windowPointer < charactersRead.length(); windowPointer++, modelPointer++) {
        kmer = charactersRead.substr(windowPointer - k + 1,k);
        if(repeatModelStopped) {
            auto it = posOfSequences.find(kmer);
            if(it != posOfSequences.end()) {
                repeatModelStopped = false;
                hits = 0;
                misses = 0;
                modelPointer = posOfSequences[kmer];
            }
        }
        
        if(repeatModelStopped) {
            fallbackPredict(charactersRead.at(windowPointer));
        }
        else {
            repeatPredict(charactersRead.at(modelPointer),charactersRead.at(windowPointer));
            repeatModelStopped = shouldStopRepeatModel();
        }

        posOfSequences[kmer] = windowPointer;
    }
}

int main(int argc,char* argv[]) {

    // if (argc != 5) {
    //     std::cerr << "Uso: " << argv[0] << " <filename> <window size> <threshold> <alpha>" << std::endl;
    //     return 1;
    // }

    // std::string filename = argv[1];
    // k = std::stoi(argv[2]);
    // threshold = std::stod(argv[3]);
    // smoothing = std::stod(argv[4]);
    //DEBUG
    std::string filename = "./teste.txt";
    k = 3;
    threshold = 0.3;
    smoothing = 1;

    std::ifstream inputFile(filename);
    
    if(!inputFile.is_open()) {
        std::cout << "Cannot open file!" << std::endl;
        return -1;
    }

    alphabetSize = getSizeOfAlphabet(inputFile);

    if(alphabetSize == 0) {
        std::cout << "File is empty!" << std::endl;
        return 1;
    }

    inputFile.clear();
    inputFile.seekg(0,std::ios::beg);

    for(int i = 0; i < k; i++) {
        charactersRead += 'A';
    }

    char buffer[bufferSize + 1];
    inputFile.read(buffer,bufferSize);
    int bufferLen;

    while ((bufferLen = inputFile.gcount())) {
        buffer[bufferLen] = '\0';
        std::string s(buffer);
        charactersRead += s;        

        processString(charactersRead.size() - s.size());
        inputFile.read(buffer,bufferSize);
    } 

    inputFile.close();

    //TODO: escrever os bits no ficheiro
    return 0;
}