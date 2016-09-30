//
// parser.cc
// A simple interpreter for the untyped lambda calculus
//
// Created by Arpad Goretity
// on 29/09/2016
//

#include <string>
#include <cassert>

#include "parser.hh"
#include "lambda.hh"


// just because we have state.
// (but this class need not be exposed at all.)
// Look ma! No lexer! (thanks spec)
struct Parser {
    const char *ptr;
    const char *end;

    std::unique_ptr<Node> parse() {
        assert(ptr < end);

        switch (*ptr) {
        case '\\':
            return parse_func();
        case '(':
            return parse_appl();
        default:
            return parse_subs();
        }
    }

private:
    std::unique_ptr<Func> parse_func() {
        assert(ptr < end && *ptr == '\\');

        // skip '\'
        ptr++;

        assert(isalpha(*ptr));
        const char *name_begin = ptr;

        while (isalpha(*ptr)) {
            ptr++;
        }

        std::string param(name_begin, ptr);

        assert(*ptr == '.');

        // skip '.'
        ptr++;

        // parse body
        auto body = parse();

        return std::make_unique<Func>(std::move(param), std::move(body));
    }

    std::unique_ptr<Appl> parse_appl() {
        assert(ptr < end && *ptr == '(');

        // skip opening parenthesis
        ptr++;

        // parse function
        auto func = parse();

        // skip separating WS
        assert(isspace(*ptr));
        while (isspace(*ptr)) {
            ptr++;
        }

        // parse argument
        auto arg = parse();

        assert(*ptr == ')');

        // skip closing parenthesis
        ptr++;

        return std::make_unique<Appl>(std::move(func), std::move(arg));
    }

    std::unique_ptr<Subs> parse_subs() {
        assert(ptr < end && isalpha(*ptr));

        const char *begin = ptr;

        while (ptr < end && isalpha(*ptr)) {
            ptr++;
        }

        return std::make_unique<Subs>(std::string(begin, ptr));
    }
};

std::unique_ptr<Node> parse(const std::string &src)
{
    Parser p { src.c_str(), src.c_str() + src.size() };
    return p.parse();
}
