#include <Ark/Parser/Parser.hpp>

#include <Ark/Log.hpp>

namespace Ark
{
    using namespace Ark::internal;

    Parser::Parser(const std::string& code, const std::string& filename) :
        m_filename(filename), internal::SimpleParser(code)
    {}

    Parser::~Parser()
    {}

    void Parser::parse()
    {
        // parse until we reach the end of the file
        while (!isEOF())
        {
            MaybeNodePtr inst = parseInstruction();
            if (!inst)
            {
                Ark::logger.error("[Parser] couldn't recognize instruction");
                break;
            }
            else
                m_program.children.push_back(std::move(inst.value()));
        }

        // TODO transform m_program into m_internalAST
    }

    void Parser::prettyPrintAST(std::ostream* os)
    {
        m_program.print(*os, 0);
    }

    const Node& Parser::ast() const
    {
        return m_internalAST;
    }

    bool Parser::operator_(std::string* s)
    {
        if (accept(IsNot(IsChar('=')), s))
        {
            while (accept(IsNot(IsChar('=')), s));
            return true;
        }
        return false;
    }

    bool Parser::inlineSpace(std::string* s)
    {
        if (accept(IsInlineSpace))
        {
            if (s != nullptr)
                s->push_back(' ');
            // loop while there are still ' ' to consume
            while (accept(IsInlineSpace));
            return true;
        }
        return false;
    }

    bool Parser::endOfLine(std::string* s)
    {
        if ((accept(IsChar('\r')) || true) && accept(IsChar('\n')))
        {
            if (s != nullptr)
                s->push_back('\n');
            while ((accept(IsChar('\r')) || true) && accept(IsChar('\n')));
            return true;
        }
        return false;
    }

    bool Parser::comment(std::string* s)
    {
        inlineSpace();

        // inline comment starts with '#'
        if (accept(IsChar('#')))
        {
            while (accept(IsNot(IsChar('\n')), s));
            return true;
        }
        return false;
    }

    bool Parser::endOfLineAndOrComment(std::string* s)
    {
        comment(s);
        return endOfLine(s);
    }

    bool Parser::type_(std::string* s)
    {
        if (!name(s))
            return false;
        if (accept(IsSpace, s) && accept(IsChar('-'), s) && accept(IsChar('>'), s) && accept(IsSpace, s))
            return type_(s);
        return true;
    }

    MaybeNodePtr Parser::parseInstruction()
    {
        // save current position in buffer to be able to go back if needed
        auto current = getCount();

        // parsing single line comments as instructions
        while (comment())
            endOfLine();

        // let x = value
        if (auto inst = parseLet())
            return inst;
        else
            back(getCount() - current + 1);
        
        // mut x = value
        if (auto inst = parseMut())
            return inst;
        else
            back(getCount() - current + 1);
        
        // x = value
        if (auto inst = parseAssignment())
            return inst;
        else
            back(getCount() - current + 1);
        
        // token 'end' closing a block
        if (auto inst = parseEnd())
            return inst;
        else
            back(getCount() - current + 1);

        if (auto inst = parseExp())
            return inst;
        else
            back(getCount() - current + 1);
        
        // while cond do {body} end
        if (auto inst = parseWhile())
            return inst;
        else
            back(getCount() - current + 1);
        
        // import "stuff.ark"
        if (auto inst = parseImport())
            return inst;
        else
            back(getCount() - current + 1);
        
        return {};
    }

    MaybeNodePtr Parser::parseLet()
    {
        /*
            let x = value
        */

        // eat the trailing white space
        inlineSpace();

        // checking if 'let' is present
        std::string let_qualifier = "";
        if (!name(&let_qualifier) || let_qualifier != "let")
            return {};
        
        inlineSpace();

        std::string varname = "";
        if (!name(&varname))
            error("Expected identifier", varname);
        
        inlineSpace();
        // checking for value
        if (!accept(IsChar('=')))
            return {};
        
        inlineSpace();

        if (auto exp = parseExp())
        {
            auto temp = std::make_shared<Let>(varname, exp.value());
            if (!endOfLineAndOrComment())
                error("Expected end of line after definition (let)", "");
            return temp;
        }
        else
            error("Expected a valid expression for definition (let)", "");

        return {};
    }

