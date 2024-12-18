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
    Input::Input(bool chk, std::string id, VarTypes dtype) : globalVar(chk), id(std::move(id)), dtype(dtype) {}

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
    VarExpression::VarExpression(Inputs i1) : i(std::move(i1)) {}

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

    Inputs VarExpression::getInput()
    {
        return i;
    }

    

    void VarExpression::print(int indent) const
    {
        printIndent(indent);
        i.print();
    }

    std::string VarExpression::toString(int indent) const
    {
        return std::string(indent, ' ') + ", Inputs: [" + i.toString() + "])";
    }

    // SetOperationExpression implementations
    SetOperationExpression::SetOperationExpression(std::unique_ptr<Expression> left, Operator op, std::unique_ptr<Expression> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}

    

    void SetOperationExpression::print(int indent) const
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

    std::string SetOperationExpression::toString(int indent) const
    {
        return std::string(indent, ' ') + "(" + (left ? left->toString() : "NULL") + " " +
               operatorToString(op) + " " + (right ? right->toString() : "NULL") + ")";
    }

    std::unique_ptr<Expression> &SetOperationExpression::getLeft()
    {
        return left;
    }

    std::unique_ptr<Expression> &SetOperationExpression::getRight()
    {
        return right;
    }

    Operator SetOperationExpression::getOp() const
    {
        return op;
    }

    std::string SetOperationExpression::operatorToString(Operator op)
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
    std::unique_ptr<Expression> SetOperationExpression::clone() const
    {
        return std::make_unique<SetOperationExpression>(
            left ? left->clone() : nullptr,
            op,
            right ? right->clone() : nullptr);
    }
    std::unique_ptr<Expression> ValueExpression::clone() const
    {
        return std::make_unique<ValueExpression>(s);
    }
} // namespace common
