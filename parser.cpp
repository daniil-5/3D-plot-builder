#include "parser.h"


bool IsUnary(const std::string& c)
{
    return ( c == "s" || c == "c" || c == "t" || c == "l") ;
}
bool isOperator(const std::string& c)
{
    return (c == "+" || c == "-" || c == "*" || c == "/" || c == "^" || c == "s" || c == "c" || c == "t" || c == "l");
}

bool isnumber(char c)
{
    return (isdigit(c) || c == '.');
}

float operation(double a, double b, char op)
{
    //Perform operation
    if(op == '+')
        return b+a;
    else if(op == '-')
        return b-a;
    else if(op == '*')
        return b*a;
    else if(op == '/')
        return b/a;
    else if(op == '^')
        return pow(b,a);
    else if (op == 's')
        return sin(a);
    else if (op == 'c')
        return cos(a);
    else if (op == 't')
        return tan(a);
    else if (op == 'l')
        return log(a);
    else
        std::throw_with_nested("Invalid operator");
}

int precedence(std::string c)
{
    if(c == "^")
        return 3;
    else if(c == "*" || c == "/")
        return 1;
    else if(c == "s" || c == "c" || c == "t" || c == "l")
        return 2;
    else if(c == "+" || c == "-")
        return 0;
    else
        return -1;
}

void replaceAll(std::string &str, const std::string &from, const std::string &to)
{
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

static std::vector<std::string> tokenize(std::string expression)
{
    bool unarmin = false;
    std:: vector<std::string> tokens;
    std::string currentToken;
    replaceAll(expression,"sin","s");
    replaceAll(expression,"cos","c");
    replaceAll(expression,"tan","t");
    replaceAll(expression,"log","l");

    for (int i = 0;i < expression.size();i++)
    {
        char c = expression.at(i);
        if(i == 0 && c == '-')
        {
            unarmin = true;
            continue;
        }

        if(c == ')' || c == '(')
        {
            tokens.emplace_back(1,c);
            if(i!= expression.size() - 1 && expression.at(i+1) == '-')
            {
                i++;
                unarmin = true;
            }
            continue;
        }
        if (isspace(c) && !currentToken.empty())
        {
            tokens.push_back(currentToken);
            currentToken.clear();
        }
        else if (isOperator(std::string(1,c)))
        {
            if (!currentToken.empty())
            {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
            tokens.emplace_back(1, c);
        }
        else if (isnumber(c) || c == 'x' || c == 'y')
        {
            if(unarmin)
            {
                currentToken += "-";
                unarmin = false;
            }
            if(expression.size() > 1 && i != 0 && (expression.at(i-1) == 'x' or expression.at(i-1) == 'y'))
            {
                std::throw_with_nested("Invalid expression");
            }
            currentToken += c;
            if(i != expression.size() - 1 && !isnumber(expression.at(i+1)))
            {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
        }
    }
    if (!currentToken.empty())
    {
        tokens.push_back(currentToken);
    }
    return tokens;
}

bool isValidEquation(const std::string& equation)
{
    int balance = 0;
    for (char c : equation) {
        if (c == '(') {
            balance++;
        } else if (c == ')') {
            balance--;
            if (balance < 0) {
                return false; // closing parenthesis before opening one
            }
        } else if (!isalnum(c) && c != '+' && c != '-' && c != '*' && c != '/' && c != '^' && c != 's' && c != 'c' && c != 't' && c != 'l' && c != 'x' && c != 'y' && c != ' ') {
            return false; // invalid character
        }
    }
    if (balance != 0) {
        return false; // unbalanced parentheses
    }

    // Check for valid structure (no two operators can be adjacent, an operator cannot be the first or last character, etc.)
    std::string prevToken;
    std::vector<std::string> tokens = tokenize(equation);
    for (const std::string& token : tokens) {
        if (isOperator(token)) {
            if (prevToken.empty() || isOperator(prevToken)) {
                return false; // two operators are adjacent or an operator is the first character
            }
        }
        prevToken = token;
    }
    if (isOperator(prevToken)) {
        return false; // an operator is the last character
    }

    return true;
}

std::vector<std::string> infixToPostfix(std::string infix)
{
    std::stack<std::string> operations;
    std::vector<std::string> input = tokenize(infix);
    int l = input.size();
    std::vector<std::string> postfix;
    for(int i = 0; i < l; i++)
    {
        if((input[i] == "x" || input[i] == "y" || input[i] == "-x" || input[i] == "-y") or isnumber(input.at(i).at(0)) or (input.at(i).at(0) == '-' && input.at(i).size() >=2 && (isnumber(input.at(i).at(1)))))
        {
            postfix.push_back(input.at(i));
        }
        else if(input[i] == "(")
        {
            operations.push("(");
        }
        else if(input[i] == ")")
        {
            while(!operations.empty() && operations.top() != "(")
            {
                std::string c = operations.top();
                operations.pop();
                postfix.push_back(c);
            }
            if(operations.top() == "(")
            {
                std::string c = operations.top();
                operations.pop();
            }
        }
        else
        {
            while(!operations.empty() && precedence(input.at(i)) <= precedence(operations.top()))
            {
                std::string c = operations.top();
                operations.pop();
                postfix.push_back(c);
            }
            operations.push(input[i]);
        }
    }
    while(!operations.empty())
    {
        std::string c = operations.top();
        operations.pop();
        postfix.push_back(c);
    }
    return postfix;
}

float Calc(float x, float y,std::string& infix)
{
    std::vector <std::string> postfixExpr = infixToPostfix(infix);
    std::stack <float> operands;
    for (int i = 0; i < postfixExpr.size(); ++i)
    {
        std::string c = postfixExpr[i];
        if(!isOperator(c))
        {
            if(c == "x" or c == "-x")
            {
                int a = 1;
                if(c.at(0) == '-')
                    a = -1;
                operands.push(x * a);
            }
            else if(c == "y" or c == "-y")
            {
                int a = 1;
                if(c.at(0) == '-')
                    a = -1;
                operands.push(y * a);
            }
            else
            {
                operands.push(std::stof(c));
            }
        }
        else
        {
            float a = operands.top();
            operands.pop();
            if(IsUnary(c))
            {
                operands.push(operation(a, 0, c.at(0)));
            }
            else
            {
                if(operands.empty())
                {
                    throw std::runtime_error("Invalid expression: not enough operands for operator " + c);
                }
                float b = operands.top();
                operands.pop();
                operands.push(operation(a, b, c.at(0)));
            }
        }
    }
    return operands.top();
}

Parser::Parser() = default;

float Parser::getvalue(float x, float y)
{
    return Calc(x,y,this->m_expr);
}

void Parser::setEquatation(std::string &expr)
{
    this->m_expr = expr;
}

