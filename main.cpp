#include <iostream>
#include "scriptingParser.h"
#include "scriptingVarIndexer.h"
#include "scriptingEvaluator.h"
#include "scriptingScenarios.h"

int main() {
    // Simple one-line script using the provided language
    const std::string script = "VALUE PAYS SPOT;";

    // Parse the script into an event (list of statements)
    Event evt = parse(script);

    // Index variables so that evaluator knows their positions
    VarIndexer indexer;
    for (auto& stmt : evt) {
        stmt->accept(indexer);
    }
    std::vector<std::string> varNames = indexer.getVarNames();

    // Build an evaluator and a dummy scenario
    Evaluator<double> eval(varNames.size());
    Scenario<double> scen(1);
    scen[0].spot = 100.0;
    scen[0].numeraire = 1.0;

    // Evaluate all statements
    eval.setScenario(&scen);
    eval.init();
    eval.setCurEvt(0);
    for (auto& stmt : evt) {
        stmt->accept(eval);
    }

    // Print variable results
    const auto& vals = eval.varVals();
    for (size_t i = 0; i < varNames.size(); ++i) {
        std::cout << varNames[i] << " = " << vals[i] << std::endl;
    }

    return 0;
}
