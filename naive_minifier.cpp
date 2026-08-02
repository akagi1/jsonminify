#include <chrono>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Error: path to a JSON file is required\n";
        return 1;
    }

    std::ifstream ifs(argv[1]);
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    std::string orig = buffer.str();

    std::string output;
    output.reserve(orig.length());

    auto start = std::chrono::steady_clock::now();

    bool in_quotes = false;

    for (int i = 0;i < orig.length();i++) {
        if (!in_quotes && isspace(orig[i])) continue;

        if (orig[i] == '"') in_quotes = !in_quotes;

        output += orig[i];
    }

    auto end = std::chrono::steady_clock::now();
    double seconds = std::chrono::duration<double>(end - start).count();
    std::cout << "Time: " << seconds << "s\n"
              << "Throughput: " << (orig.size() / 1e9) / seconds << " GB/s\n";

    std::string path(argv[1]);
    size_t dot_index = path.rfind('.');
    std::string new_path = path.substr(0, dot_index) + ".min" + path.substr(dot_index);
    std::ofstream ofs(new_path);
    ofs << output;
    ofs.close();
    std::cout << "Minified file written to " << new_path << '\n';

    return 0;
}