#ifndef ATC_H
#define ATC_H

#include <vector>
#include <memory>
#include <optional>
#include "common.h"

namespace atc
{

    class API
    {
    public:
        std::vector<common::Input> inputs;
        common::HTTPResponseCode responseCode;
        std::vector<common::Input> outputs;

        API(std::vector<common::Input> inputs, common::HTTPResponseCode code, std::vector<common::Input> outputs);
        void print(int indent = 0);
    };
// A u B u C , A u (B) , A and B or C
// A and (B or C) 
// A and (B == C)
    class Conditions
    {
    public:
        int fl;
        std::unique_ptr<common::Expression> Expr;
        void addCondition(std::unique_ptr<common::Expression> condition,
                          common::Operator op);

        explicit Conditions(int fl1);
        void print(int indent = 0) const;
        std::string toString(int indent = 0) const;

    private:
        static std::string boolConditionTypeToString(common::Operator type);
    };

    class ATC
    {
    public:
        std::vector<common::Input> ins; // uid, 
        Conditions preConditions;
        std::unique_ptr<API> api; // signup(input1, 2) => (rcode, outputs1, .. )
        Conditions postConditions;

        ATC(std::vector<common::Input> ins1, Conditions pre,
            std::unique_ptr<API> api, Conditions post);
        void print(int indent = 0);
        std::string toString();
    };

} // namespace atc

#endif  ATC_H