    MaybeNodePtr Parser::parseMut()
    {
        /*
            mut x = value
        */

        // eat the trailing white space
        inlineSpace();

        // checking if 'mut' is present
        std::string mut_qualifier = "";
        if (!name(&mut_qualifier) || mut_qualifier != "mut")
            return {};
        
        inlineSpace();

        std::string varname = "";
        if (!name(&varname))
            error("Expected identifier", varname);
        
        inlineSpace();
        // checking for value
        except(IsChar('='));
        inlineSpace();
        
        if (auto exp = parseExp())
        {
            auto temp = std::make_shared<Mut>(varname, exp.value());
            if (!endOfLineAndOrComment())
                error("Expected end of line after definition (mut)", "");
            return temp;
        }
        else
            error("Expected a valid expression as a value for definition (mut)", "");
        
        return {};
    }

    MaybeNodePtr Parser::parseAssignment()
    {
        /*
            a op= value
        */

        // eat the trailing white space
        inlineSpace();

        std::string varname = "";
        if (!name(&varname))
            return {};
        
        inlineSpace();
        
        // we can have an operator before the '=' sign
        std::string op = "";
        if (accept(IsChar('=')));
        else
        {
            if (!operator_(&op) || !isAssignOperator(op))
                return {};
            except(IsChar('='));
        }
        
        inlineSpace();
        
        if (auto exp = parseExp())
        {
            auto temp = std::make_shared<Assignment>(varname, exp.value(), op);
            if (!endOfLineAndOrComment())
                error("Expected end of line after assignment", "");
            return temp;
        }
        else
            error("Expected a valid expression as a value to assign to variable", "");
        
        return {};
    }

    MaybeNodePtr Parser::parseExp()
    {
        /*
            Trying to parse right hand side values, such as:
            - integers
            - floats
            - strings
            - booleans
            - function call
            - closure field reading
            - operations (comparisons, additions...)
            - functions
        */

        auto current = getCount();

        // parsing operations before anything else because it must use the other parsers
        if (auto exp = parseOperation())
            return exp;
        else
            back(getCount() - current + 1);

        if (auto exp = parseSingleExp())
            return exp;
        else
            back(getCount() - current + 1);

        return {};
    }

    MaybeNodePtr Parser::parseOperation()
    {
        /*
            Trying to parse operations such as
            1 + 2
            1 / (2 + 3)

            Get current token: it must be a '('
            If it's not => quit
            Otherwise, get token[n+1], it must an expression
            Token[n+2] must be an operator:
                +, -, *, /
                <<, >>, ~
                and, or, not
                ==, !=, <, <=, >, >=
            Parsing end when:
                - the expression couldn't be parsed (VarUse being a keyword can break the whole thing)
                - there was no error and we found the matching ')'
            
            TODO This parser should also do a "shunting yard"
        */
        
        // parse expressions
        NodePtrList operations;
        while (true)
        {
            inlineSpace();

            // getting prefix operator
            auto current = getCount();
            if (accept(IsMinus) && accept(IsSpace))
                operations.push_back(std::make_shared<Operator>("-"));
            else
                back(getCount() - current + 1);
            
            if (accept(IsChar('~')))
                operations.push_back(std::make_shared<Operator>("~"));
            else
                back(getCount() - current + 1);
            
            std::string token = "";
            if (name(&token) && token == "not")
                operations.push_back(std::make_shared<Operator>("not"));
            else
                back(getCount() - current + 1);
            
            inlineSpace();
            
            // get operand
            current = getCount();
            if (auto exp = parseSingleExp())
                operations.push_back(exp.value());
            else
            {
                // TODO wtf
                std::cout << "???" << std::endl;
                back(getCount() - current + 1);
                break;
            }
            
            inlineSpace();

            current = getCount();
            std::string op = "";
            if (!operator_(&op))
                return {};
            if (!isOperator(op))
            {
                if (operations.size() < 2)
                    return {};
                
                back(getCount() - current + 1);
                break;
            }
            
            operations.push_back(std::make_shared<Operator>(op));
        }

        if (operations.size() == 0)
            return {};

        return std::make_shared<OperationsList>(operations);
    }

