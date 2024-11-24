#include "apispec.h"
#include <iostream>

namespace apispec
{

    // Conditions Implementation
    void Conditions::addCondition(std::unique_ptr<common::Expression> condition,
                                  common::BoolConditionType *boolOp)
    {
        conditions.emplace_back(std::move(condition), boolOp);
    }

    void Conditions::print(int indent) const
    {
        for (const auto &condition : conditions)
        {
            const auto &cond = condition.first;
            const auto &boolOp = condition.second;
            cond->print(indent);
            if (boolOp)
            {
                std::cout << " " << boolConditionTypeToString(*boolOp);
            }
            std::cout << std::endl;
        }
    }

    std::string Conditions::toString(int indent) const
    {
        std::string result;
        for (const auto &condition : conditions)
        {
            const auto &cond = condition.first;
            const auto &boolOp = condition.second;
            result += cond->toString(indent);
            if (boolOp)
            {
                result += " " + boolConditionTypeToString(*boolOp);
            }
            result += "\n";
        }
        return result;
    }

    std::string Conditions::boolConditionTypeToString(common::BoolConditionType type)
    {
        switch (type)
        {
        case common::BoolConditionType::AND:
            return "AND";
        case common::BoolConditionType::OR:
            return "OR";
        case common::BoolConditionType::NEGATION:
            return "NOT";
        }
        return "";
    }

    // API Implementation
    API::API(std::string id, std::vector<common::Input> inputs,
             common::HTTPResponseCode code, std::vector<common::Input> outputs)
        : id(std::move(id)), inputs(std::move(inputs)), responseCode(code), outputs(std::move(outputs)) {}

    void API::print(int indent) const
    {
        common::printIndent(indent);
        std::cout << "API: " << id << std::endl;
        common::printIndent(indent + 1);
        std::cout << "Inputs: ";
        for (const auto &i : inputs)
            i.print();
        std::cout << std::endl;
        common::printIndent(indent + 1);
        std::cout << "Response Code: " << httpResponseCodeToString(responseCode) << std::endl;
        if (!outputs.empty())
        {
            common::printIndent(indent + 1);
            std::cout << "Outputs: ";
            for (const auto &output : outputs)
                output.print();
            std::cout << std::endl;
        }
    }

    std::string API::toString(int indent) const
    {
        std::string result = std::string(indent, ' ') + "API: " + id + "\n";
        result += std::string(indent + 2, ' ') + "Inputs: ";
        for (const auto &i : inputs)
            result += i.toString() + " ";
        result += "\n" + std::string(indent + 2, ' ') + "Response Code: " + httpResponseCodeToString(responseCode) + "\n";
        if (!outputs.empty())
        {
            result += std::string(indent + 2, ' ') + "Outputs: ";
            for (const auto &output : outputs)
                result += output.toString() + " ";
            result += "\n";
        }
        return result;
    }

    std::string API::httpResponseCodeToString(common::HTTPResponseCode code)
    {
        switch (code)
        {
        case common::HTTPResponseCode::OK:
            return "OK";
        case common::HTTPResponseCode::NOT_FOUND:
            return "Not Found";
        case common::HTTPResponseCode::SERVER_ERROR:
            return "Server Error";
        }
        return "";
    }

    // API_Spec Implementation
    API_Spec::API_Spec(std::string id, Conditions pre,
                       std::unique_ptr<API> api, Conditions post)
        : id(std::move(id)), preConditions(std::move(pre)), api(std::move(api)), postConditions(std::move(post)) {}

    void API_Spec::print(int indent) const
    {
        common::printIndent(indent);
        std::cout << "API Specification: " << id << std::endl;
        std::cout << "Pre-Conditions:" << std::endl;
        preConditions.print(indent + 1);
        api->print(indent + 1);
        std::cout << "Post-Conditions:" << std::endl;
        postConditions.print(indent + 1);
    }

    std::string API_Spec::toString(int indent) const
    {
        std::string result = std::string(indent, ' ') + "API Specification: " + id + "\n";
        result += "Pre-Conditions:\n" + preConditions.toString(indent + 2);
        result += api->toString(indent + 2);
        result += "Post-Conditions:\n" + postConditions.toString(indent + 2);
        return result;
    }

    // Initialization Implementation
    void Initialization::addStatement(std::unique_ptr<common::Expression> stmt)
    {
        init.push_back(std::move(stmt));
    }

    void Initialization::print(int indent) const
    {
        common::printIndent(indent);
        std::cout << "Initialization:" << std::endl;
        for (const auto &stmt : init)
        {
            stmt->print(indent + 1);
            std::cout << std::endl;
        }
    }

    std::string Initialization::toString(int indent) const
    {
        std::string result = std::string(indent, ' ') + "Initialization:\n";
        for (const auto &stmt : init)
        {
            result += stmt->toString(indent + 1) + "\n";
        }
        return result;
    }

    // TestString Implementation
    void TestString::addSpecification(std::unique_ptr<API_Spec> spec)
    {
        specifications.push_back(std::move(spec));
    }

    void TestString::print(int indent) const
    {
        common::printIndent(indent);
        std::cout << "Test String:" << std::endl;
        for (const auto &spec : specifications)
        {
            spec->print(indent + 1);
            std::cout << std::endl;
        }
    }

    std::string TestString::toString(int indent) const
    {
        std::string result = std::string(indent, ' ') + "Test String:\n";
        for (const auto &spec : specifications)
        {
            result += spec->toString(indent + 1) + "\n";
        }
        return result;
    }

} // namespace apispec