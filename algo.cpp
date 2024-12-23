#include <bits/stdc++.h>
#include <memory>
#include "apispec.h"
#include "atc.h"
#include "common.h"

using namespace std;
// 1. convert api spec to atc
// i wrote an algorithm 
// (uid, p) not in U

class SymbolTable
{
public:
    // Key for symbol table lookup: (index, variable_id)
    using VariableKey = std::pair<size_t, std::string>;

    struct KeyHash
    {
        std::size_t operator()(const VariableKey &k) const
        {
            return std::hash<size_t>()(k.first) ^
                   std::hash<std::string>()(k.second);
        }
    };

    struct Symbol
    {
        enum Type
        {
            INPUT,
            OUTPUT,
            GLOBAL,
            TEMPORARY
        };

        Type type;
        std::string originalId;
        std::string scopedId;
        bool isGlobal;

        Symbol(Type t, std::string orig, std::string scoped, bool global = false)
            : type(t), originalId(std::move(orig)),
              scopedId(std::move(scoped)), isGlobal(global) {}
    };

private:
    std::unordered_map<VariableKey, Symbol, KeyHash> symbols;
    size_t currentIndex;
    SymbolTable *parentTable;

public:
    explicit SymbolTable(size_t index = 0, SymbolTable *parent = nullptr)
        : currentIndex(index), parentTable(parent) {}

    void registerVariable(const std::string &varId, Symbol::Type type, bool isGlobal)
    {
        if (isGlobal && parentTable)
        {
            parentTable->registerVariable(varId, type, true);
            return;
        }

        VariableKey key{currentIndex, varId};
        std::string scopedId;

        if (isGlobal)
        {
            scopedId = varId; // Global variables keep their original ID
        }
        else
        {
            scopedId = varId + "_" + std::to_string(currentIndex);
        }

        symbols.emplace(key, Symbol(type, varId, scopedId, isGlobal));
    }

    std::string getScopedId(const std::string &originalId) const
    {
        // First check current scope
        VariableKey key{currentIndex, originalId};
        auto it = symbols.find(key);
        if (it != symbols.end())
        {
            return it->second.scopedId;
        }

        // If not found and parent exists, check parent scope
        if (parentTable)
        {
            return parentTable->getScopedId(originalId);
        }

        throw std::runtime_error("Variable not found: " + originalId);
    }

    bool exists(const std::string &originalId) const
    {
        VariableKey key{currentIndex, originalId};
        return symbols.find(key) != symbols.end() ||
               (parentTable && parentTable->exists(originalId));
    }

    const Symbol *getSymbol(const std::string &originalId) const
    {
        VariableKey key{currentIndex, originalId};
        auto it = symbols.find(key);
        if (it != symbols.end())
        {
            return &it->second;
        }
        return parentTable ? parentTable->getSymbol(originalId) : nullptr;
    }
};

// Modified modify function to use symbol table for variable scoping
unique_ptr<common::Expression> modify(unique_ptr<common::Expression> &expr, const SymbolTable &symbolTable)
{
    if (!expr)
    {
        // throw std::runtime_error("Null expression encountered");
        return nullptr;
    }

    if (auto *setOpExpr = dynamic_cast<common::SetOperationExpression *>(expr.get()))
    {
        
        return std::make_unique<common::SetOperationExpression>(
            modify(setOpExpr->getLeft(), symbolTable),
            setOpExpr->getOp(),
            modify(setOpExpr->getRight(), symbolTable));
    }
    else if (auto *VarExpr = dynamic_cast<common::VarExpression *>(expr.get()))
    {
        common::Inputs i = VarExpr->getInput();
        common::Inputs newInputs;

        for (const common::Input &i1 : i.inputs)
        {
            cout << i1.toString() << "\n";
            common::Input newInput = i1;
            if (symbolTable.exists(i1.id))
            {
                // Use symbol table to get the scofped ID
                newInput.id = symbolTable.getScopedId(i1.id);
            }
            newInputs.inputs.push_back(newInput);
        }
        return make_unique<common::VarExpression>(newInputs);
    }
    else if (auto *ValExpr = dynamic_cast<common::ValueExpression *>(expr.get()))
    {
        common::SpecialValues s1 = ValExpr->s;
        return make_unique<common::ValueExpression>(s1);
    }

    // Print type information before throwing the error
    cout << "Unknown expression type encountered: "
         << typeid(*expr).name() << "\n";

    throw std::runtime_error("Unknown expression type: " +
                             std::string(typeid(*expr).name()));
}

// Modified convert function to use symbol table for variable scoping
vector<atc::ATC> convert(vector<apispec::API_Spec> &specs)
{
    vector<atc::ATC> atcs;
    atcs.reserve(specs.size());

    for (size_t i = 0; i < specs.size(); i++)
    {
        // Create symbol table for this API spec with current index
        SymbolTable symbolTable(i + 1);

        // Register inputs in symbol table
        for (const auto &input : specs[i].api->inputs)
        {
            symbolTable.registerVariable(
                input.id,
                SymbolTable::Symbol::Type::INPUT,
                input.globalVar != 0);
        }

        // Register outputs in symbol table
        for (const auto &output : specs[i].api->outputs)
        {
            symbolTable.registerVariable(
                output.id,
                SymbolTable::Symbol::Type::OUTPUT,
                output.globalVar != 0);
        }

        // Process inputs using symbol table
        vector<common::Input> ins1;
        ins1.reserve(specs[i].api->inputs.size());

        for (const auto &input : specs[i].api->inputs)
        {
            common::Input modified_input = input;
            if (!modified_input.globalVar)
            {
                modified_input.id = symbolTable.getScopedId(input.id);
            }
            ins1.push_back(std::move(modified_input));
        }

        // Process outputs using symbol table
        vector<common::Input> outs1;
        outs1.reserve(specs[i].api->outputs.size());

        for (const auto &output : specs[i].api->outputs)
        {
            common::Input modified_output = output;
            if (!modified_output.globalVar)
            {
                modified_output.id = symbolTable.getScopedId(output.id);
            }
            outs1.push_back(std::move(modified_output));
        }

        // Create API object
        auto api1 = std::make_unique<atc::API>(
            std::move(ins1),
            specs[i].api->responseCode,
            std::move(outs1));

        // Handle conditions using symbol table
        atc::Conditions pre1(0);
        pre1.Expr = modify(specs[i].preConditions.Expr, symbolTable);
        pre1.print(0);
        atc::Conditions post1(1);
        post1.Expr = modify(specs[i].postConditions.Expr, symbolTable);

        // Create and add ATC object
        atcs.emplace_back(
            vector<common::Input>(ins1),
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
    U_inputs.addInput(common::Input(1, "U", common::VarTypes::Set));
    // (uid, pass) not in U 
    common::Inputs inp;
    inp.addInput(common::Input(0, "uid", common::VarTypes::String));
    inp.addInput(common::Input(0, "pass", common::VarTypes::String));

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
    auto left_var_expr = make_unique<common::VarExpression>(inp); // (uid,p)
    auto right_var_expr = std::make_unique<common::VarExpression>(U_inputs); 

    // Create a SetOperationExpression
    auto set_op_expr = std::make_unique<common::SetOperationExpression>(
        std::move(left_var_expr),
        common::Operator::NOT_IN,
        std::move(right_var_expr));

    // Create Conditions
    apispec::Conditions pre;
    pre.Expr=move(set_op_expr);
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