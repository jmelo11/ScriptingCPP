#include <iostream>
#include <cstring>
#include "scriptingParser.h"
#include "scriptingProduct.h"
#include "visitorHeaders.h"
#include "AADNumber.h"

using Date = int;
int main()
{
    string s = "X = 3";
    Date today = 1;
    map<Date, string> mapping = {{today, s}};
    Product prod;
    prod.parseEvents(mapping.begin(), mapping.end());
    prod.indexVariables();
    Debugger d;
    prod.visit(d);
    cout << d.getString() << endl;
    std::unique_ptr<Scenario<Number>> scenario = prod.buildScenario<Number>();
    Evaluator<Number> evaluator = prod.buildEvaluator<Number>();
    prod.evaluate(*scenario, evaluator);
    std::vector<Number> values = evaluator.varVals();
    std::vector<string> var_names = prod.varNames();
    for (size_t i = 0; i < values.size(); i++)
    {
        cout << var_names[i] << ": " << values[i] << endl;
    };
}
