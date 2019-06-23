#ifndef ark_parser
#define ark_parser

#include <Ark/Parser/SimpleParser.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <optional>
#include <Ark/Parser/ASTNode.hpp>
#include <vector>
#include <algorithm>
#include <Ark/Parser/Node.hpp>

namespace Ark
{
    using MaybeNodePtr = std::optional<internal::NodePtr>;

    namespace internal
    {
        const std::vector<std::string> keywords = {
            "let", "mut", "fun", "ret", "end",
            "if", "then", "elif", "else",
            "while", "do",
            "import", "use"
        };

        const std::vector<std::string> assign_op = {
            "+", "-", "*", "/", "<<", ">>", "~"
        };

        const std::vector<std::string> operators = {
            "+", "-", "*", "/", "<<", ">>", "~",
            "and", "or", "not", "==", "!=", "<",
            ">", "<=", ">="
        };
    }

    class Parser : private internal::SimpleParser
    {
    public:
        Parser(const std::string& code, const std::string& filename, bool debug=false);
        ~Parser();

        void parse();
        void prettyPrintAST(std::ostream* os=nullptr);

        const internal::Node& ast() const;
    
    private:
        internal::Program m_program;
        std::string m_filename;
        internal::Node m_internalAST;
        bool m_debug;

        inline bool isKeyword(const std::string& name)
        {
            return std::find(internal::keywords.begin(), internal::keywords.end(), name) != internal::keywords.end();
        }

        inline bool isAssignOperator(const std::string& name)
        {
            return std::find(internal::assign_op.begin(), internal::assign_op.end(), name) != internal::assign_op.end();
        }

        inline bool isOperator(const std::string& name)
        {
            return std::find(internal::operators.begin(), internal::operators.end(), name) != internal::operators.end();
        }

        // custom parsers for tokens
        bool operator_(std::string* s=nullptr);
        bool inlineSpace(std::string* s=nullptr);
        bool endOfLine(std::string* s=nullptr);
        bool comment(std::string* s=nullptr);
        bool endOfLineAndOrComment(std::string* s=nullptr);
        bool type_(std::string* s=nullptr);

        // parsers
        MaybeNodePtr parseInstruction();

        MaybeNodePtr parseLet();
        MaybeNodePtr parseMut();
        MaybeNodePtr parseAssignment();
        MaybeNodePtr parseExp();
            MaybeNodePtr parseOperation();
            MaybeNodePtr parseSingleExp();
                MaybeNodePtr parseOperationBlock();
                MaybeNodePtr parseInt();
                MaybeNodePtr parseFloat();
                MaybeNodePtr parseString();
                MaybeNodePtr parseBool();
                MaybeNodePtr parseFunctionCall();
                MaybeNodePtr parseClosureFieldReading();
                MaybeNodePtr parseVarUse();
                MaybeNodePtr parseFunction();
        MaybeNodePtr parseEnd();
        MaybeNodePtr parseIf();
        MaybeNodePtr parseWhile();
        MaybeNodePtr parseImport();
    };
}

#endif