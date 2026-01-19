#include <lexer/lexer.hpp>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file>\n";
        return 1;
    }

    fs::path file_path = argv[1];
    if (!fs::exists(file_path)) {
        std::cerr << "File not found: " << file_path << "\n";
        return 1;
    }

    // 1. Get file size and allocate buffer
    auto file_size = fs::file_size(file_path);
    
    // We allocate file_size + 1 to hold the '\0' sentinel
    std::string source_code;
    source_code.reserve(file_size + 1);

    // 2. Read file content
    std::ifstream file(file_path, std::ios::binary);
    if (!file) {
        std::cerr << "Could not open file stream.\n";
        return 1;
    }

    source_code.assign((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    // 3. Manually add the null terminator (The Sentinel)
    // This ensures your character_map['\0'] logic works perfectly
    source_code.push_back('\0');

    // 4. Initialize Lexer
    // Note: We use the raw pointer from the string
    Util::Source source = Util::Source((unsigned char*)source_code.data(), source_code.size());
    Util::Lexer lexer(source);

    std::cout << "--- Analyzing: " << file_path.filename() << " ---\n";

    // 5. Execution Loop
    while (true) {
        Util::Token token = lexer.get_next_token();

        // Display Token Info
        std::string_view text(source_code.data() + token.offset, token.length);
        
        std::cout << "Type: " << static_cast<int>(token.token_type)
                  << " | Len: " << token.length
                  << " | Text: [" << text << "]\n";

        if (token.token_type == Util::TokenType::EndOfFile) {
            break;
        }

        if (token.token_type == Util::TokenType::Error) {
            std::cout << "!! Lexer Error at index " << token.offset << "\n";
            break;
        }
    }

    return 0;
} //ąąąąąąą bcććć