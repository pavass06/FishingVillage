#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

template<typename T>
void parseKeyword(std::ifstream& file, const std::string& keyword, T& var)
{
    file.clear();        // Clear any EOF or error flags
    file.seekg(0);       // Go back to the beginning

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string word;
        iss >> word;

        if (word == keyword) {
            T value;
            iss >> value;
            if (!iss.fail()) {
                var = value;
            } else {
                std::cerr << "Failed to parse value for keyword '" << keyword << "'\n";
            }
            return; // Stop after finding the first match
        }
    }
}

