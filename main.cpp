#include <iostream>
#include <cassert>
#include <algorithm>
#include <string>
#include <vector>

#include "scriptingParser.h"
#include "scriptingVarIndexer.h"
#include "scriptingEvaluator.h"
#include "scriptingScenarios.h"
#include "scriptingProduct.h"
#include "scriptingDebugger.h"
#include "scriptingFuzzyEval.h"
#include "simpleAD.h"
#include <map>

using namespace std;

double runScript(const string& script, const string& varName)
{
    Event evt = parse(script);

    VarIndexer indexer;
    for (auto& stmt : evt) stmt->accept(indexer);
    vector<string> vars = indexer.getVarNames();

    Evaluator<double> eval(vars.size());
    Scenario<double> scen(1);
    scen[0] = {0.0, 1.0};
    eval.setScenario(&scen);
    eval.init();
    eval.setCurEvt(0);
    for (auto& stmt : evt) stmt->accept(eval);

    auto it = find(vars.begin(), vars.end(), varName);
    if (it == vars.end()) throw runtime_error("variable not found");
    return eval.varVals()[distance(vars.begin(), it)];
}

void testForInLoop()
{
    const string script =
        "x = 0\n"
        "for v in [1,2,3]:\n"
        " x = v\n"
        "end\n";
    assert(runScript(script, "x") == 3);
}

void testForRangeLoop()
{
    const string script =
        "x = 0\n"
        "for i = 0 to 2:\n"
        " x = i\n"
        "end\n";
    assert(runScript(script, "x") == 2);
}

void testInstrumentPricing()
{
    const string script =
        "price = 0\n"
        "for p in [100,50,20]:\n"
        " price = price + p\n"
        "end\n";
    assert(runScript(script, "price") == 170);
}

void testProductEvaluation()
{
    Tape tape;
    Number::tape = &tape;

    Date today = 1;
    std::map<Date, string> mapping = {
        {today, "x=0"},
        {today + 1, "y=0"},
        {today + 2, "if x > 0 then y=1 else y=0 endif"}
    };

    Product prod;
    prod.parseEvents(mapping.begin(), mapping.end());
    prod.preProcess(true, false);
    Debugger d;
    prod.visit(d);
    cout << d.getString() << endl;
    unique_ptr<Scenario<Number>> scenario = prod.buildScenario<Number>();
    FuzzyEvaluator<Number> evaluator = prod.buildFuzzyEvaluator<Number>(prod.ifProcess(), 1);

    prod.evaluate(*scenario, evaluator);
    vector<Number> values = evaluator.varVals();
    vector<string> var_names = prod.varNames();
    for (size_t i = 0; i < values.size(); i++)
    {
        cout << var_names[i] << ": " << static_cast<double>(values[i]) << endl;
    }

    values.back().propagateToStart();

    cout << values[0].adjoint() << endl;
}

int main()
{
    testForInLoop();
    testForRangeLoop();
    testInstrumentPricing();
    testProductEvaluation();
    cout << "All tests passed" << endl;
    return 0;
}