    MaybeNodePtr Parser::parseSingleExp()
    {
        auto current = getCount();

        if (auto exp = parseOperationBlock())  // (1 + 2 - 4)
            return exp;
        else
            back(getCount() - current + 1);

        // parsing float before integer because float requires a '.'
        if (auto exp = parseFloat())  // 1.5
            return exp;
        else
            back(getCount() - current + 1);

        if (auto exp = parseInt())  // 42
            return exp;
        else
            back(getCount() - current + 1);

        if (auto exp = parseString())  // "hello world"
            return exp;
        else
            back(getCount() - current + 1);

        if (auto exp = parseBool())  // true
            return exp;
        else
            back(getCount() - current + 1);
        
        if (auto exp = parseFunctionCall())  // foo(42, -6.66)
            return exp;
        else
            back(getCount() - current + 1);
        
        if (auto exp = parseClosureFieldReading())  // bar.foo, bar.foo(45, ...)
            return exp;
        else
            back(getCount() - current + 1);
        
        // must the last one, otherwise it would try to parse function/method calls
        if (auto exp = parseVarUse())  // varname
            return exp;
        else
            back(getCount() - current + 1);
        
        if (auto exp = parseFunction())  // fun (a: T, ...): T {body} end
            return exp;
        else
            back(getCount() - current + 1);
        
        error("Couldn't parse single expression", "");
        return {};  // to avoid warnings
    }

    MaybeNodePtr Parser::parseOperationBlock()
    {
        /*
            Trying to parse operations, but inside parens, such as:

            (1 + 2 - 5)
        */

        if (accept(IsChar('(')))
        {
            MaybeNodePtr op = parseOperation();
            if (op && accept(IsChar(')')))
                return op;
        }

        return {};
    }

    MaybeNodePtr Parser::parseInt()
    {
        std::string n = "";
        if (signedNumber(&n))
            return std::make_shared<Integer>(std::stoi(n));
        return {};
    }

    MaybeNodePtr Parser::parseFloat()
    {
        std::string f = "";
        if (signedNumber(&f) && accept(IsChar('.'), &f) && number(&f))
            return std::make_shared<Float>(std::stof(f));
        return {};
    }

    MaybeNodePtr Parser::parseString()
    {
        std::string s = "";

        if (accept(IsChar('"'), &s))
        {
            while (accept(IsNot(IsChar('"')), &s));
            except(IsChar('"'), &s);

            // remove " at the beginning and at the end
            s.erase(0, 1);
            s.erase(s.end() - 1);

            return std::make_shared<String>(s);
        }
        return {};
    }

    MaybeNodePtr Parser::parseBool()
    {
        std::string s = "";

        if (!name(&s))
            return {};
        
        if (s == "false")
            return std::make_shared<Bool>(false);
        else if (s == "true")
            return std::make_shared<Bool>(true);
        
        return {};
    }

    MaybeNodePtr Parser::parseFunctionCall()
    {
        /*
            Trying to parse stuff like this:

            main(1, "hello")
            foo(true)
            doStuff()
        */

        inlineSpace();

        // getting the name of the function
        std::string funcname = "";
        if (!name(&funcname))
            return {};
        
        inlineSpace();
        
        // getting the arguments
        NodePtrList arguments;
        if (accept(IsChar('(')))
        {
            while (true)
            {
                // eat the trailing white space
                inlineSpace();

                // check if end of arguments
                if (accept(IsChar(')')))
                    break;

                // find argument
                if (auto inst = parseExp())
                    arguments.push_back(inst.value());
                else
                    error("Expected a valid expression as function argument", "");

                inlineSpace();

                // check for ',' -> other arguments
                if (accept(IsChar(',')))
                    continue;
            }

            return std::make_shared<FunctionCall>(funcname, arguments);
        }
        return {};
    }

