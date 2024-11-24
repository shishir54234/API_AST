#include <bits/stdc++.h>
#include <memory>
#include "apispec.h"
#include "atc.h"
#include "common.h"

using namespace std;

vector<atc::ATC> convert(vector<apispec::API_Spec> &specs)
{
    vector<atc::ATC> atcs;
    atcs.reserve(specs.size());

    for (size_t i = 0; i < specs.size(); i++)
    {
        string id1 = specs[i].id;

        // Create copies of inputs and outputs
        vector<common::Input> ins1;
        ins1.reserve(specs[i].api->inputs.size());

        for (const auto &input : specs[i].api->inputs)
        {
            auto modified_input = input;
            modified_input.modify(to_string(i + 1));
            ins1.push_back(std::move(modified_input));
        }

        vector<common::Input> outs1;
        outs1.reserve(specs[i].api->outputs.size());

        for (const auto &output : specs[i].api->outputs)
        {
            auto modified_output = output;
            modified_output.modify(to_string(i + 1));
            outs1.push_back(std::move(modified_output));
        }

        // Create the API object
        auto api1 = std::make_unique<atc::API>(
            std::move(ins1),
            specs[i].api->responseCode,
            std::move(outs1));

        // Handle preconditions
        atc::Conditions pre1(0);
        for (const auto &condition : specs[i].preConditions.conditions)
        {
            auto expr_copy = condition.first->clone();
            expr_copy->modify(to_string(i + 1));
            pre1.addCondition(std::move(expr_copy), condition.second);
        }

        // Handle postconditions
        atc::Conditions post1(1);
        for (const auto &condition : specs[i].postConditions.conditions)
        {
            auto expr_copy = condition.first->clone();
            expr_copy->modify(to_string(i + 1));
            post1.addCondition(std::move(expr_copy), condition.second);
        }

        // Create and add the ATC object
        atcs.emplace_back(
            vector<common::Input>(ins1), // Create a fresh copy for ATC constructor
            std::move(pre1),
            std::move(api1),
            std::move(post1));
    }

    return atcs;
}

int main()
{
    // Create inputs
    common::Inputs U_inputs;
    U_inputs.addInput(common::Input("U", common::VarTypes::Set));

    common::Inputs inp;
    inp.addInput(common::Input("uid", common::VarTypes::String));
    inp.addInput(common::Input("pass", common::VarTypes::String));

    // Convert Inputs to vector<common::Input>
    vector<common::Input> input_vec;
    for (const auto &input : inp.inputs)
    {
        input_vec.push_back(input);
    }

    vector<common::Input> output_vec;

    // Create the API object
    auto api = std::make_unique<apispec::API>(
        "login",
        std::move(input_vec),
        common::HTTPResponseCode::OK,
        std::move(output_vec));

    // Create VarExpressions using Inputs
    auto left_var_expr = make_unique<common::VarExpression>(0, inp);
    auto right_var_expr = std::make_unique<common::VarExpression>(1, U_inputs);

    // Create a SetOperationExpression
    auto set_op_expr = std::make_unique<common::SetOperationExpression>(
        std::move(left_var_expr),
        common::Operator::NOT_IN,
        std::move(right_var_expr));

    // Create Conditions
    apispec::Conditions pre;
    pre.addCondition(std::move(set_op_expr), nullptr);

    apispec::Conditions post;

    // Create API_Spec
    apispec::API_Spec spec("signup", std::move(pre), std::move(api), std::move(post));

    // Create and process specs vector
    vector<apispec::API_Spec> specs;
    specs.push_back(std::move(spec)); // Move spec into vector

    // Convert and print
    vector<atc::ATC> atcs = convert(specs);
    atcs[0].print();

    return 0;
}