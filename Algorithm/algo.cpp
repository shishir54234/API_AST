#include <bits/stdc++.h>
#include <memory>
#include "../APISpecLang/apispec.h"
#include "../ATCLang/atc.h"
#include "../common/common.h"

using namespace std;

class SymbolTable
{
public:
    using VariableKey = std::pair<size_t, std::string>;
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

    std::map<VariableKey, Symbol> symbols;
    size_t currentIndex;
    SymbolTable *parentTable;

    explicit SymbolTable(size_t index = 0, SymbolTable *parent = nullptr)
        : currentIndex(index), parentTable(parent) {}

    std::string getScopedId(const std::string &originalId) const
    {
        VariableKey key{currentIndex, originalId};
        auto it = symbols.find(key);
        if (it != symbols.end())
        {
            return it->second.scopedId;
        }
        if (parentTable)
        {
            return parentTable->getScopedId(originalId);
        }
        throw std::runtime_error("Variable not found: " + originalId);
    }

    bool isGlobal(const std::string &originalId) const
    {
        if (parentTable)
        {
            return parentTable->isGlobal(originalId);
        }
        return exists(originalId);
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

unique_ptr<common::Expression> modify(unique_ptr<common::Expression> &expr, const SymbolTable &symbolTable, string s)
{
    if (!expr)
    {
        return nullptr;
    }

    if (auto *setOpExpr = dynamic_cast<common::BinaryExpression *>(expr.get()))
    {
        return std::make_unique<common::BinaryExpression>(
            modify(setOpExpr->getLeft(), symbolTable,s),
            setOpExpr->getOp(),
            modify(setOpExpr->getRight(), symbolTable,s));
    }
    else if (auto *VarExpr = dynamic_cast<common::VarExpression *>(expr.get()))
    {
        if(symbolTable.exists(VarExpr->i.id)){

            return make_unique<common::VarExpression>(VarExpr->i.id+s);
        }
    }
    else if (auto *ValExpr = dynamic_cast<common::ValueExpression *>(expr.get()))
    {
        return make_unique<common::ValueExpression>(ValExpr->s);
    }
    else if (auto TupExpr= dynamic_cast<common::TupleExpression *>(expr.get())){
        vector<unique_ptr<common::Expression>> new_exprs;
        for (auto &expr : TupExpr->i)
        {
            new_exprs.push_back(modify(expr, symbolTable,s));
        }
        return make_unique<common::TupleExpression>(new_exprs);
    }

    cout << "Unknown expression type encountered: " << typeid(*expr).name() << "\n";
    throw std::runtime_error("Unknown expression type: " + std::string(typeid(*expr).name()));
}

// Modified convert function to accept vector of symbol table pointers
vector<atc::ATC> convert(vector<apispec::API_Spec> &specs, const vector<SymbolTable *> &symbolTables)
{
    if (specs.size() != symbolTables.size())
    {
        throw std::runtime_error("Number of specs doesn't match number of symbol tables");
    }

    vector<atc::ATC> atcs;
    atcs.reserve(specs.size());

    for (size_t i = 0; i < specs.size(); i++)
    {
        const SymbolTable *currentSymbolTable = symbolTables[i];

        // Process inputs using current symbol table
        vector<common::Input> ins1;
        ins1.reserve(specs[i].api->inputs.size());

        for (const auto &input : specs[i].api->inputs)
        {
            common::Input modified_input = input;
            if (currentSymbolTable->exists(input.id))
            {
                modified_input.id = currentSymbolTable->getScopedId(input.id);
            }
            ins1.push_back(std::move(modified_input));
        }

        // Process outputs using current symbol table
        vector<common::Input> outs1;
        outs1.reserve(specs[i].api->outputs.size());

        for (const auto &output : specs[i].api->outputs)
        {
            common::Input modified_output = output;
            if (currentSymbolTable->exists(output.id))
            {
                modified_output.id = currentSymbolTable->getScopedId(output.id);
            }
            outs1.push_back(std::move(modified_output));
        }

        // Create API object
        auto api1 = std::make_unique<atc::API>(
            std::move(ins1),
            specs[i].api->responseCode,
            std::move(outs1));

        // Handle conditions using current symbol table
        atc::Conditions pre1(0);
        pre1.Expr = modify(specs[i].preConditions.Expr, *currentSymbolTable,to_string(i));
        pre1.print(0);

        atc::Conditions post1(1);
        post1.Expr = modify(specs[i].postConditions.Expr, *currentSymbolTable, to_string(i));

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
    // Create parent symbol table for global symbols
    auto parentTable = new SymbolTable(0);
    parentTable->symbols.emplace(
        SymbolTable::VariableKey{0, "U"},
        SymbolTable::Symbol(SymbolTable::Symbol::Type::GLOBAL, "U", "U", true));

    // Create symbol table for spec[0]
    auto symbolTable1 = new SymbolTable(1, parentTable);
    symbolTable1->parentTable = parentTable;
    // 1 for
    symbolTable1->symbols.emplace(
        SymbolTable::VariableKey{1, "uid"},
        SymbolTable::Symbol(SymbolTable::Symbol::Type::INPUT, "uid", "uid_1", false));
    symbolTable1->symbols.emplace(
        SymbolTable::VariableKey{1, "pass"},
        SymbolTable::Symbol(SymbolTable::Symbol::Type::INPUT, "pass", "pass_1", false));

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
    auto left_var_expr = make_unique<common::VarExpression>(inp);
    auto right_var_expr = std::make_unique<common::VarExpression>(U_inputs);

    // Create a SetOperationExpression
    auto set_op_expr = std::make_unique<common::BinaryExpression>(
        std::move(left_var_expr),
        common::Operator::NOT_IN,
        std::move(right_var_expr));

    // Create Conditions
    apispec::Conditions pre;
    pre.Expr = move(set_op_expr);
    apispec::Conditions post;

    // Create API_Spec
    apispec::API_Spec spec("signup", std::move(pre), std::move(api), std::move(post));

    // Create and process specs vector
    vector<apispec::API_Spec> specs;
    specs.push_back(std::move(spec));

    // Create vector of symbol table pointers
    vector<SymbolTable *> symbolTables{symbolTable1};

    // Convert and print using the symbol tables
    vector<atc::ATC> atcs = convert(specs, symbolTables);
    atcs[0].print();

    // Cleanup
    delete symbolTable1;
    delete parentTable;

    return 0;
}