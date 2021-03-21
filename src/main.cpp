#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include <optional>
#include <map>
#include <set>
using namespace std;

using NullaryFunc = double(*)();                    // f
using UnaryFunc = double(*)(double);                // f op1
using BinaryFunc = double(*)(double, double);       // op1 f op2
using Command = bool(*)(const vector<string>&);     // comm arg1 arg2 ...

optional<double> readExpression(const string&);
optional<int> readInteger(const string&);

namespace Calculator {
    double lastResult = 0;
    array<double, 10> memory = {};
}

class Operation {
public:
    string str;

    Operation(const string& str) : str(str) {}
};

class UnaryOperation : public Operation {
public:
    UnaryFunc func;

    UnaryOperation(const string& str, UnaryFunc func) : Operation(str), func(func) {}
};

class BinaryOperation : public Operation {
public:
    BinaryFunc func;

    BinaryOperation(const string& str, BinaryFunc func) : Operation(str), func(func) {}
};

class PrecedenceSet {
public:
    vector<BinaryOperation> operations;
    enum Assoc { LeftAssoc, RightAssoc } assoc;

    PrecedenceSet(vector<BinaryOperation>&& operations, Assoc assoc) : operations(move(operations)), assoc(assoc) {}
    auto begin() const { return operations.begin(); }
    auto end() const { return operations.end(); }
};

map<string, NullaryFunc> aliases = {
        {"pi", [](){return M_PI;}}
};

vector<UnaryOperation> unaryOperations = {
        UnaryOperation("+", [](double x){return x;}),
        UnaryOperation("-", [](double x){return -x;}),
        UnaryOperation("sin", [](double x){return sin(x);}),
        UnaryOperation("cos", [](double x){return cos(x);}),
        UnaryOperation("tan", [](double x){return tan(x);}),
        UnaryOperation("log", [](double x){return log(x);}),
        UnaryOperation("exp", [](double x){return exp(x);}),
        UnaryOperation("abs", [](double x){return abs(x);})
};

vector<PrecedenceSet> binaryOperations = {
        PrecedenceSet({                 // precedence: 0
                BinaryOperation("+", [](double x, double y){return x + y;}),
                BinaryOperation("-", [](double x, double y){return x - y;})
        }, PrecedenceSet::LeftAssoc),
        PrecedenceSet({                 // precedence: 1
                BinaryOperation("*", [](double x, double y){return x * y;}),
                BinaryOperation("/", [](double x, double y){return x / y;})
        }, PrecedenceSet::LeftAssoc),
        PrecedenceSet({                 // precedence: 2
                BinaryOperation("^", [](double x, double y){return pow(x, y);})
        }, PrecedenceSet::RightAssoc),
};

map<string, Command> commands = {
        {"exit", [](auto args){
            if (args.size() != 0) return false;
            exit(0);
        }},
        {"set", [](auto args){
            if (args.size() != 2) return false;
            auto index = readInteger(args[0]);
            if (!index || *index > Calculator::memory.size()) return false;
            auto value = readExpression(args[1]);
            if (!value) return false;
            Calculator::memory[*index] = *value;
            return true;
        }},
};

string trim(const string& str) {
    ssize_t i, j;

    for (i=0; i<str.size() && isspace(str[i]); i++);
    if (i == str.size()) return "";
    for (j=str.size()-1; isspace(str[j]); j--);
    return str.substr(i, j-i+1);
}

optional<int> readInteger(const string& str) {
    int temp;
    istringstream ss(str);

    if (str[0]=='-' || str[0]=='+') return {};   // ignore unary operator
    if (!isspace(str[0]) && ss>>temp && ss.eof()) return temp;
    else return {};
}

optional<double> readDouble(const string& str) {
    double temp;
    istringstream ss(str);

    if (str[0]=='-' || str[0]=='+') return {};   // ignore unary operator
    if (!isspace(str[0]) && ss>>temp && ss.eof()) return temp;
    else return {};
}