    MaybeNodePtr Parser::parseClosureFieldReading()
    {
        /*
            Trying to parse stuff like this:

            object.main
            player.foo(true)
            you.doStuff()
        */

        inlineSpace();

        // getting the name of the object
        std::string objectname = "";
        if (!name(&objectname))
            return {};
        
        if (!accept(IsChar('.')))  // '.' between object name and method name
            return {};
        
        // getting function name
        std::string funcname = "";
        if (!name(&funcname))
            error("Expecting a method name after '" + objectname + ".'", funcname);
        
        inlineSpace();
        
        // getting the arguments
        NodePtrList arguments;
        if (accept(IsChar('(')))
        {
            while (true)
            {
                // eat the trailing white space
                inlineSpace();

                // check if end of arguments
                if (accept(IsChar(')')))
                    break;

                // find argument
                if (auto inst = parseExp())
                    arguments.push_back(inst.value());
                else
                    error("Expected a valid expression as method argument", "");

                inlineSpace();

                // check for ',' -> other arguments
                if (accept(IsChar(',')))
                    continue;
            }

            return std::make_shared<ClosureFieldCall>(objectname, funcname, arguments);
        }
        return std::make_shared<ClosureFieldRead>(objectname, funcname);
    }

    MaybeNodePtr Parser::parseVarUse()
    {
        /*
            Trying to parse things such as

            let x = varname
            ~~~~~~~~~^^^^^^^
        */

        std::string varname = "";
        if (!name(&varname))
            return {};
        
        // checking if varname is a keyword
        if (isKeyword(varname))
            return {};
        
        return std::make_shared<VarUse>(varname);
    }

    MaybeNodePtr Parser::parseFunction()
    {
        /*
            Trying to parse functions and closures (with explicit capture):

            fun (arg: type, arg2: type, ...) -> type
                code...
            end

            use (a, b, ...) fun (arg: type, arg2: type, ...) -> type
                code...
            end
        */

        // eat trailing white spaces
        inlineSpace();

        // checking for 'fun' or 'use'
        std::string keyword = "";
        if (!name(&keyword))
            return {};
        if (keyword != "fun" && keyword != "use")
            return {};
        
        inlineSpace();
        
        NodePtrList capture;
        if (keyword == "use")
        {
            except(IsChar('('));

            while (true)
            {
                // eat the trailing white space
                inlineSpace();

                // check if end of arguments
                if (accept(IsChar(')')))
                    break;

                std::string varname = "";
                if (!name(&varname))
                    break;  // we don't have arguments
                
                inlineSpace();

                // register argument
                capture.push_back(
                    std::make_shared<Capture>(varname)
                );

                // check for ',' -> other arguments
                if (accept(IsChar(',')))
                    continue;
            }
        }

        inlineSpace();

        // getting arguments (enclosed in ())
        NodePtrList arguments;
        except(IsChar('('));

        while (true)
        {
            // eat the trailing white space
            inlineSpace();

            // check if end of arguments
            if (accept(IsChar(')')))
                break;

            std::string varname = "";
            if (!name(&varname))
                break;  // we don't have arguments
            
            inlineSpace();
            // : after varname and before type is mandatory
            if (!except(IsChar(':')))
                error("Expected ':' after argument name and before type name", "");
            inlineSpace();

            std::string type = "";
            if (!type_(&type))
                error("Expected type name for argument in function definition", type);
            
            inlineSpace();

            // register argument
            arguments.push_back(
                std::make_shared<Argument>(varname, type)
            );

            // check for ',' -> other arguments
            if (accept(IsChar(',')))
                continue;
        }

        inlineSpace();
        // need the full '->'
        except(IsChar('-')); except(IsChar('>'));
        inlineSpace();

        // getting function type
        std::string type = "";
        if (!type_(&type))
            error("Expected return type for function definition", type);
        if (!endOfLineAndOrComment())
            error("Expected end of line after function return type", "");;
        
        // getting the body
        NodePtrList body;
        while (true)
        {
            MaybeNodePtr inst = parseInstruction();

            // after getting the instruction, check if it's valid
            if (inst)
            {
                // if we found a 'end' token, stop
                if (inst.value()->nodename == "end")
                    break;
                body.push_back(inst.value());
            }
            else
                error("Expected valid instruction for body of function definition", "");
        }

        if (capture.empty())
            return std::make_shared<Function>(arguments, type, body);
        else
            return std::make_shared<Closure_>(capture, arguments, type, body);
    }

