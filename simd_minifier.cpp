#include <bitset>
#include <chrono>
#include <cctype>
#include <fstream>
#include <immintrin.h>
#include <iomanip>
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

void print_m128i(std::string_view comment, __m128i v) {
    std::cout << comment << ": ";

    uint8_t* p = (uint8_t*)&v;
    for (int i = 0;i < 16;i++) {
        std::cout << std::setw(3) << (int)p[i] << ' ';
    }
    std::cout << '\n';
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Error: path to a JSON file is required\n";
        return 1;
    }

    std::ifstream ifs(argv[1]);
    std::stringstream r_buff;
    r_buff << ifs.rdbuf();
    std::string orig = r_buff.str();

    std::string path(argv[1]);
    size_t dot_index = path.rfind('.');
    std::string new_path = path.substr(0, dot_index) + ".min" + path.substr(dot_index);
    // is fwrite faster?
    std::ofstream ofs(new_path, std::ios::binary);

    Timer t;
    t.start();

    // To skip strings
    bool in_quotes = false;
    
    // for (int i = 0;i < 32;i += 32) {
    //     //std::cout << (int)orig[i] << ' ';

    //     //_mm256_lddqu_si256
    //     __m256i bytes = _mm256_loadu_si256((const __m256i_u*)(orig.c_str() + i));
    //     print_m256i(bytes);

    //     __m256i spaces = _mm256_set1_epi8(' ');
    //     //print_m256i(spaces);
 
    //     __m256i is_space = _mm256_cmpeq_epi8(bytes, spaces);
    //     //print_m256i(is_space);

    //     __m256i new_lines = _mm256_set1_epi8('\n');
    //     //print_m256i(new_lines);

    //     __m256i is_new_line = _mm256_cmpeq_epi8(bytes, new_lines);
    //     //print_m256i(is_new_line);

    //     __m256i is_whitespace = _mm256_or_si256(is_space, is_new_line);
    //     print_m256i(is_whitespace);

    //     int mask = _mm256_movemask_epi8(is_whitespace);
    //     std::cout << std::bitset<32>(mask) << '\n';
    // }

    // lookup table
    // is_graph_mask (8 bits) -> order (16 8-bit ints, first 8 ints empty)
    // 00000001 -> ........-------0    
    // 00000010 -> ........-------1
    // 00000011 -> ........------10
    // 00000100 -> ........-------2
    // 00000101 -> ........------20
    uint8_t lookup[256][16];
    for (int mask = 0;mask < 256;mask++) {
        for (int i = 0;i < 16;i++) {
            // 0x80 (bit 7 set) to zero out 
            lookup[mask][i] = 0x80;
        }

        for (int i = 0,j = 0;i < 8;i++) {
            if (!(mask & (1 << i))) continue;

            lookup[mask][j++] = i;
        }
    }

    // detecting quotes (ascii 34)
    // cases:
    //  not in quotes
    //  entire chunk in quotes that are not visible
    //  chunk partially in quotes
    //  one quotation mark (start or end)
    //  multiple starts/ends per chunk

    alignas(16) char w_buff[16];

    for (int i = 0;i < 128;i += 16) {
        __m128i chars = _mm_loadu_si128((const __m128i_u*)(orig.c_str() + i));
        print_m128i("chars", chars);

        // 9, 10, etc., are ASCII whitespaces
        __m128i is_whitespace = _mm_cmpeq_epi8(chars, _mm_set1_epi8(9));
            is_whitespace = _mm_or_si128(is_whitespace, _mm_cmpeq_epi8(chars, _mm_set1_epi8(10))),
            is_whitespace = _mm_or_si128(is_whitespace, _mm_cmpeq_epi8(chars, _mm_set1_epi8(13))),
            is_whitespace = _mm_or_si128(is_whitespace, _mm_cmpeq_epi8(chars, _mm_set1_epi8(32)));

        // Inverse, 1111 1111 xor 1001 1001 -> 0110 0110
        __m128i is_graph = _mm_xor_si128(_mm_set1_epi8(-1), is_whitespace);

        // Detecting quotes
        // Running carry/parity
        // x ^= x << 1
        // 00010010
        // 00100100 =
        // 00110110
        // << 2
        // 00110110
        // 11011000 =
        // 11101110
        // << 4
        // 11101110
        // 11100000 =
        // 00001110
        // 
        // Carry the state to the next chunk

        __m128i in_quotes = _mm_cmpeq_epi8(chars, _mm_set1_epi8(34));
        print_m128i("x", in_quotes);
        in_quotes = _mm_xor_si128(in_quotes, _mm_slli_si128(in_quotes, 1));
        print_m128i("x ^ (x << 1)", in_quotes);
        in_quotes = _mm_xor_si128(in_quotes, _mm_slli_si128(in_quotes, 2));
        print_m128i("x ^ (x << 2)", in_quotes);
        in_quotes = _mm_xor_si128(in_quotes, _mm_slli_si128(in_quotes, 4));
        print_m128i("x ^ (x << 4)", in_quotes);

        is_graph = _mm_or_si128(is_graph, in_quotes);
        print_m128i("is_graph", is_graph);

        unsigned int mask = _mm_movemask_epi8(is_graph),
            lo_mask = mask & 0xFF,
            hi_mask = mask >> 8;

        // Compress or pack left
        // 1111110000010001 -> 1111111100000000
        // Splitting because 2^16 lookup table doesn't fit in L1.

        __m128i lo_order = _mm_loadu_si128((const __m128i*)lookup[lo_mask]);
        __m128i hi_order = _mm_loadu_si128((const __m128i*)lookup[hi_mask]);
        hi_order = _mm_or_si128(hi_order, _mm_set1_epi8(8));

        __m128i lo = _mm_shuffle_epi8(chars, lo_order),
            hi = _mm_shuffle_epi8(chars, hi_order);

        // __m128i merged = _mm_unpackhi_epi64(lo, hi);
        // print_m128i("merged", merged);

        _mm_store_si128(reinterpret_cast<__m128i*>(w_buff), lo);
        ofs.write(w_buff, __builtin_popcount(lo_mask));

        _mm_store_si128(reinterpret_cast<__m128i*>(w_buff), hi);
        ofs.write(w_buff, __builtin_popcount(hi_mask));
    }

    ofs.write("\n", 1);

    ofs.close();

    //t.stop();

    std::cout << "Minified file written to " << new_path << '\n';
    return 0;
}