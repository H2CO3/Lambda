//
// parser.hh
// A simple interpreter for the untyped lambda calculus
//
// Created by Arpad Goretity
// on 29/09/2016
//

#ifndef LAMBDA_PARSER_HH
#define LAMBDA_PARSER_HH

#include <memory>


struct Node;

std::unique_ptr<Node> parse(const std::string &src);

#endif // LAMBDA_PARSER_HH
