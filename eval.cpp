#include <iostream>
#include <stack>
#include "val.h"


int precedence(string op) {
    if(op == "(") {
        return -1;
    }
    if(op == ">" || op == "<" || op == "==") {
        return 0;
    }
    if(op == "+" || op == "-" || op == "//") {
        return 1;
    }
    if(op == "*" || op == "/") {
        return 2;
    }
    if(op == "**") {
        return 3;
    }
    return -10; //arbitrary, does nothing
}

Value reduce(Value operand1, Value operand2, string op) {
    if(op == "+") {
        return operand1 + operand2;
    } if(op == "-") {
        return operand1 - operand2;
    } if(op == "//") {
        return operand1.Catenate(operand2);
    } if(op == "*") {
        return operand1 * operand2;
    } if(op == "/") {
        return operand1 / operand2;
    } if(op == "**") {
        return operand1.Power(operand2);
    } if(op == "==") {
        return operand1 == operand2;
    } if(op == "<") {
        return operand1 < operand2;
    } if(op == ">") {
        return operand1 > operand2;
    } else {
        cout << "OPERATOR (" << op << ") NOT RECOGNIZED" << endl;
        return Value();
    }
}

bool isOperator(string op) {
    return (
            op == "+"
            || op == "-"
            || op == "/"
            || op == "//"
            || op == "*"
            || op == "**"
            || op == "=="
            || op == "<"
            || op == ">");
}

void addOperatorAndEvaluateStacks(stack<Value>& operands, stack<string>& operators, string curOp) {
    //for reducing stacks
    string doOp;
    Value op1;
    Value op2;

    //logic
    if(curOp == "(") {
        operators.push(curOp);
    }
    else if(curOp == ")") {

        while (operators.top() != "(") {
            doOp = operators.top();
            operators.pop();

            op2 = operands.top();
            operands.pop();

            op1 = operands.top();
            operands.pop();

            operands.push(reduce(op1, op2, doOp));
        }
        operators.pop(); //gets rid of "(" from stack
    }
    else{
        while( !operators.empty() && precedence(curOp) <= precedence(operators.top()) ) {
            doOp = operators.top();
            operators.pop();

            op2 = operands.top();
            operands.pop();

            op1 = operands.top();
            operands.pop();

            operands.push(reduce(op1, op2, doOp));
        }
        operators.push(curOp);
    }
}

//use for abstraction but realistically only appends to the stacks
void addOperandAndEvaluateStacks(stack<Value>& operands, stack<string>& operators, Value& operand) {
    operands.push(operand);
}

void evaluateStackRemains(stack<Value>& operands, stack<string>& operators) {
    string doOp;
    Value op1;
    Value op2;
    while(!operators.empty() && operands.size() >= 2) {
        doOp = operators.top();
        operators.pop();

        op2 = operands.top();
        operands.pop();

        op1 = operands.top();
        operands.pop();

        operands.push(reduce(op1, op2, doOp));
    }
}

void resetStacks(stack<Value>& operands, stack<string>& operators) {
    while(!operands.empty()) {
        operands.pop();
    }
    while(!operators.empty()) {
        operators.pop();
    }
}