#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <unordered_map>
#include <cstring>
#include <cstdlib>
using namespace std;
enum class TokenType
{
    LET,
    IF,
    ELSE,
    WHILE,
    PRINT,
    IDENTIFIER,
    NUMBER,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    EQUAL,
    LESS,
    GREATER,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    SEMICOLON,
    END
};

struct Token
{
    TokenType type;
    string value;
};

class Lexer
{
public:
    explicit Lexer(const std::string &input) : input(input), pos(0) {}

    vector<Token> tokenize()
    {
        vector<Token> tokens;
        while (pos < input.length())
        {
            if (std::isspace(input[pos]))
            {
                ++pos;
            }
            else if (std::isalpha(input[pos]))
            {
                tokens.push_back(identifier());
            }
            else if (std::isdigit(input[pos]))
            {
                tokens.push_back(number());
            }
            else
            {
                tokens.push_back(symbol());
            }
        }
        tokens.push_back({TokenType::END, ""});
        return tokens;
    }

private:
    string input;
    size_t pos;

    Token identifier()
    {
        string value;
        while (pos < input.length() && std::isalnum(input[pos]))
        {
            value += input[pos++];
        }
        if (value == "let")
            return {TokenType::LET, value};
        if (value == "if")
            return {TokenType::IF, value};
        if (value == "else")
            return {TokenType::ELSE, value};
        if (value == "while")
            return {TokenType::WHILE, value};
        if (value == "print")
            return {TokenType::PRINT, value};
        return {TokenType::IDENTIFIER, value};
    }

    Token number()
    {
        string value;
        while (pos < input.length() && std::isdigit(input[pos]))
        {
            value += input[pos++];
        }
        return {TokenType::NUMBER, value};
    }

    Token symbol()
    {
        switch (input[pos++])
        {
        case '+':
            return {TokenType::PLUS, "+"};
        case '-':
            return {TokenType::MINUS, "-"};
        case '*':
            return {TokenType::STAR, "*"};
        case '/':
            return {TokenType::SLASH, "/"};
        case '=':
            return {TokenType::EQUAL, "="};
        case '<':
            return {TokenType::LESS, "<"};
        case '>':
            return {TokenType::GREATER, ">"};
        case '(':
            return {TokenType::LPAREN, "("};
        case ')':
            return {TokenType::RPAREN, ")"};
        case '{':
            return {TokenType::LBRACE, "{"};
        case '}':
            return {TokenType::RBRACE, "}"};
        case ';':
            return {TokenType::SEMICOLON, ";"};
        default:
            throw std::runtime_error("Unknown symbol");
        }
    }
};

#include <fstream>
#include <unordered_map>
#include <cstring>
#include <cstdlib>

struct ASTNode {
    virtual ~ASTNode() {}
};

// Example AST node structures (similar to the ones in the previous message):
struct NumberNode : ASTNode {
    int value;
    NumberNode(int value) : value(value) {}
};

struct VariableNode : ASTNode {
    char* name;

    VariableNode(const char* name) {
        this->name = (char*)malloc(strlen(name) + 1);
        strcpy(this->name, name);
    }

    ~VariableNode() {
        free(name);
    }
};

struct BinaryOpNode : ASTNode {
    char* op;
    ASTNode* left;
    ASTNode* right;

    BinaryOpNode(const char* op, ASTNode* left, ASTNode* right) {
        this->op = (char*)malloc(strlen(op) + 1);
        strcpy(this->op, op);
        this->left = left;
        this->right = right;
    }

    ~BinaryOpNode() {
        free(op);
        delete left;
        delete right;
    }
};

struct AssignmentNode : ASTNode {
    char* varName;
    ASTNode* value;

    AssignmentNode(const char* varName, ASTNode* value) {
        this->varName = (char*)malloc(strlen(varName) + 1);
        strcpy(this->varName, varName);
        this->value = value;
    }

    ~AssignmentNode() {
        free(varName);
        delete value;
    }
};

struct IfNode : ASTNode {
    ASTNode* condition;
    ASTNode* thenBranch;
    ASTNode* elseBranch;

    IfNode(ASTNode* condition, ASTNode* thenBranch, ASTNode* elseBranch)
        : condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}

    ~IfNode() {
        delete condition;
        delete thenBranch;
        delete elseBranch;
    }
};

