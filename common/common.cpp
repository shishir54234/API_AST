#include "common.h"
#include <bits/stdc++.h>
namespace common
{

    // Function implementation
    void printIndent(int indent)
    {
        for (int i = 0; i < indent; i++)
        {
            std::cout << " ";
        }
    }

    // Input class implementations
    Input::Input(std::string id, VarTypes dtype) :  id(std::move(id)), dtype(dtype) {}

    void Input::print() const
    {
        std::cout << (dtype == VarTypes::String ? "String " : dtype == VarTypes::Integer ? "Integer "
                                                                                         : "Set ")
                  << id << " ";
    }

    std::string Input::toString() const
    {
        return (dtype == VarTypes::String ? "String " : dtype == VarTypes::Integer ? "Integer "
                                                                                   : "Set ") +
               id;
    }

    // Inputs class implementations
    void Inputs::addInput(Input input)
    {
        inputs.push_back(std::move(input));
    }

    void Inputs::print() const
    {
        for (const auto &input : inputs)
        {
            input.print();
        }
    }

    std::string Inputs::toString() const
    {
        std::string result;
        for (const auto &input : inputs)
        {
            if (!result.empty())
            {
                result += ", ";
            }
            result += input.toString();
        }
        return result;
    }

    // ValueExpression implementations
    ValueExpression::ValueExpression(SpecialValues s1) : s(s1) {}

    void ValueExpression::print(int indent) const
    {
        printIndent(indent);
        std::cout << (s == SpecialValues::Empty_Set ? "{}" : "");
    }

    std::string ValueExpression::toString(int indent) const
    {
        return std::string(indent, ' ') + (s == SpecialValues::Empty_Set ? "{}" : "");
    }

    // VarExpression implementations
    VarExpression::VarExpression(common::Input i1) : i(std::move(i1)) {}

    // void VarExpression::modify(std::string s)
    // {
    //     for (auto &input : i.inputs)
    //     {
    //         if (globalVar == 0)
    //         {
    //             input.modify(s);
    //         }
    //     }
    // }

    void VarExpression::print(int indent) const
    {
        printIndent(indent);
        i.print();
    }

    std::string VarExpression::toString(int indent) const
    {
        return std::string(indent, ' ') + ", Inputs: [" + i.toString() + "])";
    }

    // BinaryExpression implementations
    BinaryExpression::BinaryExpression(std::unique_ptr<Expression> left, Operator op, std::unique_ptr<Expression> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}

    void BinaryExpression::print(int indent) const
    {
        printIndent(indent);
        std::cout << "(";
        if (left)
            left->print();
        std::cout << " " << operatorToString(op) << " ";
        if (right)
            right->print();
        std::cout << ")";
    }

    std::string BinaryExpression::toString(int indent) const
    {
        return std::string(indent, ' ') + "(" + (left ? left->toString() : "NULL") + " " +
               operatorToString(op) + " " + (right ? right->toString() : "NULL") + ")";
    }

    std::unique_ptr<Expression> &BinaryExpression::getLeft()
    {
        return left;
    }

    std::unique_ptr<Expression> &BinaryExpression::getRight()
    {
        return right;
    }

    Operator BinaryExpression::getOp() const
    {
        return op;
    }
    TupleExpression::TupleExpression(std::vector<std::unique_ptr<common::Expression>> i1) : i(std::move(i1)) {}

    void TupleExpression::print(int indent) const
    {
        printIndent(indent);
        std::cout << "(";
        for (const auto &expr : i)
        {
            expr->print();
        }
        std::cout << ")";
    }

    std::string TupleExpression::toString(int indent) const
    {
        std::string result = std::string(indent, ' ') + "(";
        for (const auto &expr : i)
        {
            result += expr->toString();
        }
        return result + ")";
    }

    std::string BinaryExpression::operatorToString(Operator op)
    {
        switch (op)
        {
        case Operator::UNION:
            return "UNION";
        case Operator::INTERSECTION:
            return "INTERSECTION";
        case Operator::DIFFERENCE:
            return "-";
        case Operator::COMPLEMENT:
            return "COMPLEMENT";
        case Operator::EQUAL_TO:
            return "=";
        case Operator::IN:
            return "IN";
        case Operator::NOT_IN:
            return "NOT IN";
        case Operator::AND:
            return "AND";
        case Operator::OR:
            return "OR";
        case Operator::NEGATION:
            return "NOT";
        default:
            return "UNKNOWN";
        }
    }
    std::unique_ptr<Expression> VarExpression::clone() const
    {
        return std::make_unique<VarExpression>(i);
    }
    std::unique_ptr<Expression> BinaryExpression::clone() const
    {
        return std::make_unique<BinaryExpression>(
            left ? left->clone() : nullptr,
            op,
            right ? right->clone() : nullptr);
    }
    std::unique_ptr<Expression> ValueExpression::clone() const
    {
        return std::make_unique<ValueExpression>(s);
    }
} // namespace common