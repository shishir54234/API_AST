// input variables class , pre post almost same ,
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <variant>
#include <optional>

using namespace std;

class Expression;
class Statement;
class Condition;
class API_Spec;


struct Input{
    string id; 
    string dtype;
    
};
void print_inputs(vector<Input> inputs){
    for(auto &x : inputs){
        cout<<x.id<<"="<<"input"<<"<"<<x.dtype<<">"<<"\n";
    }
}
void print_inputs1(vector<Input> inputs){
    for(int i=0;i<inputs.size();i++){
        cout<<inputs[i].id<<(i==inputs.size()-1?"":",");
    }
}
// Enums for operators and HTTP codes
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
enum class HTTPResponseCode
{
    OK,
    NOT_FOUND,
    SERVER_ERROR
};
void printIndent(int indent)
{
    for (int i = 0; i < indent; ++i)
        cout << "  ";
}
// Base class for all AST nodes
class ASTNode
{
public:
    virtual ~ASTNode() = default;
    virtual void print(int indent = 0) const = 0;

protected:
    void printIndent(int indent) const
    {
        for (int i = 0; i < indent; ++i)
            cout << "  ";
    }
};

// Expression class representing Set operations
class Expression : public ASTNode
{
public:
    struct SetOperation
    {
        unique_ptr<Expression> left;
        ActionSetOperator op;
        unique_ptr<Expression> right;
    };

    variant<string, SetOperation> value; // string for Set/Var/Constant, SetOperation for operations

    explicit Expression(string val) : value(move(val)) {}
    Expression(unique_ptr<Expression> left, ActionSetOperator op, unique_ptr<Expression> right)
    {
        SetOperation operation{move(left), op, move(right)};
        value = move(operation);
    }

    void print(int indent = 0) const override
    {
        printIndent(indent);
        if (holds_alternative<string>(value))
        {
            cout << get<string>(value);
        }
        else
        {
            const auto &op = get<SetOperation>(value);
            op.left->print();
            switch (op.op)
            {
            case ActionSetOperator::UNION:
                cout << " UNION ";
                break;
            case ActionSetOperator::INTERSECTION:
                cout << " INTERSECTION ";
                break;
            case ActionSetOperator::DIFFERENCE:
                cout << " - ";
                break;
            case ActionSetOperator::COMPLEMENT:
                cout << " COMPLEMENT ";
                break;
            }
            op.right->print();
        }
    }
};

// Statement class
class Statement : public ASTNode
{
public:
    optional<string> dtype; // optional because assignment statements don't need dtype
    string id;
    unique_ptr<Expression> expression;

    Statement(optional<string> dtype, string id, unique_ptr<Expression> expr)
        : dtype(move(dtype)), id(move(id)), expression(move(expr)) {}

    void print(int indent = 0) const override
    {
        printIndent(indent);
        if (dtype)
            cout << *dtype << " ";
        cout << id << " = ";
        expression->print();
        cout << endl;
    }
};

// Condition class
class Condition : public ASTNode
{
public:

    variant<
        pair<string, pair<BooleanSetOperator, string>>, // For boolean-set operations
        unique_ptr<Statement>                           // For statement conditions
        >
        content;

    // Constructor for boolean-set operations
    Condition(string left, BooleanSetOperator op, string right)
        : content(make_pair(move(left), make_pair(op, move(right)))) {}

    // Constructor for statement conditions
    explicit Condition(unique_ptr<Statement> stmt)
        : content(move(stmt)) {}

    void print(int indent = 0) const override
    {
        printIndent(indent);
        if (holds_alternative<pair<string, pair<BooleanSetOperator, string>>>(content))
        {
            const auto &[left, opRight] = get<pair<string, pair<BooleanSetOperator, string>>>(content);
            const auto &[op, right] = opRight;
            cout << left << " ";
            switch (op)
            {
            case BooleanSetOperator::EQUAL_TO:
                cout << "equalTo";
                break;
            case BooleanSetOperator::IN:
                cout << "in";
                break;
            case BooleanSetOperator::NOT_IN:
                cout << "not-in";
                break;
            }
            cout << " " << right;
        }
        else
        {
            get<unique_ptr<Statement>>(content)->print(indent);
        }
    }
};

// Conditions class to handle multiple conditions with boolean operators
class Conditions : public ASTNode
{
public:
    int fl;
    vector<pair<unique_ptr<Condition>, optional<BoolConditionType>>> conditions;
    // idk how to make this work with brackets
    Conditions(int fl1){
        fl=fl1;
    }
    void addCondition(unique_ptr<Condition> condition, optional<BoolConditionType> boolOp = nullopt)
    {
        conditions.emplace_back(move(condition), boolOp);
    }

