#include <iostream>
#include <fstream>
#include <unordered_map>
#include <math.h>
#include <set>
#include <algorithm>
#include <stdint.h>
#include <iomanip>

typedef struct {
    char symbol;
    double probability;
    double comp_prob;
} SymbolProb;

SymbolProb sProb;
size_t hits = 0,misses = 0,alphabetSize = 0,k = 0,bufferSize = 1024,modelPointer = 0;
double totalBits = 0,threshold = 0,smoothing = 0;
std::unordered_map<std::string,size_t> posOfSequences;
std::string* charactersRead = new std::string("");
bool repeatModelStopped = false;

double fallbackBits = 0,repeatBits = 0;
size_t fallbackCalls = 0,repeatCalls = 0;

int getSizeOfAlphabet(std::ifstream& stream) {
    std::set<char> alphabet; 
    char buffer[bufferSize + 1];
    stream.read(buffer,bufferSize);
    size_t bufferLen = 0;
    while((bufferLen = stream.gcount())) {
        for(size_t i = 0; i < bufferLen; i++)
            alphabet.insert(buffer[i]);

        stream.read(buffer,bufferSize);
    }

    return alphabet.size();
}

void repeatPredict(char symbolAtModelPoint,char symbolToBePredicted) {
    repeatCalls++;
    bool hit = symbolAtModelPoint == symbolToBePredicted;
    hits += hit;
    misses += !hit;

    sProb.probability = (hits + smoothing) / (hits + misses + 2 * smoothing);
    sProb.comp_prob = (1 - sProb.probability) / (alphabetSize - 1);
    sProb.symbol = symbolToBePredicted;
    double bits = -log2(sProb.probability);
    repeatBits += bits;
    totalBits += bits;
}

void fallbackPredict(char symbol) {
    fallbackCalls++;
    const size_t fallbackWindowSize = 200;
    size_t len = std::min<size_t>((*charactersRead).size(),fallbackWindowSize);

    std::unordered_map<char,uint16_t> charCount;
    for(char c : (*charactersRead).substr((*charactersRead).size() - len,len)) {
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
    fallbackBits += totalBitsForSymbol;
    totalBits += totalBitsForSymbol;
}

bool shouldStopRepeatModel() {
    if((hits + misses) == 0)
        return false;
    
    return (static_cast<double>(hits) / (hits + misses)) < threshold;
}

void processString(int startPos) {
    if((*charactersRead).length() < k) {
        return;
    }

    std::string kmer = "";
    for(size_t windowPointer = startPos; windowPointer < (*charactersRead).length(); windowPointer++, modelPointer++) {
        kmer = (*charactersRead).substr(windowPointer - k + 1,k);
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
            fallbackPredict((*charactersRead).at(windowPointer));
        }
        else {
            repeatPredict((*charactersRead).at(modelPointer),(*charactersRead).at(windowPointer));
            repeatModelStopped = shouldStopRepeatModel();
        }

        posOfSequences[kmer] = windowPointer;
    }
}

void writeResultsToFile(std::string filename) {
    std::string outputFilename = filename + "_results.txt";

    std::ofstream outputFile(outputFilename);
    if (!outputFile.is_open()) {
        std::cerr << "Cannot create output file " << outputFilename << std::endl;
        return;
    }

    outputFile << "Params: " << filename << " alpha: " << smoothing << " window size: " << k << " threshold: " << threshold << "\n";
    outputFile << "Estimated total bits: " << std::fixed << std::setprecision(2) << totalBits << "\n";
    outputFile << "Estimated total bytes: " << std::fixed << std::setprecision(2) << totalBits / 8 << "\n";
    outputFile << "Average number of bits per symbol: " << totalBits / (*charactersRead).size() << "\n";
    outputFile.close();

    std::cout << "repeat model bits: " << std::fixed << std::setprecision(2) << repeatBits << "\n";
    std::cout << "fallback model bits: " << std::fixed << std::setprecision(2) << fallbackBits << "\n";
    std::cout << "repeat calls: " << repeatCalls << "\n";
    std::cout << "fallback calls: " << fallbackCalls << "\n";
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
    std::string filename = "./chry.txt";
    k = 11;
    threshold = 0.8;
    smoothing = 1;

    std::ifstream inputFile(filename);
    
    if(!inputFile.is_open()) {
        std::cerr << "Cannot open file!" << std::endl;
        return -1;
    }

    alphabetSize = getSizeOfAlphabet(inputFile);

    if(alphabetSize == 0) {
        std::cerr << "File is empty!" << std::endl;
        return -1;
    }

    inputFile.clear();
    inputFile.seekg(0,std::ios::beg);

    for(size_t i = 0; i < k; i++) {
        (*charactersRead) += 'A';
    }

    char buffer[bufferSize + 1];
    inputFile.read(buffer,bufferSize);
    size_t bufferLen;

    while ((bufferLen = inputFile.gcount())) {
        buffer[bufferLen] = '\0';
        std::string s(buffer);
        (*charactersRead) += s;        

        processString((*charactersRead).size() - s.size());
        inputFile.read(buffer,bufferSize);
    } 

    inputFile.close();

    writeResultsToFile(filename);    

    return 0;
}