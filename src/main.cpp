
#include <iostream>
#include <string>
#include <vector>
#include <lexer/lexer.hpp> // Include your lexer header

using namespace Util;

// Helper function to run a single test
void run_test(const std::string& test_name, const std::string& input) {
    std::cout << "=== " << test_name << " ===" << std::endl;
    std::cout << "Input: \"" << input << "\"" << std::endl;

    // Prepare source buffer
    std::vector<unsigned char> buffer(input.begin(), input.end());

    Source source(buffer.data(), buffer.size());
    Lexer lexer(source);

    // Tokenize until EOF
    while (true) {
        Token token = lexer.get_next_token();
        if (token.token_type == TokenType::EndOfFile) break;
        std::cout << "Token: Type=" << static_cast<int>(token.token_type)
                  << ", Offset=" << token.offset
                  << ", Length=" << token.length 
                  << ", Text='" << std::string_view((const char*)buffer.data() + token.offset,token.length) << "'" << std::endl;

        if (token.token_type == TokenType::Error)
        {
            std::cout << "Error occured" << std::endl; 
        };
    }

    std::cout << std::endl;
}

int main() {
    // Test Cases
    run_test("TC-01: Basic tokenization", "int x = 10;");
    run_test("TC-02: Keyword vs identifier", "if iffy");
    run_test("TC-03: Operators", "a+b*c");
    run_test("TC-04: String literal handling", "\"Hello\"");
    run_test("TC-05: Numeric edge cases", "123 0xFF");
    run_test("TC-06: Unicode handling", "ąęć");
    run_test("TC-07: Error handling", "@invalid");
    run_test("TC-08: Whitespace and comments", "int x; // comment");

    return 0;
}
