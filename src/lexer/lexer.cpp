#include <lexer/lexer.hpp>
#include <string>

#include <array>

/*
As a clarification here, the number parser here will be dumb and no number
validation will be performed here. It will be done at an AST level, because the number type might here be actually inside 
of a string or a comment and this makes the error irrelevant
*/

namespace Util { 
 
 inline bool is_numeric_char(char numeric_char)
 {
    return numeric_char >= '0' && numeric_char <= '9';
 };
 
 inline bool is_letter_char(char letter_char)
 {
    return (letter_char >= 'A' && letter_char <= 'Z') || (letter_char >= 'a' && letter_char <= 'z');
 };

 inline bool is_special_char(char special_char)
 {
    return ((special_char >= '!' && special_char <= '~') && !is_numeric_char(special_char) && !is_letter_char(special_char));
 };

 inline bool is_whitespace_char(char whitespace_char)
 {
    return whitespace_char == ' ' || whitespace_char == '\t' || whitespace_char == '\r' || whitespace_char == '\n';
 };

 inline bool is_valid_char(char unknown_char)
 {
    return (unknown_char >= ' ' && unknown_char <= '~') || is_whitespace_char(unknown_char) || unknown_char == '\0';
 };

 enum class CharacterType : uint8_t {
   Error,
   Letter,
   Numeric,
   Whitespace,
   EndOfFile,
   Symbol, //synonymous to special character
 };

 consteval auto character_map = [](){
   std::array<CharacterType,256> character_map;

   for (size_t current_character = 0;current_character <= 255;current_character++)
   {
      if(!is_valid_char(current_character))
      {
         character_map[current_character] = CharacterType::Error;
         continue;
      }
      else if (is_numeric_char(current_character))
      {
         character_map[current_character] = CharacterType::Numeric;
         continue;
      }
      else if (is_letter_char(current_character))
      {
         character_map[current_character] = CharacterType::Letter;      
         continue;
      }
      else if (is_whitespace_char(current_character))
      {
         character_map[current_character] = CharacterType::Whitespace;
         continue;
      }
      character_map[current_character] = CharacterType::Symbol;
   };

   character_map['\0'] = CharacterType::EndOfFile;

   return character_map;
 }();

 char to_lower_case(char ascii_char)
 {
    const auto offset = 'a' - 'A';
    if (ascii_char >= 'A' && ascii_char <= 'Z')
    {
        return ascii_char + offset;
    }
    return ascii_char;
 };
 
 TokenType get_token_type(LexerContext& lexer_context)
 {
   auto current_char = static_cast<unsigned char>(lexer_context.source.see_current());
   auto character_type = character_map[current_char];

   switch (character_type)
   {
      case CharacterType::Error:
        return TokenType::Error;
      case CharacterType::Letter:
         return TokenType::Identifier;
      case CharacterType::Numeric:
         return TokenType::Numeric;
      case CharacterType::Symbol:
         return TokenType::SpecialChar;
      case CharacterType::Whitespace:
         return TokenType::Whitespace;
      case CharacterType::EndOfFile:
         return TokenType::EndOfFile;
      default:
        return TokenType::Error;
   };

   return TokenType::Error;
 };

 void consume_eof_token(LexerContext& lexer_context)
 {
   Assert(
      lexer_context.source.see_current() == '\0',
      LexerError
      "Undefined behaviour: No EOF Token when expected one."
      LexerErrorEnd
   )
   lexer_context.source.consume();
 };

 void consume_error_token(LexerContext& lexer_context)
 {
   lexer_context.emit_error(ErrorCode::UnexpectedCharacter); //potentially I could add a message, but it can be derived from token of error type and displayed for the user
   lexer_context.source.consume(); 
 };

 void consume_identifier_token(LexerContext& lexer_context)
 {

 };

 void consume_numeric_token(LexerContext& lexer_context)
 {

 };

 void consume_special_token(LexerContext& lexer_context)
 {

 };

 void consume_whitespace_token(LexerContext& lexer_context)
 {

 };

 Token Lexer::get_next_token()
 {
   auto token_type = get_token_type(lexer_context);
   size_t start = lexer_context.source.index;

   switch (token_type)
   {
   case TokenType::EndOfFile:
      consume_eof_token(lexer_context);
      break;
   case TokenType::Error:
      consume_error_token(lexer_context);
      break;
   case TokenType::Identifier:
      consume_identifier_token(lexer_context);
      break;
   case TokenType::Numeric:
      consume_numeric_token(lexer_context);
      break;
   case TokenType::SpecialChar:
      consume_special_token(lexer_context);
      break;
   case TokenType::Whitespace:
      consume_whitespace_token(lexer_context);
      break;
   default:
      consume_error_token(lexer_context);
      break;
   }

   size_t end = lexer_context.source.index;
   size_t length = end - start;
   Token token;
   token.token_type = token_type;
   token.offset = start;
   token.length = length;

   return token;
 };
}

#undef LexerError
#undef LexerErrorEnd