// The CodeGenerator class for generating assembly code
class CodeGenerator {
public:
    explicit CodeGenerator(const char* outputFileName) : outputFileName(outputFileName), labelCount(0) {}

    void generateCode(ASTNode* node) {
        output.open(outputFileName);
        if (!output.is_open()) {
            throw std::runtime_error("Failed to open output file for assembly code.");
        }

        // Data section for storing variables
        output << "section .data\n";
        for (int i = 0; i < numVariables; ++i) {
            output << variables[i] << " dq 0\n";
        }

        // Text section for the main program
        output << "section .text\n";
        output << "global _start\n";
        output << "_start:\n";

        // Generate the assembly code from the AST node
        generateNode(node);

        // Add exit code to terminate the program
        output << "    mov rax, 60    ; syscall: exit\n";
        output << "    xor rdi, rdi   ; status 0\n";
        output << "    syscall\n";

        output.close();
    }

    void addVariable(const char* varName) {
        variables[numVariables] = (char*)malloc(strlen(varName) + 1);
        strcpy(variables[numVariables], varName);
        ++numVariables;
    }

private:
    std::ofstream output;
    const char* outputFileName;
    int labelCount;
    char* variables[100]; // Store up to 100 variables
    int numVariables = 0;

    void generateNode(ASTNode* node) {
        if (NumberNode* numNode = dynamic_cast<NumberNode*>(node)) {
            output << "    mov rax, " << numNode->value << "\n";
        } else if (VariableNode* varNode = dynamic_cast<VariableNode*>(node)) {
            output << "    mov rax, [rel " << varNode->name << "]\n";


        } else if (AssignmentNode* assignNode = dynamic_cast<AssignmentNode*>(node)) {
            generateNode(assignNode->value);
            output << "    mov [" << assignNode->varName << "], rax\n";
        } else if (BinaryOpNode* binOpNode = dynamic_cast<BinaryOpNode*>(node)) {
            generateNode(binOpNode->left);
            output << "    push rax\n";
            generateNode(binOpNode->right);
            output << "    pop rbx\n";
            if (strcmp(binOpNode->op, "+") == 0) {
                output << "    add rax, rbx\n";
            } else if (strcmp(binOpNode->op, "-") == 0) {
                output << "    sub rax, rbx\n";
            }
            // Add other operations as needed
        } else if (IfNode* ifNode = dynamic_cast<IfNode*>(node)) {
            int elseLabel = labelCount++;
            int endLabel = labelCount++;

            generateNode(ifNode->condition);
            output << "    cmp rax, 0\n";
            output << "    je .L" << elseLabel << "\n";
            generateNode(ifNode->thenBranch);
            output << "    jmp .L" << endLabel << "\n";
            output << ".L" << elseLabel << ":\n";
            if (ifNode->elseBranch) {
                generateNode(ifNode->elseBranch);
            }
            output << ".L" << endLabel << ":\n";
        }
    }
};
int main() {
    CodeGenerator generator("program.asm");

    // Declare variables "x" and "y"
    generator.addVariable("x");
    generator.addVariable("y");

    // Create nodes and AST for the program: let x = 5; let y = 10; if (x < y) { x = x + 1; } else { y = y + 1; }
    NumberNode* five = new NumberNode(5);
    VariableNode* varX = new VariableNode("x");
    AssignmentNode* assignX = new AssignmentNode("x", five);

    NumberNode* ten = new NumberNode(10);
    VariableNode* varY = new VariableNode("y");
    AssignmentNode* assignY = new AssignmentNode("y", ten);

    BinaryOpNode* condition = new BinaryOpNode("<", varX, varY);
    NumberNode* one = new NumberNode(1);
    BinaryOpNode* incrementX = new BinaryOpNode("+", varX, one);
    AssignmentNode* thenBranch = new AssignmentNode("x", incrementX);

    BinaryOpNode* incrementY = new BinaryOpNode("+", varY, one);
    AssignmentNode* elseBranch = new AssignmentNode("y", incrementY);

    IfNode* ifStatement = new IfNode(condition, thenBranch, elseBranch);

    // Generate code for the program
    generator.generateCode(ifStatement);

    // Clean up
    delete assignX;
    delete assignY;
    delete ifStatement;

    return 0;
}
