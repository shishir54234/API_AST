#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <bits/stdc++.h>

namespace common
{

    // Enum declarations
    enum class BoolConditionType
    {
        AND,
        OR,
        NEGATION
    };
    enum class BooleanSetOperator
    {
        EQUAL_TO,
        IN,
        NOT_IN
    };
    enum class ActionSetOperator
    {
        UNION,
        INTERSECTION,
        DIFFERENCE,
        COMPLEMENT
    };
    enum class Operator
    {
        UNION,
        INTERSECTION,
        DIFFERENCE,
        COMPLEMENT,
        EQUAL_TO,
        IN,
        NOT_IN,
        AND,
        OR,
        NEGATION
    };
    enum class HTTPResponseCode
    {
        OK,
        NOT_FOUND,
        SERVER_ERROR
    };
    enum class VarTypes
    {
        String,
        Integer,
        Set
    };
    enum class SpecialValues
    {
        Empty_Set
    };

    // Function declaration
    void printIndent(int indent);

    // Input class
    class Input
    {
    public:
        bool globalVar;
        std::string id;
        VarTypes dtype;

        explicit Input(bool chk, std::string id, VarTypes dtype);
        void print() const;
        std::string toString() const;
    };

    // Inputs class
    class Inputs
    {
    public:
        std::vector<Input> inputs;
        void addInput(Input input);
        void print() const;
        std::string toString() const;
    };

    // Abstract base class for expressions
    class Expression
    {
    public:
        virtual ~Expression() = default;
        virtual void print(int indent = 0) const = 0;
        virtual std::string toString(int indent = 0) const = 0;
        virtual std::unique_ptr<Expression> clone() const = 0;
    };

    // ValueExpression class 
    // U = {}
    class ValueExpression : public Expression
    {
        

    public:
        common::SpecialValues s;
        explicit ValueExpression(SpecialValues s1);
        void print(int indent = 0) const override;
        std::unique_ptr<Expression> clone() const override;
        std::string toString(int indent = 0) const override;
    };

    // VarExpression class
    // Input 
    // (uid, p) not in U 

    // (uid1, p1) not in U
    class VarExpression : public Expression
    {
    public:
        
        Inputs i;

        VarExpression(Inputs i1);
        Inputs getInput();
        void print(int indent = 0) const override;
        std::unique_ptr<Expression> clone() const override;
        std::string toString(int indent = 0) const override;
    };

    // SetOperationExpression class
    class SetOperationExpression : public Expression
    {
        

    public:
        std::unique_ptr<Expression> left;
        Operator op;
        std::unique_ptr<Expression> right;
        SetOperationExpression(std::unique_ptr<Expression> left, Operator op, std::unique_ptr<Expression> right);
        void print(int indent = 0) const override;
        std::string toString(int indent = 0) const override;
        std::unique_ptr<Expression> clone() const override;
        std::unique_ptr<Expression> &getLeft();
        std::unique_ptr<Expression> &getRight();
        Operator getOp() const;

    private:
        static std::string operatorToString(Operator op);
    };

} // namespace common

#endif // COMMON_H
