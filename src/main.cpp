#include <iostream>
#include <cstring>
#include "scriptingParser.h"
#include "scriptingProduct.h"
#include "visitorHeaders.h"

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
    std::unique_ptr<Scenario<double>> scenario = prod.buildScenario<double>();
    Evaluator<double> evaluator = prod.buildEvaluator<double>();
    prod.evaluate(*scenario, evaluator);
    std::vector<double> values = evaluator.varVals();
    std::vector<string> var_names = prod.varNames();
    for (size_t i = 0; i < values.size(); i++)
    {
        cout << var_names[i] << ": " << values[i] << endl;
    };
}
