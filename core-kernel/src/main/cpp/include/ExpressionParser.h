// Copyright (c) 2026 dursuntokgoz. All rights reserved.
#ifndef ANDROCAD_EXPRESSION_PARSER_H
#define ANDROCAD_EXPRESSION_PARSER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

namespace androcad {

struct EvaluationResult {
    bool success;
    double value;
    std::string errorMessage;
};

class ExpressionParser {
public:
    ExpressionParser() = default;
    ~ExpressionParser() = default;

    // Registers or updates a named user parameter
    void SetVariable(const std::string& name, double value);
    bool HasVariable(const std::string& name) const;
    double GetVariable(const std::string& name) const;
    const std::unordered_map<std::string, double>& GetVariables() const { return variables; }

    // Evaluates a mathematical string expression using current variables
    EvaluationResult Evaluate(const std::string& expression) const;

    // Evaluates a dictionary of named expressions with topological cycle detection
    // e.g. {"width": "50", "length": "width * 2", "height": "length / 4"}
    static std::unordered_map<std::string, EvaluationResult> EvaluateAll(
        const std::unordered_map<std::string, std::string>& expressions,
        const std::unordered_map<std::string, double>& baseVariables = {}
    );

private:
    std::unordered_map<std::string, double> variables;

    // Recursive descent parser implementation
    struct ParserState {
        std::string expr;
        size_t pos = 0;
        const std::unordered_map<std::string, double>& vars;

        char Peek() const;
        char Get();
        void SkipWhitespace();
        bool Match(char expected);

        EvaluationResult ParseExpression();
        EvaluationResult ParseTerm();
        EvaluationResult ParseFactor();
        EvaluationResult ParsePrimary();
    };
};

} // namespace androcad

#endif // ANDROCAD_EXPRESSION_PARSER_H
