#include <lexer/lexer.hpp>
#include <string>

/*
As a clarification here, the number parser here will be dumb and no number
validation will be performed here. It will be done at an AST level, because the number type might here be actually inside 
of a string or a comment and this makes the error irrelevant
*/

namespace Util { 
 bool is_numeric_char(char numeric_char)
 {
    return numeric_char >= '0' && numeric_char <= '9';
 };
 bool is_letter_char(char letter_char)
 {
    return (letter_char >= 'A' && letter_char <= 'Z') || (letter_char >= 'a' && letter_char <= 'z') || letter_char == '_';
 };

 bool is_special_char(char special_char)
 {
    return ((special_char >= '!' && special_char <= '~') && !is_numeric_char(special_char) && !is_letter_char(special_char)) && special_char != '_' || special_char == '\n';
 };

 bool is_whitespace_char(char whitespace_char)
 {
    return whitespace_char == ' ' || whitespace_char == '\t';
 };

 bool is_valid_char(char unknown_char)
 {
    return (unknown_char > ' ' && unknown_char < '~') || unknown_char == '\0';
 };

 char to_lower_case(char ascii_char)
 {
    const auto offset = 'a' - 'A';
    if (ascii_char >= 'A' && ascii_char <= 'Z')
    {
        return ascii_char + offset;
    }
    return ascii_char;
 };

 Token parse_special_token(Source& source)
 {
    /*
        This case is simple because all special token types must be 1 character long, otherwise they are identifiers or errors
    */ 
   Assert(
    source.can_consume_sentinel(),
    LexerError 
    "Unexpected behaviour. Can't read special characters outside of the bounds of "
    LexerErrorEnd
   )
   
   Token token;
   token.token_type = TokenType::SpecialChar;
  
   if (!is_special_char(source.see_current()))
   {
    token.token_type = TokenType::Error;  
   };

   source.consume();
 };

 Token parse_identifier_token(Source& source)
 {
    
 };

 void consume_numbers(Source& source)
 {
    auto current_char = source.see_current();
    while (is_numeric_char(current_char)) 
    {
        source.consume();
        current_char = source.see_current();
    }; 
 };

 void consume_until_scientific_notation_end(Source& source)
 {
    Assert(
        source.can_consume_sentinel(),
        LexerError
        "Unexpected behaviour: Expression has "
        LexerErrorEnd
    )
    /*
        should give an error about malformed number I guess or return early really and the AST builder should detect malformed number
        but in debug mode it should be enabled nevertheless I guess so
    */
    if (!source.can_consume_sentinel())
    {
        return;
    };

    Assert(to_lower_case(source.see_current()) == 'e',
        LexerError
        "Unexpected behaviour: "
        LexerErrorEnd
    )

    if (to_lower_case(source.see_current()) != 'e')
    {
        return;
    };

    source.consume(); 
    auto current_char = source.see_current();  

    if (current_char == '-' || current_char == '+')
    {
        source.consume();
    };

    if (is_numeric_char(current_char))
    {
        consume_numbers(source);
    };
 };

 void parse_number_hex_format(Source& source)
 {
    AssertEq(source.see_current(),'0');
    source.consume(); //0
    AssertEq(to_lower_case(source.see_current()),'x');
    source.consume(); //x
    consume_numbers(source);
 };

 void parse_number_bin_format(Source& source)
 {
    AssertEq(source.see_current(),'0');
    source.consume(); //0
    AssertEq(to_lower_case(source.see_current()),'b');
    source.consume(); //b
    consume_numbers(source);
 };

 void parse_fractions(Source& source)
 {
    //technically 1.235e is a valid number here but AST should be validating not lexer
    size_t start = source.index;
    source.consume();
    consume_numbers(source); 

    auto current_char = source.see_current();

    if (to_lower_case(current_char) == 'e')
    {
        consume_until_scientific_notation_end(source);
    };
 };