    void print(int indent = 0) const override
    {
        if(fl){
            cout<<"ASSERT(";
        }
        else{
            cout<<"ASSUME(";
        }
        for (const auto &[cond, boolOp] : conditions)
        {
            cond->print(indent);
            if (boolOp)
            {
                cout << " ";
                switch (*boolOp)
                {
                case BoolConditionType::AND:
                    cout << "AND";
                    break;
                case BoolConditionType::OR:
                    cout << "OR";
                    break;
                case BoolConditionType::NEGATION:
                    cout << "NOT";
                    break;
                }
            }
            // cout << endl;
        }
        cout<<")";
        cout<<endl;
    }
};

// API class
class API
{
public:
    string id;
    vector<Input> inputs;
    vector<Input> outputs; // pair of id and dtype
    // instead of vector<pair<>> we can just use vector<string> s

    API(string id, vector<Input> inputs, vector<Input> outputs)
        : id(move(id)), inputs(move(inputs)), outputs(move(outputs)) {}
    
    void print(int indent = 0) const
    {  
        cout<<"(";
        print_inputs1(outputs);
        cout<<")";

        cout<<"=";
        cout<<id;
        cout<<"(";
        print_inputs1(inputs);
        cout<<")\n";

    }
};

// API Specification class
class API_Spec : public ASTNode
{
public:
    string id;
    Conditions preConditions;
    unique_ptr<API> api;
    Conditions postConditions;

    API_Spec(string id, Conditions pre, unique_ptr<API> api, Conditions post)
        : id(move(id)), preConditions(move(pre)), api(move(api)), postConditions(move(post)) {}

    void print(int indent = 0) const override
    {
        printIndent(indent);
        preConditions.print(indent + 1);
        api->print(indent + 1);
        postConditions.print(indent + 1);
    }
};
class Initialization : public ASTNode
{
public:
    vector<unique_ptr<Statement>> init;
    void addStatement(unique_ptr<Statement> stmt)
    {
        init.push_back(move(stmt));
    }
    void print(int indent = 0) const override
    {
        printIndent(indent);
        cout << "Initialization:" << endl;
        for (const auto &stmt : init)
        {
            stmt->print(indent + 1);
            cout << endl;
        }
    }
};
class API_definition : public ASTNode
{
public:
    string id;
    vector<string> input_dtypes;
    HTTPResponseCode response;
    vector<string> output_dtypes;
    API_definition(string id, vector<string> input_dtypes, HTTPResponseCode response, vector<string> output_dtypes)
        : id(move(id)), input_dtypes(move(input_dtypes)), response(response), output_dtypes(move(output_dtypes)) {}
    void print(int indent = 0) const override
    {
        printIndent(indent);
        cout << "API: " << id << endl;
        printIndent(indent + 1);
        cout << ":";
        for (int i = 0; i < input_dtypes.size(); i++)
        {
            cout << input_dtypes[i] << (i == input_dtypes.size() - 1 ? "" : "*");
        }
        cout << "==>";

        cout << endl;
        printIndent(indent + 1);
        cout << "Response Code: ";
        switch (response)
        {
        case HTTPResponseCode::OK:
            cout << "OK";
            break;
        case HTTPResponseCode::NOT_FOUND:
            cout << "Not Found";
            break;
        case HTTPResponseCode::SERVER_ERROR:
            cout << "Server Error";
            break;
        }
        if (output_dtypes.size())
            cout << "*";
        for (int i = 0; i < output_dtypes.size(); i++)
        {
            cout << output_dtypes[i] << (i == output_dtypes.size() - 1 ? "" : "*");
        }
        cout << "\n";
    }
};
// Test string class to hold multiple API specifications
class TestString : public ASTNode
{
public:
    vector<unique_ptr<API_Spec>> specifications;

    void addSpecification(unique_ptr<API_Spec> spec)
    {
        specifications.push_back(move(spec));
    }

    void print(int indent = 0) const override
    {
        printIndent(indent);
        cout << "API specifications:" << endl;
        for (const auto &spec : specifications)
        {
            spec->print(indent + 1);
            cout << endl;
        }
    }
};


int main(){
    // input variables 
    vector<Input> inputs;
    inputs.push_back({"uid","string"});
    inputs.push_back({"p","string"});
    
    print_inputs(inputs);
    vector<Input> outs;
    outs.push_back({"rcode"});

    Conditions preConditions(0);
    preConditions.addCondition(
            make_unique<Condition>("{uid,p}", BooleanSetOperator::NOT_IN, "U"));
    auto api = make_unique<API>("Signup", inputs, outs);



    Conditions postConditions(1);
    postConditions.addCondition(
        make_unique<Condition>("{uid,p}", BooleanSetOperator::IN, "U"));
    auto apiSpec = make_unique<API_Spec>(
            "TestSpec",
            move(preConditions),
            move(api),
            move(postConditions));
    apiSpec.get()->print();
    
}