optional<double> readPercent(const string& str) {
    if (str.find('%') != 0) return {};
    if (str.size() == 1) return Calculator::lastResult;
    auto index = readInteger(str.substr(1));
    if (!index || *index > Calculator::memory.size()) return {};
    return Calculator::memory[*index];
}

optional<double> readBinaryOp(const string& str) {
    for (const auto& precedenceSet : binaryOperations) {
        if (precedenceSet.assoc == PrecedenceSet::LeftAssoc) {
            for (ssize_t i = str.size(); i >= 0; i--) {
                for (const auto& operation : precedenceSet) {
                    if (str.substr(i - operation.str.size() + 1, operation.str.size()) == operation.str) {
                        string expr1 = str.substr(0, i - operation.str.size() + 1);
                        string expr2 = str.substr(i + 1);

                        auto operand1 = readExpression(trim(expr1));
                        if (!operand1) break;
                        auto operand2 = readExpression(trim(expr2));
                        if (!operand2) break;

                        return operation.func(*operand1, *operand2);
                    }
                }
            }
        } else {
            for (ssize_t i = 0; i < str.size(); i++) {
                for (const auto& operation : precedenceSet) {
                    if (str.substr(i, operation.str.size()) == operation.str) {
                        string expr1 = str.substr(0, i);
                        string expr2 = str.substr(i + operation.str.size());

                        auto operand1 = readExpression(trim(expr1));
                        if (!operand1) break;
                        auto operand2 = readExpression(trim(expr2));
                        if (!operand2) break;

                        return operation.func(*operand1, *operand2);
                    }
                }
            }
        }
    }
    return {};
}

optional<double> readUnaryOp(const string& str) {
    for (const auto& operation : unaryOperations) {
        if (str.find(operation.str) != 0) continue;
        string expr = str.substr(operation.str.size());
        auto operand = readExpression(trim(expr));
        if (!operand) return {};
        return operation.func(*operand);
    }
    return {};
}

optional<double> readAlias(const string& str) {
    auto func = aliases[str];
    if (func == nullptr) return {};
    return func();
}

optional<double> readParantheses(const string& str) {
    if (!str.empty() && str.front() == '(' && str.back() == ')') {
        return readExpression(trim(str.substr(1, str.size()-2)));
    }
    return {};
}

optional<double> readExpression(const string& str) {
    optional<double> result;
    if ((result = readPercent(str)) || (result = readDouble(str)) || (result = readParantheses(str))
        || (result = readBinaryOp(str)) || (result = readUnaryOp(str)) || (result = readAlias(str))
    ) return result;
    return {};
}

bool execCommand(const string& line) {
    string command;
    vector<string> arguments;
    stringstream stream(line);
    string temp;
    if (!(stream >> command)) return false;
    while (stream >> temp) arguments.push_back(temp);

    auto func = commands[command];
    if (func == nullptr) return false;
    return func(arguments);
}

bool execLine(const string& line) {
    auto trimmed = trim(line);
    optional<double> lineVal;

    if (trimmed.empty()) {
        return true;
    } else if (execCommand(trimmed)) {
        return true;
    }
    else if ((lineVal = readExpression(trimmed))) {
        cout << *lineVal << endl;
        Calculator::lastResult = *lineVal;
        return true;
    } else {
        return false;
    }
}

bool execFile(istream& stream) {
    for (string line; getline(stream, line);) {
        if (!execLine(line)) return false;
    }
    return true;
}

int main(int argc, char *argv[]) {
    istream* streamPtr;
    ifstream file;

    try {
        if (argc == 1) {
            streamPtr = &cin;
        }
        else if (argc == 2) {
            file.open(argv[1]);
            if (!file.is_open()) {
                throw runtime_error("File " + string(argv[1]) + " couldn't be opened for input.");
            }
            streamPtr = &file;
        } else {
            throw runtime_error("Invalid number of arguments.");
        }

        if (!execFile(*streamPtr)) {
            throw runtime_error("Syntax error.");
        }
    } catch (exception& e) {
        cerr << "ERROR: " << e.what() << endl;
        return EXIT_FAILURE;
    }
}