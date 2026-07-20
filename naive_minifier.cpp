#include <chrono>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>

class Timer {
    std::chrono::high_resolution_clock::time_point start_time;

public:
    void start() {
        start_time = std::chrono::high_resolution_clock::now();
    }

    void stop() {
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Duration: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time).count() << "ms" << std::endl;
    }
};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Error: path to a JSON file is required\n";
        return 1;
    }

    std::ifstream ifs(argv[1]);
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    std::string orig = buffer.str();

    std::string path(argv[1]);
    size_t dot_index = path.rfind('.');
    std::string new_path = path.substr(0, dot_index) + ".min" + path.substr(dot_index);
    std::ofstream ofs(new_path);

    Timer t;
    t.start();

    // To skip strings
    bool in_quotes = false;

    for (int i = 0;i < orig.length();i++) {
        if (!in_quotes && isspace(orig[i])) continue;

        if (orig[i] == '"') in_quotes = !in_quotes;

        ofs << orig[i];
    }

    t.stop();

    std::cout << "Minified file written to " << new_path << '\n';
    return 0;
}