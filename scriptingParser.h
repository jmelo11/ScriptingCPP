#pragma once

#include <string>
#include <variant>
#include <vector>

#include "visitorHeaders.h"

// Token definitions ---------------------------------------------------------

struct Identifier { std::string name; };
struct NumberToken { double value; };
struct Punctuation { char ch; };
struct Keyword { enum class Type { For, In, To, End, If, Then, Else, EndIf } type; };
struct Newline {};

using Token = std::variant<Identifier, NumberToken, Punctuation, Keyword, Newline>;

// Parse a script into an event composed of scripting nodes
Event parse(const std::string& src);

