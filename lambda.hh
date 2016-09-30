//
// lambda.hh
// A simple interpreter for the untyped lambda calculus
//
// Created by Arpad Goretity
// on 29/09/2016
//

#ifndef LAMBDA_HH
#define LAMBDA_HH

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>


struct Node;
struct Subs;

struct lambda {
    lambda(const lambda &) noexcept;
    lambda(lambda &&) noexcept;
    lambda(const std::string &);

    std::string to_string() const noexcept;
    void reduce();

private:
    std::unique_ptr<Node> root;
};

struct SubstCtx {
private:
    // Represents a substitution operation
    struct SubstOp {
        const std::string *from;
        const Node *to;
    };

    // stack of arguments to be substituted.
    // Order matters because we always want to perform the innermost substitution.
    std::vector<SubstOp> args;

    // Set of shadowing function definitions.
    // Order does not matter, only the number of times
    // a parameter occurs in a manner that it shadows other names.
    std::unordered_map<std::string, std::size_t> shadow;

public:
    struct ShadowGuard {
        SubstCtx *ctx;
        const std::string *parm_name;

        ~ShadowGuard() {
            ctx->shadow[*parm_name]--;
        }
    };

    struct SubstGuard {
        SubstCtx *ctx;

        ~SubstGuard() {
            ctx->args.pop_back();
        }
    };

    ShadowGuard register_shadow(const std::string &parm_name) noexcept;
    SubstGuard register_substitution(const std::string *from, Node *to) noexcept;
    std::unique_ptr<Node> reduced_node_for_substitution(const Subs *subs) noexcept;
};

struct Node {
    Node();
    Node(const Node &) = delete;
    Node(Node &&) = delete;

    virtual ~Node();

    virtual std::unique_ptr<Node> clone() const noexcept = 0;

    virtual void to_string(std::string *) const noexcept = 0;

    virtual std::unique_ptr<Node> reduce(SubstCtx *) const noexcept = 0;

    virtual std::string type_name() const noexcept;
};

struct Func : Node {
    std::string parm;
    std::unique_ptr<Node> body;

    Func(std::string _parm, std::unique_ptr<Node> _body);

    virtual std::unique_ptr<Node> clone() const noexcept;

    virtual void to_string(std::string *) const noexcept;

    virtual std::unique_ptr<Node> reduce(SubstCtx *) const noexcept;
};

struct Appl : Node {
    std::unique_ptr<Node> func;
    std::unique_ptr<Node> parm;

    Appl(std::unique_ptr<Node> _func, std::unique_ptr<Node> _parm);

    virtual std::unique_ptr<Node> clone() const noexcept;

    virtual void to_string(std::string *) const noexcept;

    virtual std::unique_ptr<Node> reduce(SubstCtx *) const noexcept;
};

struct Subs : Node {
    std::string name;

    Subs(std::string _name);

    virtual std::unique_ptr<Node> clone() const noexcept;

    virtual void to_string(std::string *) const noexcept;

    virtual std::unique_ptr<Node> reduce(SubstCtx *) const noexcept;
};

#endif // LAMBDA_HH