    MaybeNodePtr Parser::parseEnd()
    {
        /*
            Trying to parse 'end' tokens
        */

        inlineSpace();

        std::string keyword = "";
        if (!name(&keyword))
            return {};
        if (keyword != "end")
            return {};
        
        auto temp = std::make_shared<End>();
        if (!endOfLineAndOrComment())
            error("Expected end of line after keyword end", "");
        return temp;
    }

    MaybeNodePtr Parser::parseIf()
    {
        /*
            Trying to parse:

            if exp then
                exps*
            elif exp then
                exps*
            else
                exps*
            end
        */

        inlineSpace();

        std::string keyword = "";
        if (!name(&keyword))
            return {};
        if (keyword != "if")
            return {};
        
        // parse condition
        if (auto exp = parseExp())
        {
            // parse 'then'
            keyword = "";
            if (!name(&keyword) || keyword != "then")
                error("Expecting 'then' keyword after condition in if-clause", keyword);
            
            bool has_elifs = false;
            bool has_else = false;

            // read body
            NodePtrList body;
            while (true)
            {
                MaybeNodePtr inst = parseInstruction();

                // after getting the instruction, check if it's valid
                if (inst)
                {
                    // if we found a 'end' token, stop
                    if (inst.value()->nodename == "end")
                        break;
                    else if (inst.value()->nodename == "elif")
                    {
                        has_elifs = true;
                        break;
                    }
                    else if (inst.value()->nodename == "else")
                    {
                        has_else = true;
                        break;
                    }
                    body.push_back(inst.value());
                }
                else
                    error("Expected valid instruction for body of if", "");
            }

            // no elifs or else, just return the if
            if (!has_elifs && !has_else)
                return std::make_shared<IfClause>(exp.value(), body, NodePtrList{}, NodePtrList{});
            
            NodePtrList elifClauses;

            if (!has_elifs && has_else)
                goto label_parse_else;

            // if then elif ...
            if (has_elifs)
            {
                // read all the elifs
                while (true)
                {
                    has_elifs = has_else = false;

                    // read condition
                    if (auto cond2 = parseExp())
                    {
                        // parse 'then'
                        keyword = "";
                        if (!name(&keyword) || keyword != "then")
                            error("Expecting 'then' keyword after condition in if-clause", keyword);

                        // read body
                        NodePtrList bodyElif;
                        while (true)
                        {
                            MaybeNodePtr inst = parseInstruction();

                            // after getting the instruction, check if it's valid
                            if (inst)
                            {
                                // if we found a 'end' token, stop
                                if (inst.value()->nodename == "end")
                                    break;
                                else if (inst.value()->nodename == "elif")
                                {
                                    has_elifs = true;
                                    break;
                                }
                                else if (inst.value()->nodename == "else")
                                {
                                    has_else = true;
                                    break;
                                }
                                bodyElif.push_back(inst.value());
                            }
                            else
                                error("Expected valid instruction for body of if", "");
                        }

                        elifClauses.push_back(std::make_shared<IfClause>(cond2.value(), bodyElif, NodePtrList{}, NodePtrList{}));

                        if (!has_elifs)
                            break;
                    }
                    else
                        error("Expected valid expression as a condition for 'elif'", "");
                }
            }

        label_parse_else:
            if (has_else)
            {
                NodePtrList bodyElse;
                while (true)
                {
                    MaybeNodePtr inst = parseInstruction();

                    // after getting the instruction, check if it's valid
                    if (inst)
                    {
                        // if we found a 'end' token, stop
                        if (inst.value()->nodename == "end")
                            break;
                        bodyElse.push_back(inst.value());
                    }
                    else
                        error("Expected valid instruction for body of else", "");
                }

                return std::make_shared<IfClause>(exp.value(), body, elifClauses, bodyElse);
            }
            else
                return std::make_shared<IfClause>(exp.value(), body, elifClauses, NodePtrList{});
        }
        else
            error("Expected valid expression as a condition for 'if'", "");
        
        return {};
    }

    // TODO
    MaybeNodePtr Parser::parseWhile()
    {
        return {};
    }

    // TODO
    MaybeNodePtr Parser::parseImport()
    {
        return {};
    }
}