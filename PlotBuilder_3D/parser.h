#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <stack>
#include <string>

#include "QtCore/qtmetamacros.h"

class Parser
{
public:
    Parser();
    float getvalue(float x,float y);
    void setEquatation(std::string& expr);
private:
    std::string m_expr;
};
#endif // PARSER_H
