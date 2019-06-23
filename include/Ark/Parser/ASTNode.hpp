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

        virtual void print(std::ostream& os, std::size_t indent) = 0;
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

        void print(std::ostream& os, std::size_t indent) override;
    };

    // ASTNode handling definition, i.e. let name = value
    struct Let : public ASTNode
    {
        Let(const std::string& varname, NodePtr value);

        const std::string varname;
        NodePtr value;

        void print(std::ostream& os, std::size_t indent) override;
    };

    // ASTNode handling definition: mut name = value
    struct Mut : public ASTNode
    {
        Mut(const std::string& varname, NodePtr value);

        const std::string varname;
        NodePtr value;

        void print(std::ostream& os, std::size_t indent) override;
    };

    struct Assignment : public ASTNode
    {
        Assignment(const std::string& varname, NodePtr value, const std::string& op);

        const std::string varname;
        NodePtr value;
        const std::string op;

        void print(std::ostream& os, std::size_t indent) override;
    };

    // ASTNode handling function: fun name(arg1: A, arg2: B) -> C *body* end
    struct Function : public ASTNode
    {
        Function(NodePtrList arguments, const std::string& type, NodePtrList body);

        NodePtrList arguments;  // should be a vector of declaration
        const std::string type;
        NodePtrList body;

        void print(std::ostream& os, std::size_t indent) override;
    };

    struct Closure_ : public ASTNode
    {
        Closure_(NodePtrList capture, NodePtrList arguments, const std::string& type, NodePtrList body);

        NodePtrList capture;
        NodePtrList arguments;  // should be a vector of declaration
        const std::string type;
        NodePtrList body;

        void print(std::ostream& os, std::size_t indent) override;
    };

    struct ClosureFieldCall : public ASTNode
    {
        ClosureFieldCall(const std::string& objectname, const std::string& funcname, NodePtrList args);

        const std::string objectname;
        const std::string funcname;
        NodePtrList args;

        void print(std::ostream& os, std::size_t indent) override;
    };

    struct ClosureFieldRead : public ASTNode
    {
        ClosureFieldRead(const std::string& objectname, const std::string& field);

        const std::string objectname;
        const std::string field;

        void print(std::ostream& os, std::size_t indent) override;
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

        void print(std::ostream& os, std::size_t indent) override;
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

        void print(std::ostream& os, std::size_t indent) override;
    };

    struct Integer : public ASTNode
    {
        Integer(int n);

        const int value;

        void print(std::ostream& os, std::size_t indent) override;
    };

    struct Float : public ASTNode
    {
        Float(float f);

        const float value;

        void print(std::ostream& os, std::size_t indent) override;
    };

    struct String : public ASTNode
    {
        String(const std::string& s);

        const std::string value;

        void print(std::ostream& os, std::size_t indent) override;
    };

    struct Bool : public ASTNode
    {
        Bool(bool b);

        const bool value;

        void print(std::ostream& os, std::size_t indent) override;
    };

    struct VarUse : public ASTNode
    {
        VarUse(const std::string& name);

        const std::string name;

        void print(std::ostream& os, std::size_t indent) override;
    };

    struct Operator : public ASTNode
    {
        Operator(const std::string& name);

        const std::string name;

        void print(std::ostream& os, std::size_t indent) override;
    };

    struct OperationsList : public ASTNode
    {
        OperationsList(NodePtrList operations);

        NodePtrList operations;

        void print(std::ostream& os, std::size_t indent) override;
    };

    struct FunctionCall : public ASTNode
    {
        FunctionCall(const std::string& name, NodePtrList arguments);

        const std::string name;
        NodePtrList arguments;

        void print(std::ostream& os, std::size_t indent) override;
    };

    // shouldn't be in the AST
    // just useful for code gen
    struct End : public ASTNode
    {
        End();

        void print(std::ostream& os, std::size_t indent) override;
    };

    struct Elif : public ASTNode
    {
        Elif();

        void print(std::ostream& os, std::size_t indent) override;
    };

    struct Else : public ASTNode
    {
        Else();

        void print(std::ostream& os, std::size_t indent) override;
    };

    struct Argument : public ASTNode
    {
        Argument(const std::string& varname, const std::string& type);

        const std::string varname;
        const std::string type;

        void print(std::ostream& os, std::size_t indent) override;
    };

    struct Capture : public ASTNode
    {
        Capture(const std::string& varname);

        const std::string varname;

        void print(std::ostream& os, std::size_t indent) override;
    };
}

#endif