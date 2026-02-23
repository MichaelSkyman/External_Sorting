#include <iostream>
#include <fstream>
#include <iomanip>

void readFirst100(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);

    if (!in) {
        std::cout << "Cannot open file: " << filename << "\n";
        return;
    }

    double x;
    int count = 0;

    std::cout << "File: " << filename << "\n";

    while (in.read(reinterpret_cast<char*>(&x), sizeof(double)) 
           && count < 100) {
        std::cout << std::fixed << std::setprecision(6) << x << "\n";
        count++;
    }

    std::cout << "---- End preview ----\n\n";
}

int main() {
    readFirst100("input.bin");
    readFirst100("output.bin");
    return 0;
}
