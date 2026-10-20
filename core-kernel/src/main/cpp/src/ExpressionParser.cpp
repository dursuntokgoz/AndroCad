// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#include "ExpressionParser.h"

#include <cmath>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <unordered_set>

namespace androcad {

namespace {

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E 2.71828182845904523536
#endif

bool IsIdentStart(char c) {
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

bool IsIdentChar(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

} // namespace

void ExpressionParser::SetVariable(const std::string& name, double value) {
    variables[name] = value;
}

bool ExpressionParser::HasVariable(const std::string& name) const {
    return variables.find(name) != variables.end();
}

double ExpressionParser::GetVariable(const std::string& name) const {
    auto it = variables.find(name);
    return it != variables.end() ? it->second : 0.0;
}

EvaluationResult ExpressionParser::Evaluate(const std::string& expression) const {
    if (expression.empty()) {
        return EvaluationResult{false, 0.0, "Empty expression."};
    }

    ParserState state{expression, 0, variables};
    EvaluationResult res = state.ParseExpression();

    if (res.success) {
        state.SkipWhitespace();
        if (state.pos < state.expr.size()) {
            return EvaluationResult{false, 0.0, "Unexpected trailing characters: " + state.expr.substr(state.pos)};
        }
    }

    return res;
}

char ExpressionParser::ParserState::Peek() const {
    if (pos >= expr.size()) return '\0';
    return expr[pos];
}

char ExpressionParser::ParserState::Get() {
    if (pos >= expr.size()) return '\0';
    return expr[pos++];
}

void ExpressionParser::ParserState::SkipWhitespace() {
    while (pos < expr.size() && std::isspace(static_cast<unsigned char>(expr[pos]))) {
        pos++;
    }
}

bool ExpressionParser::ParserState::Match(char expected) {
    SkipWhitespace();
    if (Peek() == expected) {
        pos++;
        return true;
    }
    return false;
}

EvaluationResult ExpressionParser::ParserState::ParseExpression() {
    EvaluationResult left = ParseTerm();
    if (!left.success) return left;

    while (true) {
        SkipWhitespace();
        char op = Peek();
        if (op == '+' || op == '-') {
            Get();
            EvaluationResult right = ParseTerm();
            if (!right.success) return right;

            if (op == '+') {
                left.value += right.value;
            } else {
                left.value -= right.value;
            }
        } else {
            break;
        }
    }
    return left;
}

EvaluationResult ExpressionParser::ParserState::ParseTerm() {
    EvaluationResult left = ParseFactor();
    if (!left.success) return left;

    while (true) {
        SkipWhitespace();
        char op = Peek();
        if (op == '*' || op == '/') {
            Get();
            EvaluationResult right = ParseFactor();
            if (!right.success) return right;

            if (op == '*') {
                left.value *= right.value;
            } else {
                if (std::abs(right.value) < 1e-15) {
                    return EvaluationResult{false, 0.0, "Division by zero."};
                }
                left.value /= right.value;
            }
        } else {
            break;
        }
    }
    return left;
}

EvaluationResult ExpressionParser::ParserState::ParseFactor() {
    SkipWhitespace();
    if (Match('+')) {
        return ParseFactor();
    }
    if (Match('-')) {
        EvaluationResult res = ParseFactor();
        if (!res.success) return res;
        res.value = -res.value;
        return res;
    }

    EvaluationResult left = ParsePrimary();
    if (!left.success) return left;

    SkipWhitespace();
    if (Match('^')) {
        EvaluationResult exponent = ParseFactor(); // Right-associative
        if (!exponent.success) return exponent;
        left.value = std::pow(left.value, exponent.value);
    }

    return left;
}

EvaluationResult ExpressionParser::ParserState::ParsePrimary() {
    SkipWhitespace();
    char c = Peek();

    if (c == '(') {
        Get(); // Consume '('
        EvaluationResult inner = ParseExpression();
        if (!inner.success) return inner;
        SkipWhitespace();
        if (!Match(')')) {
            return EvaluationResult{false, 0.0, "Missing closing parenthesis ')'"};
        }
        return inner;
    }

    // Number literal
    if (std::isdigit(static_cast<unsigned char>(c)) || c == '.') {
        size_t start = pos;
        bool hasDot = false;
        while (pos < expr.size()) {
            char ch = expr[pos];
            if (std::isdigit(static_cast<unsigned char>(ch))) {
                pos++;
            } else if (ch == '.' && !hasDot) {
                hasDot = true;
                pos++;
            } else {
                break;
            }
        }
        std::string numStr = expr.substr(start, pos - start);
        try {
            double val = std::stod(numStr);
            return EvaluationResult{true, val, ""};
        } catch (...) {
            return EvaluationResult{false, 0.0, "Invalid number literal: " + numStr};
        }
    }

    // Identifier: function or variable or constant
    if (IsIdentStart(c)) {
        size_t start = pos;
        while (pos < expr.size() && IsIdentChar(expr[pos])) {
            pos++;
        }
        std::string ident = expr.substr(start, pos - start);

        // Check standard mathematical constants
        if (ident == "PI" || ident == "pi") return EvaluationResult{true, M_PI, ""};
        if (ident == "E" || ident == "e") return EvaluationResult{true, M_E, ""};

        SkipWhitespace();
        if (Peek() == '(') {
            Get(); // Consume '('
            EvaluationResult arg = ParseExpression();
            if (!arg.success) return arg;

            // Handle two-argument functions (min, max)
            double secondArg = 0.0;
            bool hasSecondArg = false;
            SkipWhitespace();
            if (Match(',')) {
                EvaluationResult arg2 = ParseExpression();
                if (!arg2.success) return arg2;
                secondArg = arg2.value;
                hasSecondArg = true;
            }

            SkipWhitespace();
            if (!Match(')')) {
                return EvaluationResult{false, 0.0, "Missing closing parenthesis for function call '" + ident + "'"};
            }

            if (ident == "sqrt") {
                if (arg.value < 0.0) return EvaluationResult{false, 0.0, "Square root of negative number."};
                return EvaluationResult{true, std::sqrt(arg.value), ""};
            }
            if (ident == "abs") return EvaluationResult{true, std::abs(arg.value), ""};
            if (ident == "sin") return EvaluationResult{true, std::sin(arg.value), ""};
            if (ident == "cos") return EvaluationResult{true, std::cos(arg.value), ""};
            if (ident == "tan") return EvaluationResult{true, std::tan(arg.value), ""};
            if (ident == "min" && hasSecondArg) return EvaluationResult{true, std::min(arg.value, secondArg), ""};
            if (ident == "max" && hasSecondArg) return EvaluationResult{true, std::max(arg.value, secondArg), ""};

            return EvaluationResult{false, 0.0, "Unknown function: " + ident};
        }

        // Look up variable in state
        auto it = vars.find(ident);
        if (it != vars.end()) {
            return EvaluationResult{true, it->second, ""};
        }

        return EvaluationResult{false, 0.0, "Undefined variable: " + ident};
    }

    return EvaluationResult{false, 0.0, std::string("Unexpected character: ") + c};
}

std::unordered_map<std::string, EvaluationResult> ExpressionParser::EvaluateAll(
    const std::unordered_map<std::string, std::string>& expressions,
    const std::unordered_map<std::string, double>& baseVariables) {

    std::unordered_map<std::string, EvaluationResult> results;
    std::unordered_map<std::string, double> currentVars = baseVariables;

    // Build dependency graph
    // Extract tokens from each expression to detect which variables it depends on
    std::unordered_map<std::string, std::vector<std::string>> deps;
    for (const auto& [name, expr] : expressions) {
        std::vector<std::string>& nodeDeps = deps[name];
        for (size_t i = 0; i < expr.size();) {
            if (IsIdentStart(expr[i])) {
                size_t start = i;
                while (i < expr.size() && IsIdentChar(expr[i])) i++;
                std::string token = expr.substr(start, i - start);
                if (expressions.find(token) != expressions.end() && token != name) {
                    nodeDeps.push_back(token);
                }
            } else {
                i++;
            }
        }
    }

    // Topological sort (DFS with cycle detection)
    std::vector<std::string> sortedOrder;
    std::unordered_map<std::string, int> visitState; // 0: unvisited, 1: visiting, 2: visited

    auto dfs = [&](auto& self, const std::string& node) -> bool {
        visitState[node] = 1; // Mark visiting
        for (const auto& dep : deps[node]) {
            if (visitState[dep] == 1) {
                return false; // Cycle detected!
            }
            if (visitState[dep] == 0) {
                if (!self(self, dep)) return false;
            }
        }
        visitState[node] = 2; // Visited
        sortedOrder.push_back(node);
        return true;
    };

    bool hasCycle = false;
    for (const auto& [name, expr] : expressions) {
        if (visitState[name] == 0) {
            if (!dfs(dfs, name)) {
                hasCycle = true;
                break;
            }
        }
    }

    if (hasCycle) {
        for (const auto& [name, expr] : expressions) {
            results[name] = EvaluationResult{false, 0.0, "Cyclic dependency detected in parameter definitions."};
        }
        return results;
    }

    // Evaluate in topologically sorted order
    ExpressionParser parser;
    for (const auto& [name, val] : baseVariables) {
        parser.SetVariable(name, val);
    }

    for (const auto& name : sortedOrder) {
        const std::string& expr = expressions.at(name);
        EvaluationResult res = parser.Evaluate(expr);
        results[name] = res;
        if (res.success) {
            parser.SetVariable(name, res.value);
        }
    }

    return results;
}

} // namespace androcad
