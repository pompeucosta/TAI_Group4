#include <iostream>
#include <fstream>

#define k 4
#define bufferSize 100

void processString(const std::string& s) {

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