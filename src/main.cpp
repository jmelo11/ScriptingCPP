#include <iostream>
#include <cstring>
#include "scriptingParser.h"
#include "scriptingProduct.h"
#include "visitorHeaders.h"
#include "AADNumber.h"

using Date = int;
int main()
{
    Tape tape;
    Number::tape = &tape;

    Date today = 1;
    std::map<Date, string> mapping = {{today, "x=0"}, {today + 1, "y=0"}, {today + 2, "if x > 0 then y=1 else y=0 endif"}};

    Product prod;
    prod.parseEvents(mapping.begin(), mapping.end());
    prod.preProcess(true, false);
    Debugger d;
    prod.visit(d);
    cout << d.getString() << endl;
    std::unique_ptr<Scenario<Number>> scenario = prod.buildScenario<Number>();
    FuzzyEvaluator<Number> evaluator = prod.buildFuzzyEvaluator<Number>(prod.ifProcess(), 1);

    prod.evaluate(*scenario, evaluator);
    std::vector<Number> values = evaluator.varVals();
    std::vector<string> var_names = prod.varNames();
    for (size_t i = 0; i < values.size(); i++)
    {
        cout << var_names[i] << ": " << values[i] << endl;
    };

    values.back().propagateToStart();

    cout << values[0].adjoint() << endl;
}
