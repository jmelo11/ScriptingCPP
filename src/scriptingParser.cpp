
#include "scriptingParser.h"

#include <algorithm>
#include <cctype>

vector<Token> tokenize(const string &str)
{
        vector<Token> v;
        v.reserve(str.size());

        size_t i = 0;
        while (i < str.size())
        {
                char c = str[i];
                if (isspace(static_cast<unsigned char>(c)))
                {
                        ++i;
                        continue;
                }

                if (isdigit(static_cast<unsigned char>(c)) || (c == '.' && i + 1 < str.size() && isdigit(static_cast<unsigned char>(str[i + 1]))))
                {
                        size_t j = i;
                        while (j < str.size() && (isdigit(static_cast<unsigned char>(str[j])) || str[j] == '.'))
                                ++j;
                        v.emplace_back(stod(str.substr(i, j - i)));
                        i = j;
                }
                else if (isalpha(static_cast<unsigned char>(c)) || c == '_')
                {
                        size_t j = i;
                        while (j < str.size() && (isalnum(static_cast<unsigned char>(str[j])) || str[j] == '_'))
                                ++j;
                        string s = str.substr(i, j - i);
                        string up;
                        up.reserve(s.size());
                        for (char ch : s)
                                up.push_back(static_cast<char>(toupper(static_cast<unsigned char>(ch))));
                        Token::Type t = Token::Type::Identifier;
                        if (up == "IF" || up == "ELSE" || up == "END" || up == "FOR" || up == "IN" || up == "TO" || up == "PAYS")
                                t = Token::Type::Keyword;
                        v.emplace_back(up, t);
                        i = j;
                }
                else
                {
                        if (i + 1 < str.size())
                        {
                                string two = str.substr(i, 2);
                                if (two == ">=" || two == "<=" || two == "!=")
                                {
                                        v.emplace_back(two, Token::Type::Symbol);
                                        i += 2;
                                        continue;
                                }
                        }
                        v.emplace_back(c);
                        ++i;
                }
        }

        return v;
}

//      Event = vector<Statement>
Event parse(const string &eventString)
{
        Event e;

        auto tokens = tokenize(eventString);

        auto it = tokens.begin();
        while (it != tokens.end())
        {
                e.push_back(Parser<decltype(it)>::parseStatement(it, tokens.end()));
        }

        //      C++11 --> vectors are moved, not copied
        return e;
}

//      Single expression
Expression parseExpression(const string &exprString)
{
        auto tokens = tokenize(exprString);
        auto it = tokens.begin();
        return Parser<decltype(tokens.begin())>::parseStatement(it, tokens.end());
}
