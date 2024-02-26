#include <iostream>
#include <fstream>
#include <unordered_map>

#define k 3
#define bufferSize 5
#define smoothing 1.0

std::string processedString = "";
int hits = 0,misses = 0;
double prob = 0;

//TODO: criar a hash table e inserir as sequencias la dentro juntamente com a ultima posicao dessa sequencia
std::unordered_map<std::string,int> hashTable;

char predict(std::string& s) {
    // if sequence is in hash table, return the next character
    if(hashTable.find(s) != hashTable.end()) {
        int pos = hashTable[s];
        return processedString[pos+1];
    }
    return ' ';    
}

void processString(const std::string& s) {
    if(s.length() < k) {
        return; // pensar no que fazer quando a string é menor que o tamanho da janela
    }

    std::string window = "";
    processedString += s.substr(0,s.length() - k);
    for(int start = 0; start + k < s.length(); start++) {
        window = s.substr(start,k);
        char c = predict(window);
        bool hit = s[start + k] == c;

        if (c != ' ') {
            hits += hit;
            misses += !hit;
        }

        prob = (hits + smoothing) / (hits + misses + 2 * smoothing);
        std::cout << "Sequence: " << window << " Predicted: " << c << " Actual: " << s[start+k] << " Hit: " << hit << " Probability: " << prob << std::endl;

        std::cout << processedString << std::endl;
        int pos = processedString.length() + start;
        hashTable[window] = pos;
        //std::cout << "Sequence: " << window << " Pos: " << pos << std::endl;
    }
}

int main(int argc,char* argv[]) {
    std::ifstream inputFile("teste.txt");

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