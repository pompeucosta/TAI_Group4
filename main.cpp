#include <iostream>
#include <fstream>
#include <unordered_map>
#include <math.h>
#include <set>
#include <algorithm>
#include <stdint.h>
#include <iomanip>
#include <unistd.h>

size_t hits = 0,misses = 0,alphabetSize = 0,k = 11,bufferSize = 8000,modelPointer = 0;
double totalBits = 0,threshold = 0.8,smoothing = 1;
std::unordered_map<std::string,size_t> posOfSequences;
std::string* charactersRead = new std::string("");
bool repeatModelStopped = false,statistics = false;

double fallbackBits = 0,repeatBits = 0;
size_t fallbackCalls = 0,repeatCalls = 0;

void printHelp(const char* programName) {
    std::cout << "Usage: " << programName << " <input file> [-s] [-h] [-k <kmer size>] [-t <threshold>] [-a <alpha>]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -s               Enable statistics (Default: false)" << std::endl;
    std::cout << "  -k <value>       Set the size of the kmer (Default: 11)" << std::endl;
    std::cout << "  -t <value>       Set the threshold [0-1] (Default: 0.8)" << std::endl;
    std::cout << "  -a <value>       Set the value of alpha or smoothing (Default: 1)" << std::endl;
    std::cout << "  -h               Display this help message" << std::endl;
}

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

    double probability = (hits + smoothing) / (hits + misses + 2 * smoothing);
    double bits;

    if(hit) {
        bits = -log2(probability);
    }
    else {
        bits = -log2((1 - probability) / (alphabetSize - 1));
    }

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
    bool fileFlag = false;
    int option;

    while((option = getopt(argc,argv,"k:t:a:sh")) != -1) {
        switch (option)
        {
            case 'k':
                try {
                    long x = std::stol(optarg);
                    if(x <= 0) {
                        std::cerr << "Error: Invalid value for -k option. Must be a positive integer" << std::endl;
                        exit(EXIT_FAILURE);
                    }
                    k = x;
                } catch (const std::invalid_argument& e) {
                    std::cerr << "Error: Invalid value for -k option. Must be an integer." << std::endl;
                    exit(EXIT_FAILURE);
                } catch (const std::out_of_range& e) {
                    std::cerr << "Error: Value for -k option out of range." << std::endl;
                    exit(EXIT_FAILURE);
                }
                break;
            case 't':
                try {
                    threshold = std::stod(optarg);
                    if(threshold < 0) {
                        std::cerr << "Error: Invalid value for -t option. Must be positive" << std::endl;
                        exit(EXIT_FAILURE);
                    }

                    if(threshold > 1) {
                        std::cerr << "Error: Invalid value for -t option. Must be between 0 and 1" << std::endl;
                        exit(EXIT_FAILURE);
                    }
                } catch (const std::invalid_argument& e) {
                    std::cerr << "Error: Invalid value for -t option. Must be a double." << std::endl;
                    exit(EXIT_FAILURE);
                } catch (const std::out_of_range& e) {
                    std::cerr << "Error: Value for -t option out of range." << std::endl;
                    exit(EXIT_FAILURE);
                }
                break;
            case 'a':
                try {
                    smoothing = std::stod(optarg);
                    if(smoothing < 0) {
                        std::cerr << "Error: Invalid value for -a option. Must be greater than 0" << std::endl;
                        exit(EXIT_FAILURE);
                    }
                } catch (const std::invalid_argument& e) {
                    std::cerr << "Error: Invalid value for -a option. Must be a double." << std::endl;
                    exit(EXIT_FAILURE);
                } catch (const std::out_of_range& e) {
                    std::cerr << "Error: Value for -a option out of range." << std::endl;
                    exit(EXIT_FAILURE);
                }
                break;
            case '?':
            case 'h':
                printHelp(argv[0]);
                exit(EXIT_SUCCESS);
                break;
            case 's':
                statistics = true;
                break;
            default:
                printHelp(argv[0]);
                exit(EXIT_SUCCESS);
                break;
        }
    }

    if(optind >= argc) {
        std::cerr << "Error: Please provide an input file" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string filename = argv[optind];

    if(k > bufferSize)
        bufferSize = k;

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