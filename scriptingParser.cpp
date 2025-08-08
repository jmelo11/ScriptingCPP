#include "scriptingParser.h"

#include <cctype>
#include <stdexcept>

namespace {

std::vector<Token> lex(const std::string& src)
{
    std::vector<Token> tokens;
    for (size_t i = 0; i < src.size();) {
        char c = src[i];
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (c == '\n') tokens.push_back(Newline{});
            ++i;
        } else if (std::isalpha(static_cast<unsigned char>(c))) {
            size_t j = i + 1;
            while (j < src.size() && (std::isalnum(static_cast<unsigned char>(src[j])) || src[j] == '_')) ++j;
            std::string word = src.substr(i, j - i);
            if (word == "for") tokens.push_back(Keyword{Keyword::Type::For});
            else if (word == "in") tokens.push_back(Keyword{Keyword::Type::In});
            else if (word == "to") tokens.push_back(Keyword{Keyword::Type::To});
            else if (word == "end") tokens.push_back(Keyword{Keyword::Type::End});
            else if (word == "if") tokens.push_back(Keyword{Keyword::Type::If});
            else if (word == "then") tokens.push_back(Keyword{Keyword::Type::Then});
            else if (word == "else") tokens.push_back(Keyword{Keyword::Type::Else});
            else if (word == "endif") tokens.push_back(Keyword{Keyword::Type::EndIf});
            else tokens.push_back(Identifier{word});
            i = j;
        } else if (std::isdigit(static_cast<unsigned char>(c)) || c == '.') {
            size_t j = i + 1;
            while (j < src.size() && (std::isdigit(static_cast<unsigned char>(src[j])) || src[j] == '.')) ++j;
            double val = std::stod(src.substr(i, j - i));
            tokens.push_back(NumberToken{val});
            i = j;
        } else {
            tokens.push_back(Punctuation{c});
            ++i;
        }
    }
    return tokens;
}

class ParserImpl {
    using It = std::vector<Token>::const_iterator;
    It cur, end;

public:
    ParserImpl(const std::vector<Token>& toks) : cur(toks.begin()), end(toks.end()) {}

    Event parseEvent() {
        Event evt;
        skipNewlines();
        while (cur != end) {
            evt.push_back(parseStatement());
            skipNewlines();
        }
        return evt;
    }

private:
    void skipNewlines() { while (cur != end && std::holds_alternative<Newline>(*cur)) ++cur; }

    Statement parseStatement() {
        if (cur != end && std::holds_alternative<Keyword>(*cur)) {
            auto kw = std::get<Keyword>(*cur).type;
            if (kw == Keyword::Type::For) return parseFor();
            if (kw == Keyword::Type::If) return parseIf();
        }
        return parseAssign();
    }

    Statement parseAssign() {
        std::string name = expectIdentifier();
        expectPunct('=');
        ExprTree value = parseExpr();
        if (cur != end && std::holds_alternative<Newline>(*cur)) ++cur;
        auto node = std::make_unique<NodeAssign>();
        node->arguments.push_back(ExprTree(new NodeVar(name)));
        node->arguments.push_back(std::move(value));
        return node;
    }

    Statement parseIf() {
        expectKeyword(Keyword::Type::If);
        ExprTree lhs = parseExpr();
        if (!isPunct('>')) throw std::runtime_error("Expected '>'");
        ++cur;
        ExprTree rhs = parseExpr();
        auto cond = std::make_unique<NodeSup>();
        cond->arguments.push_back(std::move(lhs));
        cond->arguments.push_back(std::move(rhs));
        expectKeyword(Keyword::Type::Then);
        Statement thenStmt = parseStatement();
        expectKeyword(Keyword::Type::Else);
        Statement elseStmt = parseStatement();
        expectKeyword(Keyword::Type::EndIf);
        auto node = std::make_unique<NodeIf>();
        node->firstElse = 2;
        node->arguments.push_back(std::move(cond));
        node->arguments.push_back(std::move(thenStmt));
        node->arguments.push_back(std::move(elseStmt));
        return node;
    }

