#include <iostream>
#include <fstream>
#include <unordered_map>
#include <optional>
#include <math.h>


#define smoothing 1.0
int k;
int bufferSize;
std::string processedString = "";
int hits = 0,misses = 0;
int pos = 0;    // valor inicial antes de ser conhecido o k
double prob = 0;

std::unordered_map<std::string,int> posOfSequences;
std::unordered_map<char,double> bitsToEncode;

std::optional<char> predict(std::string& s) {
    // if sequence is in hash table, return the next character
    if(posOfSequences.find(s) != posOfSequences.end()) {
        int pos = posOfSequences[s];
        return processedString[pos+1];
    }
    return {};    
}

void processString(const std::string& s) {
    if(s.length() < k) {
        return; // pensar no que fazer quando a string é menor que o tamanho da janela
    }

    std::string window = "";
    processedString += s.substr(0,s.length() - k);
    for(int start = 0; start + k < s.length(); start++) {
        window = s.substr(start,k);
        std::optional<char> c = predict(window);

        if (c.has_value()) {
            bool hit = s[start + k] == c.value();
            hits += hit;
            misses += !hit;
            prob = (hits + smoothing) / (hits + misses + 2 * smoothing);
            bitsToEncode[c.value()] = -log2(prob);
            std::cout << "Sequence: " << window << " Predicted: " << c.value_or("(no value)") << " Actual: " << s[start+k] << " Probability: " << prob << std::endl;
        }


        posOfSequences[window] = pos++;
        //std::cout << "Sequence: " << window << " Pos: " << pos-1 << std::endl;
    }
}

int main(int argc,char* argv[]) {

    // ./... textFIle k bufferSize

    if (argc != 4) {
        std::cerr << "Uso: " << argv[0] << " <filename> <window size> <bufferSize>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    k = std::stoi(argv[2]);
    bufferSize = std::stoi(argv[3]);
    pos = k - 1;    // atualizacao do pos

    std::ifstream inputFile(filename);
    
    if(!inputFile.is_open()) {
        std::cout << "Cannot open file!" << std::endl;
        return -1;
    }

    char buffer[bufferSize + 1];
    inputFile.read(buffer,bufferSize);
    int bufferLen = 0;
    while((bufferLen = inputFile.gcount())) {
        buffer[bufferLen] = '\0';
        std::string s(buffer);
        processString(s);

        //mete o ponteiro para tras para garantir que a ultima sequencia é processada
        //EXEMPLO
        //ficheiro: exemplomaisexemplo
        //k: 3
        //bufferSize: 5
        //buffer: exemp
        //primeira sequencia: exe => adivinha m
        //segunda sequencia: xem => adivinha p
        //nao faz mais nenhuma sequencia nesse buffer porque chega ao fim do buffer, sequencia seria emp
        //nesta altura o ponteiro do ficheiro esta no l
        //metendo para tras k vezes, ou seja, -3
        //o ponteiro passa a estar no e de emp
        //logo o buffer fica emplo em vez de lpoma
        //e assim a sequencia emp é processada em vez de ser descartada
        inputFile.seekg(-k,inputFile.cur);
        inputFile.read(buffer,bufferSize);
    }

    inputFile.close();
    return 0;
}