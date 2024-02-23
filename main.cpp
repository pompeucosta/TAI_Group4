#include <iostream>
#include <fstream>

#define k 4
#define bufferSize 100

void processString(const std::string& s) {

}

int main(int argc,char* argv[]) {
    std::string line;
    std::ifstream inputFile("teste.txt");

    if(!inputFile.is_open()) {
        std::cout << "Cannot open file!" << std::endl;
        return -1;
    }

    char buffer[bufferSize + 1];
    inputFile.read(buffer,bufferSize);
    buffer[inputFile.gcount()] = '\0';
    int bufferLen = 0;
    while((bufferLen = inputFile.gcount())) {
        std::string s(buffer);

        processString(s);
        inputFile.read(buffer,bufferSize);
        buffer[bufferLen] = '\0';
    }

    inputFile.close();
    return 0;
}