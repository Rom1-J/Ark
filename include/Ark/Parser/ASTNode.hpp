#ifndef ark_internal_astnode
#define ark_internal_astnode

#include <string>
#include <vector>
#include <memory>
#include <iostream>

namespace Ark::internal
{
    // base for all the nodes of the AST (Abstract Syntax Tree) of a Kafe program
    struct ASTNode
    {
        ASTNode(const std::string& nodename);

        const std::string nodename;

        virtual void toString(std::ostream& os, std::size_t indent) = 0;
    };

    using NodePtr = std::shared_ptr<ASTNode>;
    using NodePtrList = std::vector<NodePtr>;

    // basic class to hold all the sub-nodes
    struct Program : public ASTNode
    {
        Program();

        /*
            To add a ASTNode to a program:
            program.append<NodeType>(arg1, arg2);
        */
        template <typename T, typename... Args>
        void append(Args&&... args)
        {
            children.push_back(
                std::move(
                    std::make_shared<T>(std::forward<Args>(args)...)
                    )
                );
        }

        NodePtrList children;

        virtual void toString(std::ostream& os, std::size_t indent);
    };

    // ASTNode handling definition, i.e. let name = value
    struct Let : public ASTNode
    {
        Let(const std::string& varname, NodePtr value);

        const std::string varname;
        NodePtr value;

        virtual void toString(std::ostream& os, std::size_t indent);
    };

    // ASTNode handling definition: mut name = value
    struct Mut : public ASTNode
    {
        Mut(const std::string& varname, NodePtr value);

        const std::string varname;
        NodePtr value;

        virtual void toString(std::ostream& os, std::size_t indent);
    };

    struct Assignment : public ASTNode
    {
        Assignment(const std::string& varname, NodePtr value, const std::string& op);

        const std::string varname;
        NodePtr value;
        const std::string op;

        virtual void toString(std::ostream& os, std::size_t indent);
    };

    // ASTNode handling function: fun name(arg1: A, arg2: B) -> C *body* end
    struct Function : public ASTNode
    {
        Function(NodePtrList arguments, const std::string& type, NodePtrList body);

        NodePtrList arguments;  // should be a vector of declaration
        const std::string type;
        NodePtrList body;

        virtual void toString(std::ostream& os, std::size_t indent);
    };

    struct Closure : public ASTNode
    {
        Closure(NodePtrList capture, NodePtrList arguments, const std::string& type, NodePtrList body);

        NodePtrList capture;
        NodePtrList arguments;  // should be a vector of declaration
        const std::string type;
        NodePtrList body;

        virtual void toString(std::ostream& os, std::size_t indent);
    };

    struct ClosureFieldCall : public ASTNode
    {
        ClosureFieldCall(const std::string& objectname, const std::string& funcname, NodePtrList args);

        const std::string objectname;
        const std::string funcname;
        NodePtrList args;

        virtual void toString(std::ostream& os, std::size_t indent);
    };

    struct ClosureFieldRead : public ASTNode
    {
        ClosureFieldRead(const std::string& objectname, const std::string& field);

        const std::string objectname;
        const std::string field;

        virtual void toString(std::ostream& os, std::size_t indent);
    };

    /*
        ASTNode handling:

        if cond then
            stuff here
        elif cond2 then
            stuff here
        elif cond3 then
            stuff here
        ...
        else
            stuff
        end
    */
    struct IfClause : public ASTNode
    {
        IfClause(NodePtr condition, NodePtrList body, NodePtrList elifClause, NodePtrList elseClause);

        NodePtr condition;
        NodePtrList body;
        NodePtrList elifClause;  // should be a vector of ifclause (acting as elifs)
        NodePtrList elseClause;  // contains the body of the else clause

        virtual void toString(std::ostream& os, std::size_t indent);
    };

    /*
        ASTNode handling:

        while cond do
            stuff here
        end
    */
    struct WhileLoop : public ASTNode
    {
        WhileLoop(NodePtr condition, NodePtrList body);

        NodePtr condition;
        NodePtrList body;

        virtual void toString(std::ostream& os, std::size_t indent);
    };

    struct Integer : public ASTNode
    {
        Integer(int n);

        const int value;

        virtual void toString(std::ostream& os, std::size_t indent);
    };

    struct Float : public ASTNode
    {
        Float(float f);

        const float value;

        virtual void toString(std::ostream& os, std::size_t indent);
    };

    struct String : public ASTNode
    {
        String(const std::string& s);

        const std::string value;

        virtual void toString(std::ostream& os, std::size_t indent);
    };

    struct Bool : public ASTNode
    {
        Bool(bool b);

        const bool value;

        virtual void toString(std::ostream& os, std::size_t indent);
    };

    struct VarUse : public ASTNode
    {
        VarUse(const std::string& name);

        const std::string name;

        virtual void toString(std::ostream& os, std::size_t indent);
    };

    struct Operator : public ASTNode
    {
        Operator(const std::string& name);

        const std::string name;

        virtual void toString(std::ostream& os, std::size_t indent);
    };

    struct OperationsList : public ASTNode
    {
        OperationsList(NodePtrList operations);

        NodePtrList operations;

        virtual void toString(std::ostream& os, std::size_t indent);
    };

    struct FunctionCall : public ASTNode
    {
        FunctionCall(const std::string& name, NodePtrList arguments);

        const std::string name;
        NodePtrList arguments;

        virtual void toString(std::ostream& os, std::size_t indent);
    };

    // shouldn't be in the AST
    // just useful for code gen
    struct End : public ASTNode
    {
        End();

        virtual void toString(std::ostream& os, std::size_t indent);
    };

    struct Elif : public ASTNode
    {
        Elif();

        virtual void toString(std::ostream& os, std::size_t indent);
    };

    struct Else : public ASTNode
    {
        Else();

        virtual void toString(std::ostream& os, std::size_t indent);
    };

    struct Argument : public ASTNode
    {
        Argument(const std::string& varname, const std::string& type);

        const std::string varname;
        const std::string type;

        virtual void toString(std::ostream& os, std::size_t indent);
    };
}

#endif