 bool parse_number_normal(Source& source)
 {
    bool is_valid = consume_numbers(source);

    if (!is_valid)
    {
        return false;
    };

    auto current_char = source.see_current();

    if (to_lower_case(current_char) == 'e')
    {
        return consume_until_scientific_notation_end(source);
    }
    else if (current_char == '.')
    {
        return parse_fractions(source);
    };

    return false;
 };

 Token parse_number_token(Source& source)
 {
    size_t start = source.index;
    Token token;
    token.token_type = TokenType::Numeric;

    /*First determine initial format and then load that number according to the format*/
    auto current_char = source.see_current();

    if (current_char == '0' && source.can_peek())
    {
        auto next_char = to_lower_case(source.peek());
        if (next_char == 'x')
        {
            parse_number_hex_format(source);
        } else if (next_char == 'b')
        {
            parse_number_bin_format(source);
        };
    } else if (current_char == '.')
    {
        parse_fractions(source);
    } else if (is_numeric_char(current_char)){
        parse_number_normal(source);
    } else{
        Assert(false,
            LexerError
            "Unexpected Behaviour: Number type was given, but the parsing function couldn't confirm that."
            LexerErrorEnd
        );
    };

    return token;
 };

 Token parse_eof_token(Source& source)
 {
    Assert(
        source.see_current() == '\0',
        LexerError
        "EOF Token Not Found"
        LexerErrorEnd
    )

    Token token;
    token.token_type = TokenType::EndOfFile;

    return token;
 };

 Token parse_whitespace_token(Source& source)
 {
    Token token;
    token.token_type = TokenType::Whitespace;
    auto current_char = source.see_current();
    while (is_whitespace_char(current_char))
    {
        token.length++;
        source.consume();
    };  

    return token;
 };

 Token parse_error_token(Source& source)
 {
    Assert(
        !source.can_consume_sentinel(),
        LexerError 
        "Attempting to read outside of bounds of the source"
        LexerErrorEnd
    )

    Token token;
    token.token_type = TokenType::Error;

    return token;
 };

 TokenType guess_token_type(Source& source)
 {
    auto current_char = source.see_current();

    if (!is_valid_char(current_char))
    {
        return TokenType::Error;
    };

    if (current_char == '\0')
    {
        return TokenType::EndOfFile;
    };

    if (current_char == '.')
    {
        if (!source.can_peek())
        {
            return TokenType::SpecialChar;
        };
        
        auto next_token = source.peek(); 
        if (is_numeric_char(next_token))
        {
            return TokenType::Numeric;
        };
    };

    if (is_special_char(current_char))
    {
        return TokenType::SpecialChar;
    }
    else if (is_numeric_char(current_char))
    {
        return TokenType::Numeric;
    } else if(is_letter_char(current_char))
    {
        return TokenType::Identifier;
    } else if (is_whitespace_char(current_char))
    {
        return TokenType::Whitespace;
    };

    return TokenType::Error;
 };

 Token Lexer::get_next_token()
 {
  auto token_type = guess_token_type(source);
  Token current_token;
  size_t start = source.index;

  switch (token_type)
  {
  case TokenType::SpecialChar:
    current_token = parse_special_token(source);
    break;
  case TokenType::Identifier:
    current_token = parse_identifier_token(source);
    break;
  case TokenType::Numeric:
    current_token = parse_number_token(source);
    break;
  case TokenType::Whitespace:
    current_token = parse_whitespace_token(source);
    break;
  case TokenType::EndOfFile:
    current_token = parse_eof_token(source);
    break;
  default:
    current_token = parse_error_token(source);
  };

  if (current_token.token_type == TokenType::Error)
  {
    //emit_error(ErrorCode::UnexpectedCharacter); would make sense if there were more non-critical error codes than 1 (UnexpectedCharacter)
  }

  size_t end = source.index;
  size_t length = end - start;

  current_token.offset = start;
  current_token.length = length;

  return current_token;
 };
};

#undef LexerError
#undef LexerErrorEnd