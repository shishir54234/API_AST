#include "atc.h"
#include <iostream>

namespace atc
{

    // API Implementation
    API::API(std::vector<common::Input> inputs, common::HTTPResponseCode code,
             std::vector<common::Input> outputs)
        : inputs(std::move(inputs)), responseCode(code), outputs(std::move(outputs)) {}

    void API::print(int indent)
    {
        std::cout << "\n(";

        for (auto &i : inputs)
            std::cout << i.id << " ,";
        std::cout << ")" << std::endl;
        common::printIndent(indent + 1);
        std::cout << "==>";

        switch (responseCode)
        {
        case common::HTTPResponseCode::OK:
            std::cout << "OK";
            break;
        case common::HTTPResponseCode::NOT_FOUND:
            std::cout << "Not Found";
            break;
        case common::HTTPResponseCode::SERVER_ERROR:
            std::cout << "Server Error";
            break;
        }

        std::cout << " ";
        if (!outputs.empty())
        {
            common::printIndent(indent + 1);
            for (auto &output : outputs)
                std::cout << output.id << " ";
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    // Conditions Implementation
    Conditions::Conditions(int fl1) : fl(fl1) {}

    void Conditions::addCondition(std::unique_ptr<common::Expression> condition,
                                  common::Operator op)
    {
        Expr = std::make_unique<common::SetOperationExpression>(move(condition), op, move(Expr));
    }

    void Conditions::print(int indent) const
    {
        if (fl)
            std::cout << "ASSERT(";
        else
            std::cout << "ASSUME(";
        Expr->print(indent);
        std::cout << ")" << std::endl;
    }

    std::string Conditions::toString(int indent) const
    {
        std::string result;
        if (fl)
            result += "ASSERT(";
        else
            result += "ASSUME(";

        result = Expr->toString(indent);
        result += ")";
        return result;
    }

    std::string Conditions::boolConditionTypeToString(common::Operator type)
    {
        switch (type)
        {
        case common::Operator::AND:
            return "AND";
        case common::Operator::OR:
            return "OR";
        case common::Operator::NEGATION:
            return "NOT";
        }
        return "";
    }

    // ATC Implementation
    ATC::ATC(std::vector<common::Input> ins1, Conditions pre,
             std::unique_ptr<API> api, Conditions post)
        : ins(std::move(ins1)), preConditions(std::move(pre)), api(std::move(api)), postConditions(std::move(post)) {}

    void ATC::print(int indent)
    {
        common::printIndent(indent);
        for (auto &j : ins)
            j.print();
        preConditions.print(indent + 1);
        api->print(indent + 1);
        postConditions.print(indent + 1);
    }

    std::string ATC::toString()
    {
        // TODO: Implement toString method
        return "";
    }

} // namespace atc