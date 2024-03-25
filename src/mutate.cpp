#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <set>
#include <filesystem>

// Função para ler um ficheiro e retornar o seu alfabeto
std::set<char> Alphabet(const std::string& inputFilename) {
    std::ifstream inputFile(inputFilename);
    std::set<char> alphabet;

    char symbol;
    while (inputFile.get(symbol)) {
        alphabet.insert(symbol);
    }
    inputFile.close();
    return alphabet;
}

std::string mutateFile(const std::string& inputFilename, double mutationProbability) {    
    std::ifstream inputFile(inputFilename, std::ios::binary);

    if (!inputFile.is_open() ) {
        std::cerr << "Erro ao abrir o ficheiro de entrada " << inputFilename << std::endl;
        return "";
    }

    // Dar nome ao ficheiro de saida
    std::filesystem::path filePath(inputFilename);
    std::string fileNameWithoutExtension = filePath.stem().string();
    std::string outputFilename = fileNameWithoutExtension + "_mutated.txt";


    // Abrir o ficheiro de saída para escrita
    std::ofstream outputFile(outputFilename);
    if (!outputFile.is_open()) {
        std::cerr << "Erro ao criar o ficheiro de saída "<< outputFilename << std::endl;
        return "";
    }

    // Seed para gerar números aleatórios
    std::srand(std::time(nullptr));


    // Obter o alfabeto do ficheiro de entrada
    std::set<char> alphabet = Alphabet(inputFilename);

    char symbol, mutatedSymbol;

    while (inputFile.get(symbol)) {
        mutatedSymbol = symbol;
        // Verifica se o símbolo deve ser mutado ou não
        if ((double)std::rand() / RAND_MAX <= mutationProbability) {
            while(mutatedSymbol == symbol){
                mutatedSymbol = *std::next(alphabet.begin(), std::rand() % alphabet.size());
            }
        }
        outputFile.put(mutatedSymbol);
    }

    inputFile.close();
    outputFile.close();

    std::cout << "Ficheiro mutado com sucesso: " << outputFilename << std::endl;
    
    return outputFilename;
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_filename> <mutation_probability>" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string inputFilename = argv[1];
    double mutationProbability;
    try{
        mutationProbability = std::stod(argv[2]);
        if(mutationProbability < 0 || mutationProbability > 1) {
            std::cerr << "Error: Probability must be between 0 and 1 -> [0..1]" << std::endl;
            exit(EXIT_FAILURE);
        }
    }catch(const std::invalid_argument& e) {
        std::cerr << "Error: Probability must be a decimal value" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string outputFilename = mutateFile(inputFilename, mutationProbability);

    if (outputFilename.empty()) {
        std::cerr << "Error: Failed to create file" << std::endl;
        exit(EXIT_FAILURE);
    } 

    return 0;
}