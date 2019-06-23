#include <Ark/Parser/ASTNode.hpp>

namespace Ark::internal
{
    inline void printIndent(std::ostream& os, std::size_t level)
    {
        for (std::size_t i=0; i < level; ++i)
            os << "    ";
    }

    // ---------------------------

    ASTNode::ASTNode(const std::string& nodename) :
        nodename(nodename)
    {}

    // ---------------------------

    Program::Program() :
        ASTNode("program")
    {}

    void Program::print(std::ostream& os, std::size_t indent)
    {
        os << "(Program";
        for (auto& node: children)
        {
            os << "\n";
            node->print(os, indent + 1);
        }
        os << "\n)";
    }

    // ---------------------------

    Let::Let(const std::string& varname, NodePtr value) :
        varname(varname), value(std::move(value))
        , ASTNode("let")
    {}

    void Let::print(std::ostream& os, std::size_t indent)
    {
        printIndent(os, indent);     os << "(Let\n";
        printIndent(os, indent + 1);     os << "(VarName " << varname << ")\n";
                                        value->print(os, indent + 1); os << "\n";
        printIndent(os, indent);     os << ")";
    }

    // ---------------------------

    Mut::Mut(const std::string& varname, NodePtr value) :
        varname(varname), value(std::move(value))
        , ASTNode("mut")
    {}

    void Mut::print(std::ostream& os, std::size_t indent)
    {
        printIndent(os, indent);     os << "(Mut\n";
        printIndent(os, indent + 1);     os << "(VarName " << varname << ")\n";
                                        value->print(os, indent + 1); os << "\n";
        printIndent(os, indent);     os << ")";
    }

    // ---------------------------

    Assignment::Assignment(const std::string& varname, NodePtr value, const std::string& op) :
        varname(varname), value(std::move(value)), op(op)
        , ASTNode("assignment")
    {}

    void Assignment::print(std::ostream& os, std::size_t indent)
    {
        printIndent(os, indent);     os << "(Assignment\n";
        printIndent(os, indent + 1);     os << "(VarName " << varname << ")\n";
        printIndent(os, indent + 1);     os << op << "\n";
                                        value->print(os, indent + 1); os << "\n";
        printIndent(os, indent);     os << ")";
    }

    // ---------------------------

    Function::Function(NodePtrList arguments, const std::string& type, NodePtrList body) :
        arguments(std::move(arguments)), type(type), body(std::move(body))
        , ASTNode("function")
    {}

    void Function::print(std::ostream& os, std::size_t indent)
    {
        printIndent(os, indent);     os << "(Function\n";
        printIndent(os, indent + 1);     os << "(Args";
        for (auto& node: arguments)
        {
            os << "\n";
            node->print(os, indent + 2);
        }
        if (arguments.size() > 0)
        {
            os << "\n";
            printIndent(os, indent + 1);
        }
        os << ")\n";
        printIndent(os, indent + 1);     os << "(Type " << type << ")\n";
        printIndent(os, indent + 1);     os << "(Body";
        for (auto& node: body)
        {
            os << "\n";
            node->print(os, indent + 2);
        }
        if (body.size() > 0)
        {
            os << "\n";
            printIndent(os, indent + 1);
        }
        os << ")\n";
        printIndent(os, indent);     os << ")";
    }

    // ---------------------------

    Closure_::Closure_(NodePtrList capture, NodePtrList arguments, const std::string& type, NodePtrList body) :
        capture(std::move(capture)), arguments(std::move(arguments)), type(type), body(std::move(body))
        , ASTNode("closure")
    {}

    void Closure_::print(std::ostream& os, std::size_t indent)
    {
        printIndent(os, indent);     os << "(Closure\n";
        printIndent(os, indent + 1);     os << "(Capture";
        for (auto& node: capture)
        {
            os << "\n";
            node->print(os, indent + 2);
        }
        if (capture.size() > 0)
        {
            os << "\n";
            printIndent(os, indent + 1);
        }
        os << ")\n";
        printIndent(os, indent + 1);     os << "(Args";
        for (auto& node: arguments)
        {
            os << "\n";
            node->print(os, indent + 2);
        }
        if (arguments.size() > 0)
        {
            os << "\n";
            printIndent(os, indent + 1);
        }
        os << ")\n";
        printIndent(os, indent + 1);     os << "(Type " << type << ")\n";
        printIndent(os, indent + 1);     os << "(Body";
        for (auto& node: body)
        {
            os << "\n";
            node->print(os, indent + 2);
        }
        if (body.size() > 0)
        {
            os << "\n";
            printIndent(os, indent + 1);
        }
        os << ")\n";
        printIndent(os, indent);     os << ")";
    }

    // ---------------------------

    ClosureFieldCall::ClosureFieldCall(const std::string& objectname, const std::string& funcname, NodePtrList args) :
        objectname(objectname), funcname(funcname), args(std::move(args))
        , ASTNode("closure field call")
    {}

    void ClosureFieldCall::print(std::ostream& os, std::size_t indent)
    {
        printIndent(os, indent);     os << "(ClosureFieldCall\n";
        printIndent(os, indent + 1);     os << "(ObjectName " << objectname << ")\n";
        printIndent(os, indent + 1);     os << "(FuncName " << funcname << ")\n";
        printIndent(os, indent + 1);     os << "(Args";
        for (auto& node: args)
        {
            os << "\n";
            node->print(os, indent + 2);
        }
        if (args.size() > 0)
        {
            os << "\n";
            printIndent(os, indent + 1);
        }
        os << ")\n";
        printIndent(os, indent);     os << ")";
    }

