#include <iostream>
#include <fstream>
#include <unordered_map>
#include <math.h>
#include <set>
#include <algorithm>
#include <stdint.h>
#include <iomanip>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <filesystem>

typedef struct {
    const uint8_t repeatIteration = 1,fallbackIteration = 0;
    double fallbackBits = 0,repeatBits = 0;
    size_t fallbackCalls = 0,repeatCalls = 0;
    uint64_t elapsedTime = 0;
    std::vector<uint8_t> modelUsed;
    std::vector<double> bits;
} Statistics;

size_t hits = 0,misses = 0,alphabetSize = 0,k = 10,modelPointer = 0;
double totalBits = 0,threshold = 0.9,smoothing = 1;
std::unordered_map<std::string,size_t> posOfSequences;
std::string charactersRead("");
bool repeatModelStopped = false,statisticsEnable = false;

Statistics statistics;

void printHelp(const char* programName) {
    std::cout << "Usage: " << programName << " <input file> [-s] [-h] [-k <kmer size>] [-t <threshold>] [-a <alpha>]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -s               Enable statistics (Default: false)" << std::endl;
    std::cout << "  -k <value>       Set the size of the kmer (Default: 10)" << std::endl;
    std::cout << "  -t <value>       Set the threshold [0-1] (Default: 0.9)" << std::endl;
    std::cout << "  -a <value>       Set the value of alpha or smoothing (Default: 1)" << std::endl;
    std::cout << "  -h               Display this help message" << std::endl;
}

int getSizeOfAlphabet(std::ifstream& stream,size_t& charsRead) {
    std::set<char> alphabet; 
    charsRead = 0;
    const size_t bufferSize = 8000;
    char* buffer = new char[bufferSize + 1];
    stream.read(buffer,bufferSize);
    size_t bufferLen = 0;
    while((bufferLen = stream.gcount())) {
        charsRead += bufferLen;
        for(size_t i = 0; i < bufferLen; i++)
            alphabet.insert(buffer[i]);

        stream.read(buffer,bufferSize);
    }

    return alphabet.size();
}

void repeatPredict(char symbolAtModelPoint,char symbolToBePredicted) {
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

    totalBits += bits;
}

void fallbackPredict(char symbol) {
    const size_t fallbackWindowSize = 200;
    size_t len = std::min<size_t>(charactersRead.size(),fallbackWindowSize);

    std::unordered_map<char,uint16_t> charCount;
    for(size_t i = charactersRead.size() - len; i < charactersRead.size(); i++) {
        charCount[charactersRead[i]]++;
    }

    double totalBitsForSymbol = 0;
    double dLen = static_cast<double>(len);
    for(const auto& pair : charCount) {
        double division = pair.second / dLen;
        totalBitsForSymbol += -division * log2(division);
    }

    totalBits += totalBitsForSymbol;
}

bool shouldStopRepeatModel() {
    if((hits + misses) == 0)
        return false;
    
    return (static_cast<double>(hits) / (hits + misses)) < threshold;
}

void processString(int startPos) {
    if(charactersRead.length() < k) {
        return;
    }

    std::string kmer = "";
    for(size_t windowPointer = startPos; windowPointer < charactersRead.length(); windowPointer++, modelPointer++) {
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
        
        double bitsBeforePredict = totalBits;
        uint8_t model;
        if(repeatModelStopped) {
            model = statistics.fallbackIteration;
            fallbackPredict(charactersRead[windowPointer]);
        }
        else {
            model = statistics.repeatIteration;
            repeatPredict(charactersRead[modelPointer],charactersRead[windowPointer]);
            repeatModelStopped = shouldStopRepeatModel();
        }

        if(statisticsEnable) {
            statistics.modelUsed.push_back(model);

            double b = totalBits - bitsBeforePredict;
            statistics.bits.push_back(b);
            bool isFallback = model == statistics.fallbackIteration;

            statistics.fallbackBits += b * isFallback;
            statistics.fallbackCalls += isFallback;

            statistics.repeatBits += b * (!isFallback);
            statistics.repeatCalls += !isFallback;
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
    outputFile << "Average number of bits per symbol: " << totalBits / charactersRead.size() << "\n";
    outputFile.close();
}

void writeStatisticsToFile(std::ofstream& output,const Statistics& statistics) {
    output << "{\"elapsedTime\": " << statistics.elapsedTime << ",";
    output << "\"fallbackModelBits\": " << std::fixed << std::setprecision(5) << statistics.fallbackBits << ",";
    output << "\"fallbackModelCalls\": " << statistics.fallbackCalls << ",";
    output << "\"repeatModelBits\": " << std::fixed << std::setprecision(5) << statistics.repeatBits << ",";
    output << "\"repeatModelCalls\": " << statistics.repeatCalls << ",";
    output << "\"repeatIterationValue\": " << static_cast<unsigned int>(statistics.repeatIteration) << ",";
    output << "\"fallbackIterationValue\": " << static_cast<unsigned int>(statistics.fallbackIteration) << ",";
    output << "\"modelsUsed\": [";
    for(size_t i = 0; i < statistics.modelUsed.size(); i++) {
        output << static_cast<unsigned int>(statistics.modelUsed[i]);
        if(i != statistics.modelUsed.size() - 1) {
            output << ",";
        }
    }
    output << "],";
    output << "\"bitsCalculated\": [";
    for(size_t i = 0; i < statistics.bits.size(); i++) {
        output << std::fixed << std::setprecision(5) << statistics.bits[i];
        if(i != statistics.bits.size() - 1) {
            output << ",";
        }
    }
    output << "]";
    output << "}" << std::endl;
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
                statisticsEnable = true;
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
    size_t bufferSize = 8000;

    if(k > bufferSize)
        bufferSize = k;

    std::ifstream inputFile(filename);

    if(!inputFile.is_open()) {
        std::cerr << "Cannot open file!" << std::endl;
        return -1;
    }

    size_t numCharsInFile = 0;
    alphabetSize = getSizeOfAlphabet(inputFile,numCharsInFile);

    if(alphabetSize == 0) {
        std::cerr << "File is empty!" << std::endl;
        return -1;
    }

    if(statisticsEnable) {
        statistics.modelUsed.reserve(numCharsInFile);
        statistics.bits.reserve(numCharsInFile);
    }

    inputFile.clear();
    inputFile.seekg(0,std::ios::beg);

    for(size_t i = 0; i < k; i++) {
        charactersRead += 'A';
    }

    char* buffer = new char[bufferSize];
    size_t bufferLen;

    auto start = std::chrono::high_resolution_clock::now();
    while ((bufferLen = inputFile.readsome(buffer,bufferSize)) > 0) {
        charactersRead.append(buffer,bufferLen);        

        processString(charactersRead.size() - bufferLen);
    } 

    inputFile.close();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    statistics.elapsedTime = duration.count();

    std::filesystem::path filePath(filename);
    std::string fileNameWithoutExtension = filePath.stem().string();

    writeResultsToFile(fileNameWithoutExtension);

    if(statisticsEnable) {
        std::ofstream output(fileNameWithoutExtension + "_statistics.json"); 
        if(!output.is_open()) {
            std::cerr << "Error: Unable to create file " << filename + "_statistics.json" << std::endl;
        }
        else {
            writeStatisticsToFile(output,statistics);
        }
        output.close();
    }

    std::cout << "Finished in " << statistics.elapsedTime << " ms" << std::endl;    

    return 0;
}