    Statement parseFor() {
        expectKeyword(Keyword::Type::For);
        std::string var = expectIdentifier();
        if (cur != end && std::holds_alternative<Keyword>(*cur) && std::get<Keyword>(*cur).type == Keyword::Type::In) {
            ++cur;
            ExprTree iterable = parseArray();
            expectPunct(':');
            expectNewline();
            auto body = parseBlock();
            auto node = std::make_unique<NodeFor>();
            node->arguments.push_back(ExprTree(new NodeVar(var)));
            node->arguments.push_back(std::move(iterable));
            for (auto& st : body) node->arguments.push_back(std::move(st));
            return node;
        } else {
            expectPunct('=');
            ExprTree startExpr = parseExpr();
            expectKeyword(Keyword::Type::To);
            ExprTree endExpr = parseExpr();
            auto* s = dynamic_cast<NodeConst*>(startExpr.get());
            auto* e = dynamic_cast<NodeConst*>(endExpr.get());
            if (!s || !e) throw std::runtime_error("Range bounds must be numbers");
            int start = static_cast<int>(s->constVal);
            int finish = static_cast<int>(e->constVal);
            auto list = std::make_unique<NodeList>();
            if (start <= finish)
                for (int i = start; i <= finish; ++i) list->arguments.push_back(ExprTree(new NodeConst(i)));
            else
                for (int i = start; i >= finish; --i) list->arguments.push_back(ExprTree(new NodeConst(i)));
            expectPunct(':');
            expectNewline();
            auto body = parseBlock();
            auto node = std::make_unique<NodeFor>();
            node->arguments.push_back(ExprTree(new NodeVar(var)));
            node->arguments.push_back(std::move(list));
            for (auto& st : body) node->arguments.push_back(std::move(st));
            return node;
        }
    }

    std::vector<Statement> parseBlock() {
        std::vector<Statement> body;
        skipNewlines();
        while (cur != end && !(std::holds_alternative<Keyword>(*cur) && std::get<Keyword>(*cur).type == Keyword::Type::End)) {
            body.push_back(parseStatement());
            skipNewlines();
        }
        expectKeyword(Keyword::Type::End);
        if (cur != end && std::holds_alternative<Newline>(*cur)) ++cur;
        return body;
    }

    ExprTree parseExpr() {
        ExprTree lhs = parseTerm();
        while (cur != end && (isPunct('+') || isPunct('-'))) {
            char op = std::get<Punctuation>(*cur).ch;
            ++cur;
            ExprTree rhs = parseTerm();
            ExprTree node = (op == '+') ? ExprTree(new NodeAdd()) : ExprTree(new NodeSub());
            node->arguments.push_back(std::move(lhs));
            node->arguments.push_back(std::move(rhs));
            lhs = std::move(node);
        }
        return lhs;
    }

    ExprTree parseTerm() {
        if (cur == end) throw std::runtime_error("Unexpected end of input");
        if (std::holds_alternative<NumberToken>(*cur)) {
            double v = std::get<NumberToken>(*cur).value;
            ++cur;
            return ExprTree(new NodeConst(v));
        }
        if (std::holds_alternative<Identifier>(*cur)) {
            std::string name = std::get<Identifier>(*cur).name;
            ++cur;
            return ExprTree(new NodeVar(name));
        }
        if (isPunct('[')) {
            return parseArray();
        }
        throw std::runtime_error("Invalid expression");
    }

    ExprTree parseArray() {
        expectPunct('[');
        auto lst = std::make_unique<NodeList>();
        if (!isPunct(']')) {
            while (true) {
                lst->arguments.push_back(parseExpr());
                if (isPunct(',')) { ++cur; continue; }
                break;
            }
        }
        expectPunct(']');
        return lst;
    }

    std::string expectIdentifier() {
        if (cur == end || !std::holds_alternative<Identifier>(*cur))
            throw std::runtime_error("Expected identifier");
        std::string name = std::get<Identifier>(*cur).name;
        ++cur;
        return name;
    }

    bool isPunct(char ch) const {
        return cur != end && std::holds_alternative<Punctuation>(*cur) && std::get<Punctuation>(*cur).ch == ch;
    }

    void expectPunct(char ch) {
        if (!isPunct(ch)) throw std::runtime_error(std::string("Expected '") + ch + "'");
        ++cur;
    }

    void expectKeyword(Keyword::Type k) {
        if (cur == end || !std::holds_alternative<Keyword>(*cur) || std::get<Keyword>(*cur).type != k)
            throw std::runtime_error("Expected keyword");
        ++cur;
    }

    void expectNewline() {
        if (cur == end || !std::holds_alternative<Newline>(*cur))
            throw std::runtime_error("Expected newline");
        ++cur;
    }
};

} // namespace

Event parse(const std::string& src)
{
    auto tokens = lex(src);
    ParserImpl p(tokens);
    return p.parseEvent();
}