    // ---------------------------

    ClosureFieldRead::ClosureFieldRead(const std::string& objectname, const std::string& field) :
        objectname(objectname), field(field)
        , ASTNode("closure field read")
    {}

    void ClosureFieldRead::print(std::ostream& os, std::size_t indent)
    {
        printIndent(os, indent);     os << "(ClosureFieldRead\n";
        printIndent(os, indent + 1);     os << "(ObjectName " << objectname << ")\n";
        printIndent(os, indent + 1);     os << "(Field " << field << ")\n";
        printIndent(os, indent);     os << ")";
    }

    // ---------------------------

    IfClause::IfClause(NodePtr condition, NodePtrList body, NodePtrList elifClause, NodePtrList elseClause) :
        condition(std::move(condition)), body(std::move(body)), elifClause(std::move(elifClause)), elseClause(std::move(elseClause))
        , ASTNode("if")
    {}

    void IfClause::print(std::ostream& os, std::size_t indent)
    {
        printIndent(os, indent);     os << "(IfClause)";
    }

    // ---------------------------

    WhileLoop::WhileLoop(NodePtr condition, NodePtrList body) :
        condition(std::move(condition)), body(std::move(body))
        , ASTNode("while")
    {}

    void WhileLoop::print(std::ostream& os, std::size_t indent)
    {
        printIndent(os, indent);     os << "(WhileLoop)";
    }

    // ---------------------------

    Integer::Integer(int n) :
        value(n)
        , ASTNode("integer")
    {}

    void Integer::print(std::ostream& os, std::size_t indent)
    {
        printIndent(os, indent);     os << "(Integer " << value << ")";
    }

    // ---------------------------

    Float::Float(float f) :
        value(f)
        , ASTNode("float")
    {}

    void Float::print(std::ostream& os, std::size_t indent)
    {
        printIndent(os, indent);     os << "(Float " << value << ")";
    }

    // ---------------------------

    String::String(const std::string& s) :
        value(s)
        , ASTNode("string")
    {}

    void String::print(std::ostream& os, std::size_t indent)
    {
        printIndent(os, indent);     os << "(String \"" << value << "\")";
    }

    // ---------------------------

    Bool::Bool(bool b) :
        value(b)
        , ASTNode("bool")
    {}

    void Bool::print(std::ostream& os, std::size_t indent)
    {
        printIndent(os, indent);     os << "(Bool " << (value ? "true" : "false") << ")";
    }

    // ---------------------------

    VarUse::VarUse(const std::string& name) :
        name(name)
        , ASTNode("var use")
    {}

    void VarUse::print(std::ostream& os, std::size_t indent)
    {
        printIndent(os, indent);     os << "(VarUse " << name << ")";
    }

    // ---------------------------

    Operator::Operator(const std::string& name) :
        name(name)
        , ASTNode("operator")
    {}

    void Operator::print(std::ostream& os, std::size_t indent)
    {
        printIndent(os, indent);     os << "(Operator " << name << ")";
    }

    // ---------------------------

    OperationsList::OperationsList(NodePtrList operations) :
        operations(std::move(operations))
        , ASTNode("op list")
    {}

    void OperationsList::print(std::ostream& os, std::size_t indent)
    {
        printIndent(os, indent);     os << "(OperationsList";
        for (auto& node: operations)
        {
            os << "\n";
            node->print(os, indent + 1);
        }
        if (operations.size() > 0)
            os << "\n";
        printIndent(os, indent);     os << ")";
    }

    // ---------------------------

    FunctionCall::FunctionCall(const std::string& name, NodePtrList arguments) :
        name(name), arguments(std::move(arguments))
        , ASTNode("function call")
    {}

    void FunctionCall::print(std::ostream& os, std::size_t indent)
    {
        printIndent(os, indent);     os << "(FunctionCall\n";
        printIndent(os, indent + 1);     os << "(Name " << name << ")\n";
        printIndent(os, indent + 1);     os << "(Args";
        for (auto& node: arguments)
        {
            os << "\n";
            node->print(os, indent + 2);
        }
        if (arguments.size() > 0)
        {
            os << "\n";
            printIndent(os, indent + 1);
        }
        os << ")\n";
        printIndent(os, indent);     os << ")";
    }

    // ---------------------------

    End::End() :
        ASTNode("end")
    {}

    void End::print(std::ostream& os, std::size_t indent)
    {
        printIndent(os, indent);     os << "(End)";
    }

    // ---------------------------

    Elif::Elif() :
        ASTNode("elif")
    {}

    void Elif::print(std::ostream& os, std::size_t indent)
    {
        printIndent(os, indent);     os << "(Elif)";
    }

    // ---------------------------

    Else::Else() :
        ASTNode("else")
    {}

    void Else::print(std::ostream& os, std::size_t indent)
    {
        printIndent(os, indent);     os << "(Else)";
    }

    // ---------------------------

    Argument::Argument(const std::string& varname, const std::string& type) :
        varname(varname), type(type)
        , ASTNode("argument")
    {}

    void Argument::print(std::ostream& os, std::size_t indent)
    {
        printIndent(os, indent);     os << "(Argument " << varname << " " << type << ")";
    }

    // ---------------------------

    Capture::Capture(const std::string& varname) :
        varname(varname)
        , ASTNode("capture")
    {}

    void Capture::print(std::ostream& os, std::size_t indent)
    {
        printIndent(os, indent);     os << "(Capture " << varname << ")";
    }
}