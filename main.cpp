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
int hits = 0,misses = 0,alphabetSize = 0,k = 0,bufferSize = 5;
double totalBits = 0,threshold = 0,smoothing = 0;
std::unordered_map<std::string,int> posOfSequences;

void predict(char symbol) {
    sProb.probability = (hits + smoothing) / (hits+ misses + 2 * smoothing);
    sProb.comp_prob = (1 - sProb.probability) / (alphabetSize - 1);
    totalBits += -log2(sProb.probability);  
}

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

bool shouldStopRepeatModel() {
    return (hits + threshold) < (hits + misses);
}

void updatePointerPosition(std::string& kmer,int& pointer) {
    if(posOfSequences.find(kmer) != posOfSequences.end()) {
        pointer = posOfSequences[kmer];
    }
    else {
        pointer--;
    }

    hits = 0;
    misses = 0;
}

void processString(const std::string& s,int startPos) {
    if(s.length() < k) {
        return;
    }

    std::string kmer = "";
    for(int windowPointer = startPos,modelPointer = startPos - 1; windowPointer < s.length(); windowPointer++, modelPointer++) {
        kmer = s.substr(windowPointer - k + 1,k);
        posOfSequences[kmer] = windowPointer;
        predict(s.at(modelPointer));
        if(shouldStopRepeatModel()) {
            updatePointerPosition(kmer,modelPointer);
        }
    }
}

int main(int argc,char* argv[]) {

    if (argc != 5) {
        std::cerr << "Uso: " << argv[0] << " <filename> <window size> <threshold> <alpha>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    k = std::stoi(argv[2]);
    threshold = std::stod(argv[3]);
    smoothing = std::stod(argv[4]);
    //DEBUG
    // std::string filename = "./teste.txt";
    // k = 3;
    // threshold = 10;
    // smoothing = 1;

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

    std::string charactersRead = "";

    for(int i = 0; i < k; i++) {
        charactersRead += 'A';
    }

    char buffer[bufferSize + 1];
    inputFile.read(buffer,bufferSize);
    int bufferLen = inputFile.gcount();

    while ((bufferLen = inputFile.gcount())) {
        buffer[bufferLen] = '\0';
        std::string s(buffer);
        charactersRead += s;        

        processString(charactersRead,charactersRead.size() - s.size());
        inputFile.read(buffer,bufferSize);
    } 

    inputFile.close();

    //TODO: escrever os bits no ficheiro
    return 0;
}