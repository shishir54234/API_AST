#ifndef APISPEC_H
#define APISPEC_H

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include "common.h"

namespace apispec
{

    class Conditions
    {
    public:
        std::unique_ptr<common::Expression> Expr;
        void addCondition(std::unique_ptr<common::Expression> condition,
                          common::Operator op);
        void print(int indent = 0) const;
        std::string toString(int indent = 0) const;

    private:
        static std::string boolConditionTypeToString(common::BoolConditionType type);
    };

    class API
    {
    public:
        std::string id;
        std::vector<common::Input> inputs;
        common::HTTPResponseCode responseCode;
        std::vector<common::Input> outputs;

        API(std::string id, std::vector<common::Input> inputs,
            common::HTTPResponseCode code, std::vector<common::Input> outputs);

        void print(int indent = 0) const;
        std::string toString(int indent = 0) const;

    private:
        static std::string httpResponseCodeToString(common::HTTPResponseCode code);
    };

    class API_Spec
    {
    public:
        std::string id;
        Conditions preConditions;
        std::unique_ptr<API> api;
        Conditions postConditions;

        API_Spec(std::string id, Conditions pre,
                 std::unique_ptr<API> api, Conditions post);

        void print(int indent = 0) const;
        std::string toString(int indent = 0) const;
    };
    // U={} , Program={}
    class Initialization
    {
    public:
        std::vector<std::unique_ptr<common::Expression>> init;

        void addStatement(std::unique_ptr<common::Expression> stmt);
        void print(int indent = 0) const;
        std::string toString(int indent = 0) const;
    };

    class TestString
    {
    public:
        std::vector<std::unique_ptr<API_Spec>> specifications;

        void addSpecification(std::unique_ptr<API_Spec> spec);
        void print(int indent = 0) const;
        std::string toString(int indent = 0) const;
    };

} // namespace apispec

#endif // APISPEC_H