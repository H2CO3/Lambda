//
// lambda.cc
// A simple interpreter for the untyped lambda calculus
//
// Created by Arpad Goretity
// on 29/09/2016
//

#include <cassert>

#include "lambda.hh"
#include "parser.hh"


////////// lambda //////////
lambda::lambda(const lambda &other) noexcept {
    root = other.root->clone();
}

lambda::lambda(lambda &&other) noexcept {
    root = std::move(other.root);
}

lambda::lambda(const std::string &src) {
    root = parse(src);
}

std::string lambda::to_string() const noexcept {
    std::string str;
    str.reserve(256);
    root->to_string(&str);
    return str;
}

void lambda::reduce() {
    SubstCtx ctx;
    root = root->reduce(&ctx);
}

//////// SubstCtx ////////
SubstCtx::ShadowGuard SubstCtx::register_shadow(const std::string &parm_name) noexcept {
    shadow[parm_name]++;
    return ShadowGuard { this, &shadow.find(parm_name)->first };
}

SubstCtx::SubstGuard SubstCtx::register_substitution(const std::string *from, Node *to) noexcept {
    args.push_back({ from, to });
    return SubstGuard { this };
}

std::unique_ptr<Node> SubstCtx::reduced_node_for_substitution(const Subs *subs) noexcept {
    // iterate on stack in reverse direction:
    // innermost substitutions are the most recently added ones.
    for (auto it = args.rbegin(); it != args.rend(); it++) {
        if (subs->name == *it->from) {
            // if shadowing is in effect, do not perform substitution
            return shadow[subs->name] > 0 ? subs->clone() : it->to->reduce(this);
        }
    }

    return subs->clone();
}


////////// Node //////////
Node::Node() {}
Node::~Node() {}

////////// Func //////////
Func::Func(std::string _parm, std::unique_ptr<Node> _body) : parm(std::move(_parm)),
                                                             body(std::move(_body)) {}

std::unique_ptr<Node> Func::clone() const noexcept {
    return std::make_unique<Func>(parm, body->clone());
}

void Func::to_string(std::string *str) const noexcept {
    str->append("\\");
    str->append(parm);
    str->append(".");
    body->to_string(str);
}

std::unique_ptr<Node> Func::reduce(SubstCtx *ctx) const noexcept {
    // take note of the shadowing
    auto guard = ctx->register_shadow(parm);
    return std::make_unique<Func>(parm, body->reduce(ctx));
}

////////// Appl //////////
Appl::Appl(std::unique_ptr<Node> _func, std::unique_ptr<Node> _parm) : func(std::move(_func)),
                                                                       parm(std::move(_parm)) {}

std::unique_ptr<Node> Appl::clone() const noexcept {
    return std::make_unique<Appl>(func->clone(), parm->clone());
}

void Appl::to_string(std::string *str) const noexcept {
    str->append("(");
    func->to_string(str);
    str->append(" ");
    parm->to_string(str);
    str->append(")");
}

std::unique_ptr<Node> Appl::reduce(SubstCtx *ctx) const noexcept {
    // attempt to evaluate function
    auto resolved_func_uptr = func->reduce(ctx);
    auto resolved_func = dynamic_cast<Func *>(resolved_func_uptr.get());

    if (resolved_func == nullptr) {
        // LHS is not a function, so do not try substituting its argument.
        // Do reduce, however, the RHS.
        return std::make_unique<Appl>(
            std::move(resolved_func_uptr),
            parm->reduce(ctx)
        );
    }

    // substitute unevaluated argument for every occurrence of Subs(`name`)
    // in the transitive closure of the lambda body
    auto guard = ctx->register_substitution(&resolved_func->parm, parm.get());
    return resolved_func->body->reduce(ctx);
}

////////// Subs //////////
Subs::Subs(std::string _name) : name(std::move(_name)) {}

std::unique_ptr<Node> Subs::clone() const noexcept {
    return std::make_unique<Subs>(name);
}

void Subs::to_string(std::string *str) const noexcept {
    str->append(name);
}

std::unique_ptr<Node> Subs::reduce(SubstCtx *ctx) const noexcept {
    return ctx->reduced_node_for_substitution(this);
}
