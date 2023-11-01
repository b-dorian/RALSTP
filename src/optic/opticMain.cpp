
/************************************************************************
 * Copyright 2012; Planning, Agents and Intelligent Systems Group,
 * Department of Informatics,
 * King's College, London, UK
 * http://www.inf.kcl.ac.uk/staff/andrew/planning/
 *
 * Amanda Coles, Andrew Coles - OPTIC
 * Amanda Coles, Andrew Coles, Maria Fox, Derek Long - POPF
 * Stephen Cresswell - PDDL Parser
 *
 * This file is part of OPTIC.
 *
 * OPTIC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * OPTIC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OPTIC.  If not, see <http://www.gnu.org/licenses/>.
 *
 ************************************************************************/

#include <cstdio>
#include <iostream>
#include <iomanip>
#include <fstream>
#include "ptree.h"
#include <assert.h>
#include <FlexLexer.h>
#include "instantiation.h"
#include "SimpleEval.h"
#include "DebugWriteController.h"
#include "typecheck.h"
#include "TIM.h"
#include "FuncAnalysis.h"
#include <chrono>

//#include "graphconstruct.h"
#include "RPGBuilder.h"
#include "FFSolver.h"
#include "globals.h"
#ifdef TOTALORDERSTATES
#include "colintotalordertransformer.h"
#else
#include "totalordertransformer.h"
#include "partialordertransformer.h"
#endif
#include "lpscheduler.h"
#include "numericanalysis.h"
#include "PreferenceHandler.h"

#ifdef STOCHASTICDURATIONS
#include "StochasticDurations.h"
#endif

#include <sys/times.h>

#include <sstream>
#include "temporalanalysis.h"
#include "TemporalLandmarksAndConstraints.h"
#include "landmarks/Graph.h"
#include <unistd.h>
#include <algorithm>
#include <random>

using std::ifstream;
using std::cerr;
using std::endl;
using std::ostringstream;
using std::istringstream;

using namespace TIM;
using namespace Inst;
using namespace VAL;
using namespace Planner;
using namespace temporal_landmarks;
using namespace strategic_tactical;
using namespace std;


map<double, RPGBuilder::ClusterDef > RPGBuilder::factLayerNodes;
map<double, RPGBuilder::ClusterDef > RPGBuilder::actionLayerNodes;
list<string> RPGBuilder::endActionsToStarts;
list<string> RPGBuilder::factsToActions;
map<int, string> RPGBuilder::achieverForFact;
map<string,string> rpgElements;
vector<string> factLayerZeroCodes;
map <string,std::vector<string>> objectsMap;
map <int,string> typeMap;
vector<VAL::proposition*> toplevel_goals;
vector<VAL::proposition*> reachability_facts;
//vector<pair<int,int>> agent_dependencies;
Graph* g;
std::time_t main_start;

//set<int> not_agents;
set<string> dynamic_types;
bool reachabilityAnalysis = true;


struct dependency {
    int parent;
    int child;
    vector<VAL::proposition*> not_found_goals; // not found clasical relaxation landmarks
};
vector<dependency> agent_dependencies;
vector<dependency> graph_dependencies;


vector<RPGBuilder::agent> agents;

namespace VAL
{
    extern yyFlexLexer* yfl;
    extern TypeChecker *theTC;
};

extern int yyparse();
extern int yydebug;




void getGraphandAgents(){
    agents.clear();
    g = new Graph(objectsMap.size(),typeMap);
    for (int i = 0; i < agent_dependencies.size(); i++){
        dependency dependency = agent_dependencies[i];
        g->addEdge(dependency.parent,dependency.child);
        cout << "added dependecy: " <<  typeMap[dependency.parent] << " -> " << typeMap[dependency.child] << endl;
        if(agents.size() == 0){
            cout << "first agents: " <<  typeMap[dependency.parent] << " -> " << typeMap[dependency.child] << endl;
            RPGBuilder::agent parent;
            parent.data = dependency.parent;
            parent.children.insert(dependency.child);
            agents.push_back(parent);
            RPGBuilder::agent child;
            child.data = dependency.child;
            child.parents.insert(dependency.parent);
            agents.push_back(child);
        }
        bool firstExists = false;
        bool secondExists = false;
        for(int i = 0; i < agents.size(); i++){
            if(agents[i].data == dependency.parent){
                agents[i].children.insert(dependency.child);
                secondExists = true;
            }
            if(agents[i].data == dependency.child){
                agents[i].parents.insert(dependency.parent);
                firstExists = true;
            }
        }
        if(!firstExists){
            cout << "added node based on dependency.child:  " << typeMap[dependency.child] << endl;
            RPGBuilder::agent child;
            child.data = dependency.child;
            child.parents.insert(dependency.parent);
            agents.push_back(child);
            for(int i = 0; i < agents.size(); i++){
                if(agents[i].data == dependency.parent)agents[i].children.insert(dependency.child);
            }
        }
        if(!secondExists){
            cout << "added node based on dependency.parent:  " << typeMap[dependency.parent] << endl;
            RPGBuilder::agent parent;
            parent.data = dependency.parent;
            parent.children.insert(dependency.child);
            agents.push_back(parent);
            for(int i = 0; i < agents.size(); i++){
                if(agents[i].data == dependency.child)agents[i].parents.insert(dependency.parent);
            }
        }
    }
}


string porpositionToString(VAL::proposition* prop){
    stringstream ss;
    ss << "(" << prop->head->getName();
    if(prop->args->begin() != prop->args->end()){
        for (VAL::parameter_symbol_list::iterator pit = prop->args->begin(); pit != prop->args->end(); pit++){
            VAL::parameter_symbol* param = *pit;
            ss << " " << param->getName();
        }
    }
    ss << ")";
    return ss.str();
}



//dependecy where goals are
//TODO change to searching goals, not abstract landmarks
bool haveSameBase(VAL::proposition* origin, VAL::proposition* target){
    stringstream origin_ss;
    origin_ss << origin->head->getName();
    for (VAL::parameter_symbol_list::iterator pit = origin->args->begin(); pit != origin->args->end(); pit++){
        VAL::parameter_symbol* param = *pit;
        origin_ss << " " << param->type->getName();
    }
    stringstream target_ss;
    target_ss << target->head->getName();
    for (VAL::parameter_symbol_list::iterator pit = target->args->begin(); pit != target->args->end(); pit++){
        VAL::parameter_symbol* param = *pit;
        target_ss << " " << param->type->getName();
    }
    //cout << "   checking: " << origin_ss.str() << " - " << target_ss.str() << endl;
    if(origin_ss.str().compare(target_ss.str()) == 0){
        //cout << "       found: " << origin_ss.str() << " - " << target_ss.str() << endl;
        return true;
    }
    return false;
}

//TODO add as a min_not_found_goal tiebreaker ??
bool containsTopLevelGoals(vector<VAL::proposition*> target_goals){
    for(int i = 0; i < target_goals.size(); i++){
        for(int j = 0; j < toplevel_goals.size(); j++){
            //cout << "   checking: " << porpositionToString(target_goals[i]) << " - " << porpositionToString(toplevel_goals[j]) << endl;
            if(porpositionToString(target_goals[i]).compare(porpositionToString(toplevel_goals[j])) == 0){
                //cout << "       found: " << porpositionToString(target_goals[i]) << " - " << porpositionToString(toplevel_goals[j]) << endl;
                return true;
            }
//            if(haveSameBase(target_goals[i],toplevel_goals[j])){
//                return true;
//            }
        }
    }
    return false;
}

void removeDependency(pair<int,int> conflict){
    for (vector<dependency>::iterator it = agent_dependencies.begin(); it != agent_dependencies.end();){
        dependency dependency = *it;
        if((dependency.parent == conflict.first) && (dependency.child == conflict.second))
            it = agent_dependencies.erase(it);
        else
            ++it;
    }
}

//TODO  implement >2 vertices in cyle method (vertice with lowets no of notfound goals is removed), it is in the commented method below
bool mitigateCycle(vector<pair<int,int>> cycleVertices){
    cout << "Attempting cycle mitigation..." << endl;
    vector<pair<int,int>> conflict;

    cout << "Conflict is between the following dependencies: " << endl;
    for (int i = 0; i < cycleVertices.size(); i++){
        cout << "   " << typeMap[cycleVertices[i].first] << " -> " << typeMap[cycleVertices[i].second] << endl;
    }
    cout << endl;

    vector<bool> conflictHasTopLevelGoals;
    for(int i = 0; i < agent_dependencies.size(); i++){
        for(int j = 0; j < cycleVertices.size(); j++){
            if((agent_dependencies[i].parent == cycleVertices[j].first) && (agent_dependencies[i].child == cycleVertices[j].second)){
                cout << "checking if '" << typeMap[cycleVertices[j].second] << "' facts are top level goals..." << endl;
                conflictHasTopLevelGoals.push_back(containsTopLevelGoals(agent_dependencies[i].not_found_goals));
            }
        }
    }
    for (int i = 0; i < cycleVertices.size(); i++){
        cout << typeMap[cycleVertices[i].second] << " " << conflictHasTopLevelGoals[i] << endl;
    }


    for (int i = 0; i < cycleVertices.size(); i++){
        for (int j = 0; j < cycleVertices.size(); j++){

            if (i == j) { continue;}
            if (conflictHasTopLevelGoals[i] && !conflictHasTopLevelGoals[j]){
                removeDependency(cycleVertices[j]);
                cout << "Minor impact dependency identified and removed: " << endl << "   " << typeMap[cycleVertices[j].first] << " -> " << typeMap[cycleVertices[j].second] << endl << endl;
                return true;
            }
            if (!conflictHasTopLevelGoals[i] && conflictHasTopLevelGoals[j]){
                removeDependency(cycleVertices[i]);
                cout << "Minor impact dependency identified and removed: " << endl << "   " << typeMap[cycleVertices[i].first] << " -> " << typeMap[cycleVertices[i].second] << endl << endl;
                return true;
            }
        }
    }

    return false;
}



// TODO NOT - mitigate cycle by evaluating all not found goals
// TODO implement multi-cycle analysis to determine least usefull relationships between cycles with common reltionships
// TODO implement containsTopLevelGoals() as a tie breaker in case 2 dependencies have the exact no of min_not_found_goals ??
//bool mitigateCycle(vector<pair<int,int>> cycleVertices){
//    cout << "Attempting cycle mitigation..." << endl;
//
//    //identify dependency to be removed
//    int target = 0;
//    int min_not_found_goals = agent_dependencies[0].not_found_goals.size();
//    cout << "Conflict is between the following dependencies: " << endl;
//    for (int i = 0; i < cycleVertices.size(); i++){
//        cout  << "   " << typeMap[cycleVertices[i].first] << " -> " << typeMap[cycleVertices[i].second] << endl;
//        for (int j = 0; j < agent_dependencies.size(); j++){
//            if ((agent_dependencies[j].parent == cycleVertices[i].first ) && (agent_dependencies[j].child == cycleVertices[i].second)){
//                cout << "   " << "not_found goals: " << agent_dependencies[j].not_found_goals.size() << endl;
//                if (agent_dependencies[j].not_found_goals.size() < min_not_found_goals){
//                    min_not_found_goals = agent_dependencies[j].not_found_goals.size();
//                    target = i;
//                }
//            }
//        }
//    }
//    cout << endl;
//
//    //check if min_not_found_goals is found in multiple dependencies
//    int found = 0;
//    for (int i = 0; i < cycleVertices.size(); i++){
//        for (int j = 0; j < agent_dependencies.size(); j++){
//            if ((agent_dependencies[j].parent == cycleVertices[i].first ) && (agent_dependencies[j].child == cycleVertices[i].second)){
//                if (agent_dependencies[j].not_found_goals.size() == min_not_found_goals){
//                    found++;
//                }
//            }
//        }
//    }
//    if (found > 1) return false;
//    else {
//        removeDependency(cycleVertices[target]);
//        cout << "Minor impact dependency identified and removed: " << endl << "   " << typeMap[cycleVertices[target].first] << " -> " << typeMap[cycleVertices[target].second] << endl << endl;
//        return true;
//    }
//}

bool processCycles(){
    getGraphandAgents();
    if(g->isCyclic()){
        cout << endl << "!!! Graph contains cycle !!!" << endl;
        if (!mitigateCycle(g->cycleVertices)) return false;
        return processCycles();
    }
    else {
        cout << endl << "Graph doesn't contain cycle" << endl;
        return true;
    }
}



namespace VAL
{
    bool ContinueAnyway;
    bool ErrorReport;
    bool InvariantWarnings;
    bool LaTeX;
    bool makespanDefault;
};



void usage(char * argv[])
{
    cout << "OPTIC: Optimising Preferences and Time-Dependant Costs\n";
    cout << "By releasing this code we imply no warranty as to its reliability\n";
    cout << "and its use is entirely at your own risk.\n\n";
    cout << "Usage: " << argv[0] << " [OPTIONS] domainfile problemfile [planfile, if -r specified]\n\n";

    cout << "Options are: \n\n";
    cout << "\t-N\tDon't optimise solution quality (ignores preferences and costs);\n";
    cout << "\t-0\tAbstract out timed initial literals that represent recurrent windows;\n";
    cout << "\t-n<lim>\tOptimise solution quality, capping cost at <lim>;\n\n";
    cout << "\t" << "-citation" << "\t" << "Display citation to relevant papers;\n";
    cout << "\t" << "-b" << "\t\t" << "Disable best-first search - if EHC fails, abort;\n";
    cout << "\t" << "-E" << "\t\t" << "Skip EHC: go straight to best-first search;\n";
    cout << "\t" << "-e" << "\t\t" << "Use standard EHC instead of steepest descent;\n";
    cout << "\t" << "-h" << "\t\t" << "Disable helpful-action pruning;\n";
    cout << "\t" << "-k" << "\t\t" << "Disable compression-safe action detection;\n";
    cout << "\t" << "-c" << "\t\t" << "Enable the tie-breaking in RPG that favour actions that slot into the partial order earlier;\n";
    cout << "\t" << "-S" << "\t\t" << "Sort initial layer facts in RPG by availability order (only use if using -c);\n";
    cout << "\t" << "-m" << "\t\t" << "Disable the tie-breaking in search that favours plans with shorter makespans;\n";
    cout << "\t" << "-F" << "\t\t" << "Full FF helpful actions (rather than just those in the RP applicable in the current state);\n";
    cout << "\t" << "-r" << "\t\t" << "Read in a plan instead of planning;\n";
    cout << "\t" << "-T" << "\t\t" << "Rather than building a partial order, build a total-order\n";
    cout << "\t" << "-v<n>" << "\t\t" << "Verbose to degree n (n defaults to 1 if not specified).\n";
    cout << "\t" << "-L<n>" << "\t\t" << "LP verbose to degree n (n defaults to 1 if not specified).\n";
};

list<FFEvent> * readPlan(char* filename);

void runCommand(string cmd){
    string data;
    FILE *stream;
    char buffer[1000];
    stream = popen(cmd.c_str(), "r");
    while ( fgets(buffer, 1000, stream) != NULL )
        data.append(buffer);
    pclose(stream);
}



void printLayerDef(const bool & factLayer, const double & atTime, const int & idx, const RPGBuilder::ClusterDef & def) {
//    cout << "\tsubgraph cluster_" << idx << " {\n";
//    if (factLayer) {
//        cout << "\t\tstyle=filled;\n";
//        cout << "\t\tcolor=lightgrey;\n";
//        cout << "\t\tnode [style=filled,color=white];\n";
//        cout << "\t\tlabel = \"\";\n";
//        cout << "\t\tc" << idx << " [label = \"fl(" << atTime << ")\"];\n";
//    } else {
//        cout << "\t\tnode [style=filled];\n";
//        cout << "\t\tcolor=blue;\n";
//        cout << "\t\tlabel = \"\";\n";
//        cout << "\t\tc" << idx << " [label = \"al(" << atTime << ")\"];\n";
//    }

    for (map<string,string>::const_iterator nItr = def.nodes.begin(); nItr != def.nodes.end(); ++nItr) {
        string code = nItr->first;
        std::string target = nItr->second;
        std::size_t pos = target.find("(");
        target = target.replace(0,pos,"");
        pos = target.find(")");
        rpgElements[code] = target.substr(0,pos+1);
        if(idx == 0)factLayerZeroCodes.push_back(code);
        //cout << "\t rpgElement entry " << code << " [" << rpgElements[code] << "];\n";
    }


    //cout << "\t}\n\n";
};

//string porpositionToStringOld(pair<string,VAL::parameter_symbol_list> goal){
//    stringstream ss;
//    ss << "(" << goal.first;
//    for (VAL::parameter_symbol_list::iterator pit = goal.second.begin(); pit != goal.second.end(); pit++){
//        VAL::parameter_symbol* param = *pit;
//        ss << " " << param->getName();
//    }
//    ss << ")";
//    return ss.str();
//}



string getInstanceType(string instance){
    string instance_type = "not_found";
    for (VAL::const_symbol_list::iterator oit = VAL::current_analysis->the_problem->objects->begin();
         oit != VAL::current_analysis->the_problem->objects->end(); oit++) {
        VAL::const_symbol *const_symbol_object = *oit;
        if(instance.compare(const_symbol_object->getName()) == 0){
            instance_type = const_symbol_object->type->getName();
            return instance_type;
        }
    }
    for (VAL::const_symbol_list::iterator oit = VAL::current_analysis->the_domain->constants->begin();
         oit != VAL::current_analysis->the_domain->constants->end(); oit++) {
        VAL::const_symbol *const_symbol_object = *oit;
        if(instance.compare(const_symbol_object->getName()) == 0){
            instance_type = const_symbol_object->type->getName();
            return instance_type;
        }
    }
    cout << "TYPE FOR " << instance << " NOT FOUND!!" << endl;
    return instance_type;
}

//made messy without using VAL::proposition due to goal adding bug
VAL::proposition* stringToProposition(string prop_string){
    //cout << "start string: " << prop_string << endl;
    pred_symbol* head;
    VAL::parameter_symbol_list *parameter_symbol_list = new VAL::parameter_symbol_list;
    if (prop_string.find(" ") != std::string::npos){
        std::size_t pos = prop_string.find(" ");
        head = new pred_symbol(prop_string.substr(1,pos-1));
        prop_string = prop_string.substr(pos,prop_string.size());
        prop_string = prop_string.erase(prop_string.size()-1);
        while(prop_string.find(" ") != std::string::npos){
            prop_string = prop_string.substr(1,prop_string.size());
            if (prop_string.find(" ") != std::string::npos){
                pos = prop_string.find(" ");
                string instance = prop_string.substr(0,pos);
                VAL::pddl_type* type = new VAL::pddl_type(getInstanceType(instance));
                VAL::parameter_symbol* param = new VAL::parameter_symbol(instance);
                param->type = type;
                parameter_symbol_list->push_back(param);
                prop_string = prop_string.substr(pos,prop_string.size());;

            }
            else{
                VAL::pddl_type* type = new VAL::pddl_type(getInstanceType(prop_string));
                VAL::parameter_symbol* param = new VAL::parameter_symbol(prop_string);
                param->type = type;
                parameter_symbol_list->push_back(param);
            }
        }
    }
    else{
        head = new pred_symbol(prop_string.substr(1,prop_string.length()-2));
    }
    VAL::proposition* prop = new VAL::proposition(head,parameter_symbol_list);
    //cout << "string to prop succesfull " << porpositionToString(prop)  << endl;
    return prop;
}

//bool hasDistinctTypes(pair<string,VAL::parameter_symbol_list> goal) {
//    for (VAL::parameter_symbol_list::iterator pit1 = goal.second.begin(); pit1 != goal.second.end(); pit1++){
//        VAL::parameter_symbol* param1 = *pit1;
//        for (VAL::parameter_symbol_list::iterator pit2 = goal.second.begin(); pit2 != goal.second.end(); pit2++){
//            VAL::parameter_symbol* param2 = *pit2;
//            if(param1->type->getName().compare(param2->type->getName()) != 0)return true;
//        }
//    }
//    return false;
//}

//bool hasRemovedTypeOld(pair<string,VAL::parameter_symbol_list> goal, string removed_type) {
//    for (VAL::parameter_symbol_list::iterator pit1 = goal.second.begin(); pit1 != goal.second.end(); pit1++){
//        VAL::parameter_symbol* param1 = *pit1;
//        if(param1->type->getName().compare(removed_type) == 0)return true;
//    }
//    return false;
//}

bool hasRemovedType(VAL::proposition* goal, string removed_type) {
    for (VAL::parameter_symbol_list::iterator pit1 = goal->args->begin(); pit1 != goal->args->end(); pit1++){
        VAL::parameter_symbol* param = *pit1;
        string type_name = param->type->getName();
        if(type_name.compare(removed_type) == 0){
            return true;
        }
    }
    return false;
}

/*
 * only predicates with 2 parameters different from each other;
 */
void rpgAnalysys(int parent, pair <string,std::vector<string>> removed_objects){
    int child = 0;
    int total_goals = 0;
    int ignored_goals = 0;
    int total_found_goals = 0;
    for (map <string,std::vector<string>>::iterator oit1 = objectsMap.begin(); oit1 != objectsMap.end(); oit1++){
        string agent_type = oit1->first;
        vector<string> instances = oit1->second;
        cout << "   facts with agent type : " << agent_type << "\n";
        int target_goals = 0;
        int found_goals = 0;
        set<string> other_types;
        //bool has_distinct_types = false;
        vector<VAL::proposition*> not_found_goals;
        for (int i = 0; i < reachability_facts.size(); i++) {
            string goal = porpositionToString(reachability_facts[i]);
            bool goal_found = false;
            bool instance_in_goal = false;
            bool goal_added = false;
            for (int j = 0; j < instances.size(); j++) {
                if(goal_found)break;
                if((goal.find(" "+instances[j]+" ") != std::string::npos) || (goal.find(" "+instances[j]+")") != std::string::npos)){
                    instance_in_goal = true;
                    if(!goal_added){
                        total_goals++;
                        target_goals++;
                        goal_added = true;
                        //cout << "target goal: " << goal << endl;
                    }

                    //if(hasDistinctTypes(reachability_facts[i]))has_distinct_types = true;
                    if(hasRemovedType(reachability_facts[i],typeMap[parent])){
                        target_goals--;
                        ignored_goals++;
                        goal_found = true;
                        //cout << "       ignored - contains the removed type: " << goal << endl;
                        break;
                    }
                }

                if(!instance_in_goal || goal_found) continue;
                for (map<int, string>::iterator affIt = RPGBuilder::achieverForFact.begin(); affIt != RPGBuilder::achieverForFact.end();affIt++){
                    string factCode = affIt->second;
                    std::size_t pos = factCode.find("> ");
                    factCode = factCode.replace(0,pos+2,"");
                    factCode = factCode.replace(factCode.length()-2,2,"");
                    if(goal.compare(rpgElements[factCode]) == 0){
                        found_goals++;
                        goal_found = true;
                        //cout << "       goal " << target_goals << " found: " << goal << " " << found_goals << "\n";
                        break;
                    }
                }
                //add goals solved in initial state
                if(goal_found)continue;
                for (int p = 0; p < factLayerZeroCodes.size(); p++){
                    if(goal.compare(rpgElements[factLayerZeroCodes[p]]) == 0){
                        bool has_removed_instance = false;
                        for(int q = 0; q < removed_objects.second.size(); q++){
                            string var1 = " "+removed_objects.second[q]+" ";
                            string var2 = " "+removed_objects.second[q]+")";
                            if((goal.find(var1) != std::string::npos) || (goal.find(var2) != std::string::npos)){
                                has_removed_instance = true;
                                break;
                            }
                        }
                        if(!has_removed_instance){
                            target_goals--;
                            ignored_goals++;
                            goal_found = true;
                            //cout << "       ignored - goal in initial state: " << goal << "\n";
                            break;
                        }

                    }
                }
                if(goal_found)break;
                if(instance_in_goal)break;
            }
            if(!goal_found && instance_in_goal){
                cout << "       not found - " << goal << endl;
                not_found_goals.push_back(reachability_facts[i]);
            }
        }
        total_found_goals = total_found_goals + found_goals;
        if(target_goals == 0){
            child++;
            cout << "no target facts" << endl;
            continue;
        }
        if(target_goals == found_goals){
            //agent_type NOT
            cout << "all " << agent_type << " facts found" << endl;
            //cout << agent_type << " NOT dependent on " << removed_objects.first << endl;
        }
        if((found_goals == 0)){
            cout << agent_type << " facts NOT found" << endl;
            cout << agent_type << " dependent on " << removed_objects.first << endl;
//            if (parent != child){
//                pair<int,int> dependency;
//                dependency.first = child;
//                dependency.second = parent;
//                agent_dependencies.push_back(dependency);
//                //cout << "inserted dependecy: " <<  dependency.second << " -> " << dependency.first << endl;
//            }
            if (parent != child){
                dependency dependency;
                dependency.child = child;
                dependency.parent = parent;
                dependency.not_found_goals = not_found_goals;
                agent_dependencies.push_back(dependency);
                //cout << "inserted dependecy: " <<  dependency.parent << " -> " << dependency.child << endl;
            }

        }
        if((target_goals != found_goals) && (found_goals > 0)){
            cout << "   total child facts: " << target_goals + ignored_goals << endl;
            cout << "   target only facts: " << target_goals << endl;
            cout << "   not found facts: " << target_goals-found_goals << endl;
            cout << "   found facts: " << found_goals << endl;
            cout << "some " << agent_type << " facts NOT found" << endl;
            cout << agent_type << " dependent on " << removed_objects.first << endl;
//            if (parent != child) {
//                pair<int, int> dependency;
//                dependency.first = child;
//                dependency.second = parent;
//                agent_dependencies.push_back(dependency);
//            }
            if (parent != child){
                dependency dependency;
                dependency.child = child;
                dependency.parent = parent;
                dependency.not_found_goals = not_found_goals;
                agent_dependencies.push_back(dependency);
                //cout << "inserted dependecy: " <<  dependency.parent << " -> " << dependency.child << endl;
            }

        }
        child++;
    }
    cout << "Total Facts: " << total_goals << endl;
    cout << "Total Found Facts: " << total_found_goals << endl;
}


// 1. dual parameters of same types in operator
// +allow predicates with 3 or more types
// -drones exchanging batteries

// 2. two different types, some ok some not ok
// +dronesx exchanging batteries
// -in predicates with 3 or more types (not parameters) not sure which is non agent (maybe corroborate with another rule like the 2 parameters with same type)
// sokoban and mapanalyser have a predicate with 3 parameters but just 2 types
// 2014 storage "either" works as only 2 distinct types in a proposition
//+no domains in ipc2014 and ipc2018
// -can not identify location if proposition with multiple types not in goal (delivered card is dependent on location, but not present in proposition)
// if instance not in predicate in rpg, no connection (




// 3. mapped static predicates - if they are not in any effects - remove
//sokoban , mapanalyser, tile (it is a location even though it is also an agent)
// - attribute of agent that does not change throughout problem (has drone camerasensor)
// + must have same type
// - mapped

// - there is a static proposition with 2 parameters of type x
// - there is an operator with 2 parameters of type x
// - type X and all of X subtypes





int main(int argc, char * argv[])
{
    main_start = std::time(0);
    const auto begin = std::chrono::system_clock::now();
    cout << "main started" << endl;

    FAverbose = false;

    int argcount = 1;

    FF::steepestDescent = false;
    FF::incrementalExpansion = false;
    FF::invariantRPG = false;
    FF::timeWAStar = false;
    LPScheduler::hybridBFLP = false;

    bool benchmark = false;
    bool readInAPlan = false;
    bool postHocTotalOrder = false;
    bool debugPreprocessing = false;
    bool postHocScheduleToMetric = false;

    string inputed_problem_name;
    string original_domain_file_name;
    string original_problem_file_name;
    std::time_t procedure_start_time;
    vector<RPGBuilder::agent> agents_structure;
    vector<string> dynamic_types_vector;
    bool connected_map = false;
    bool try_connected_map = false;
    bool all_dead_end_agent_goals_problem = false;
    bool active_agents_problem = false;
    bool generate_landmarks = true;
    bool random = false;
    string node_id = "0";
    string node = "/node_";
    std::time_t true_start;


#ifdef STOCHASTICDURATIONS
    const char * const defaultDurationManager = "montecarlo";

    const char * durationManagerString = defaultDurationManager;
#endif


    while (argcount < argc && argv[argcount][0] == '-') {

        string remainder(&(argv[argcount][1]));
        if (remainder == "citation") {

            cout << "If you are using the planner to optimise preferences or goals with time-dependent\n";
            cout << "collection costs, the citation for the paper describing OPTIC is:\n\n";
            cout << "@CONFERENCE{opticicaps,\n";
            cout << "\tauthor = \"J. Benton and A. J. Coles and A. I. Coles\",\n";
            cout << "\ttitle = \"Temporal Planning with Preferences and Time-Dependent Continuous Costs\",\n";
            cout << "\tbooktitle = \"Twenty Second International Conference on Automated Planning and Scheduling (ICAPS 12)\",\n";
            cout << "\tyear = \"2012\",\n";
            cout << "\tpublisher = \"AAAI Press\",\n";
            cout << "\tmonth = \"June\"\n";
            cout << "}\n\n";



            cout << "If you are using the planner as an implementation of the `TIL Abstraction' extension\n";
            cout << "of POPF, the relevant paper is:\n\n";
            cout << "@CONFERENCE{lsfrpicaps,\n";
            cout << "\tauthor = \"K. Tierney, A. J. Coles, A. I. Coles, C. Kroer, A. Britt, and R. M. Jensen\",\n";
            cout << "\ttitle = \"Automated Planning for Liner Shipping Fleet Repositioning\",\n";
            cout << "\tbooktitle = \"Twenty Second International Conference on Automated Planning and Scheduling (ICAPS 12)\",\n";
            cout << "\tyear = \"2012\",\n";
            cout << "\tpublifr = \"AAAI Press\",\n";
            cout << "\tmonth = \"June\"\n";
            cout << "}\n\n";


            cout << "OPTIC builds on POPF, described in:\n\n";
            cout << "@CONFERENCE{colespopficaps,\n";
            cout << "\tauthor = \"A. J. Coles and A. I. Coles and M. Fox and D. Long\",\n";
            cout << "\ttitle = \"Forward-Chaining Partial-Order Planning\",\n";
            cout << "\tbooktitle = \"Twentieth International Conference on Automated Planning and Scheduling (ICAPS 10)\",\n";
            cout << "\tyear = \"2010\",\n";
            cout << "\tpublisher = \"AAAI Press\",\n";
            cout << "\tmonth = \"May\"\n";
            cout << "}\n\n";


            cout << "--------------------------------------------------------------------------------\n\n";

        } else {

            switch (argv[argcount][1]) {
#ifdef POPF3ANALYSIS
                case 'l': {
                    NumericAnalysis::doGoalLimitAnalysis = false;
                    break;
                }
#endif
                case ']': {
                    RPGHeuristic::alwaysExpandFully = true;
                    break;
                }
                case '[': {
                    RPGHeuristic::addTheMaxCosts = true;
                    cout << "Warning: assuming max costs are additive-max safe\n";
                    break;
                }
                case '2': {
                    RPGBuilder::noSelfOverlaps = true;
                    break;
                }
                case 'D': {
                    Globals::paranoidScheduling = true;
                    break;
                }
                case 'd': {
                    FF::nonDeletorsFirst = true;
                    break;
                }
                case 'P': {
                    Globals::profileScheduling = true;
                    break;
                }
                case 'A': {
                    RPGHeuristic::estimateCosts = false;
                    break;
                }
                case 'a': {
                    FF::costOptimalAStar = true;
                    break;
                }
                case 'g': {
                    RPGHeuristic::setGuidance(&(argv[argcount][2]));
                    break;
                }
                case '/': {
                    LPScheduler::workOutFactLayerZeroBoundsStraightAfterRecentAction = true;
                    break;
                }
                case 'G': {
                    FF::biasG = true;
                    break;
                }
                case '8': {
                    FF::biasD = true;
                    break;
                }
                case 'S': {
                    RPGBuilder::sortedExpansion = true;
                    break;
                }
                case '3': {
                    FF::relaxMIP = true;
                    break;
                }
                case 'F': {
                    RPGBuilder::fullFFHelpfulActions = true;
                    break;
                }
#ifdef STOCHASTICDURATIONS
                    case 'f': {
                solutionDeadlineTime = atof(&(argv[argcount][2]));
                break;
            }
            case 'M': {
                durationManagerString = &(argv[argcount][2]);
                break;
            }
#else
                case 'M': {
                    FF::makespanTieBreak = false;
                    break;
                }
#endif
                case 'b': {
                    FF::bestFirstSearch = false;
                    break;
                }
                case 'B': {
                    benchmark = true;
                    break;
                }
                case 'e': {
                    FF::steepestDescent = true;
                    break;
                }
                case 'E': {
                    FF::skipEHC = true;
                    break;
                }
                case 'k': {
                    RPGBuilder::doSkipAnalysis = false;
                    break;
                }
                case 'm': {
                    if (argv[argcount][2] == '2') {
                        FF::openListOrderLowCostFirst = true;
                    } else {
                        FF::openListOrderLowMakespanFirst = true;
                    }
                    break;
                }
                case 'c': {
                    RPGBuilder::modifiedRPG = false;
                    break;
                }
                case 'C': {
                    FF::allowCompressionSafeScheduler = true;
                    break;
                }
#ifdef ENABLE_DEBUGGING_HOOKS
                case 'H': {
                    debugPreprocessing = true;
                    break;
                }
#endif
                case 'h': {
                    FF::helpfulActions = false;
                    break;
                }
                case 'j': {
                    Globals::use_trpg_bounds = true;
                    break;
                }
                case 'u': {
                    Globals::unique_landmarks_only = true;
                    //RPGBuilder::allow_all = false;
                    break;
                }
                case 'K': {
                    if (argv[argcount][2] == 0) {
                        cerr << "Error: must specify the complete initial problem name, e.g. -Kproblem.pddl\n";
                        usage(argv);
                        exit(1);
                    }
                    inputed_problem_name = &(argv[argcount][2]);
                    break;
                }
                case 'V': {
                    if (argv[argcount][2] == 0) {
                        cerr << "Error: must specify a starting duration, e.g. -V0.0\n";
                        usage(argv);
                        exit(1);
                    }
                    string argument = &(argv[argcount][2]);
                    string delimiter = "@";
                    std::size_t prev = 0, pos;
                    pos = argument.find_first_of(delimiter, prev);
                    string procedure_start_time_string = argument.substr(prev,pos-prev);
                    prev = pos+1;
                    while((pos = argument.find_first_of(delimiter, prev)) != std::string::npos){
                        if(pos > prev){
                            dynamic_types_vector.push_back(argument.substr(prev,pos-prev));
                        }
                        prev = pos+1;
                    }
                    if (prev < argument.length()){
                        dynamic_types_vector.push_back(argument.substr(prev,std::string::npos));
                    }
                    procedure_start_time = stoi(procedure_start_time_string);

                    break;
                }
                case 'Y': {
                    if (argv[argcount][2] == 0){
                        cerr << "Error: must specify the parent agent, priority and dynamic type, e.g. -Y0|0|ambulance\n";
                        usage(argv);
                        exit(1);
                    }
                    string argument = &(argv[argcount][2]);
                    string delimiter = "@";
                    vector<string> agent_details;
                    std::size_t prev = 0, pos;
                    while((pos = argument.find_first_of(delimiter, prev)) != std::string::npos){
                        if(pos > prev){
                            agent_details.push_back(argument.substr(prev,pos-prev));
                        }
                        prev = pos+1;
                    }
                    if (prev < argument.length()){
                        agent_details.push_back(argument.substr(prev,std::string::npos));
                    }
                    RPGBuilder::agent current_agent;
                    if(agent_details[0] == "1") current_agent.is_parent = true;
                    else current_agent.is_parent = false;
                    current_agent.priority = stoi(agent_details[1]);
                    current_agent.type = agent_details[2];
                    agents_structure.push_back(current_agent);
//                    reachabilityAnalysis = false;
                    break;
                }

                case '1': {
                    try_connected_map = true;
                    RPGBuilder::allow_all = false;
                    break;
                }
                case '4': {
                    connected_map = true;
                    break;
                }
                case '5': {
                    // create active agents problem
                    active_agents_problem = true;
                    procedure_start_time = main_start;
                    RPGBuilder::allow_all = false;
                    break;
                }
                case '6': {
                    generate_landmarks = false;
                    break;
                }
                case '7': {
                    reachabilityAnalysis = false;
                    break;
                }
                case '{': {
                    if (argv[argcount][2] == 0) {
                        cerr << "Error: must specify the new parent node e.g. -60-1\n";
                        usage(argv);
                        exit(1);
                    }
                    node_id = &(argv[argcount][2]);
                    break;
                }
                case 'i': {
                    FF::firstImprover = true;
                    break;
                }
                case 'O': {
                    FF::startsBeforeEnds = false;
                    break;
                }
                case 'o': {
                    LPScheduler::optimiseOrdering = false;
                    break;
                }
                case 'p': {
                    FF::pruneMemoised = false;
                    break;
                }
                case 'R': {
                    FF::invariantRPG = true;
                    break;
                }
                case 'q': {
                    FF::useDominanceConstraintsInStateHash = true;
                    break;
                }
                case 'x': {
                    if (argv[argcount][2] == 0) {
                        cerr << "Error: must specify a time limit after -x, e.g. -x1800\n";
                        usage(argv);
                        exit(1);
                    }
                    Globals::timeLimit = atoi(&(argv[argcount][2]));
                    break;
                }
                case 'T': {
                    Globals::totalOrder = true;
                    RPGBuilder::modifiedRPG = false;
                    FF::tsChecking = true;
                    if (argv[argcount][2] == 'T') {
                        postHocTotalOrder = true;
                    }
                    break;
                }
                case 't': {
                    FF::tsChecking = false;
                    break;
                }
                case 'X': {
                    NumericAnalysis::readBounds = true;
                    break;
                }
                case 'Q': {
                    postHocScheduleToMetric = true;
                    break;
                }
                case 'v': {
                    if (argv[argcount][2] == 'p') {
                        PreferenceHandler::preferenceDebug = true;
                    } else if (argv[argcount][2] != 0) {
                        Globals::writeableVerbosity = atoi(&(argv[argcount][2]));
                    } else {
                        cout << "No verbosity level specified with -v, defaulting to 1\n";
                        Globals::writeableVerbosity = 1;
                    }
                    break;
                }
                case 'L': {
                    if (argv[argcount][2] != 0) {
                        LPScheduler::lpDebug = atoi(&(argv[argcount][2]));
                    } else {
                        LPScheduler::lpDebug = 255;
                    }
                    break;
                }
                case 'W': {
                    string Warg(&(argv[argcount][2]));
                    if (Warg.empty()) {
                        cerr << "Error: must specify weight after W, e.g. -W5, or specify a weight and restart weight reduction, e.g. -W5,1\n";
                        usage(argv);
                        exit(1);
                    }
                    const int commaAt = Warg.find(',');
                    if (commaAt == string::npos) {
                        istringstream conv(Warg);
                        if (!(conv >> FF::doubleU)) {
                            cerr << "Error: must specify weight after W, e.g. -W5, or specify a weight and restart weight reduction, e.g. -W5,1\n";
                            usage(argv);
                            exit(1);
                        }
                    } else {
                        const string W = Warg.substr(0,commaAt);
                        {
                            istringstream conv(W);
                            if (!(conv >> FF::doubleU)) {
                                cerr << "Error: must specify weight after W, e.g. -W5, or specify a weight and restart weight reduction, e.g. -W5,1\n";
                                usage(argv);
                                exit(1);
                            }
                        }
                        const string WR = Warg.substr(commaAt + 1);
                        {
                            istringstream conv(WR);
                            if (!(conv >> FF::doubleUReduction)) {
                                cerr << "Error: must specify weight after W, e.g. -W5, or specify a weight and restart weight reduction, e.g. -W5,1\n";
                                usage(argv);
                                exit(1);
                            }
                        }
                    }
                    break;
                }
                case 'w': {
                    RPGHeuristic::orderByDeadlineRelevance = true;
                    break;
                }
                case 'I': {
                    LPScheduler::hybridBFLP = false;
                    break;
                }
                case 'r': {
                    readInAPlan = true;
#ifdef POPF3ANALYSIS
                    if (argv[argcount][2] != 0) {
                        FF::reprocessQualityBound = atof(&(argv[argcount][2]));
                    }
#endif
                    break;
                }
                case '0': {
                    TemporalAnalysis::abstractTILsWherePossible = true;
                    LPScheduler::hybridBFLP = false;
                    break;
                }
                case 's': {
                    FF::planMustSucceed = true;
                    break;
                }
                case 'z': {
                    FF::zealousEHC = false;
                    break;
                }
                case 'Z': {
                    RPGHeuristic::printRPGAsDot = true;
                    break;
                }
#ifdef POPF3ANALYSIS
                case 'n': {
                    Globals::optimiseSolutionQuality = true;
                    if (argv[argcount][2] != 0) {
                        Globals::givenSolutionQualityDefined = true;
                        Globals::givenSolutionQuality = atof(&(argv[argcount][2]));
                    }
                    break;
                }
                case 'N': {
                    Globals::optimiseSolutionQuality = false;
                    break;
                }
                case '>': {
                    Globals::improvementBetweenSolutions = atof(&(argv[argcount][2]));
                    break;
                }
#endif
                default:
                    cout << "Unrecognised command-line switch '" << argv[argcount][1] << "'\n";
                    usage(argv);
                    exit(0);

            }

        }
        ++argcount;
    }

#ifdef STOCHASTICDURATIONS
    const int expectFromHere = 3;
#else
    const int expectFromHere = 2;
#endif

    if (argcount + ((readInAPlan || debugPreprocessing) ? expectFromHere + 1 : expectFromHere) > argc) {
        usage(argv);
        exit(0);
    }

#ifdef STOCHASTICDURATIONS
    solutionDeadlinePercentage = atof(argv[argcount]);
    ++argcount;
#endif
    original_domain_file_name =  &(argv[argcount][0]);
    original_problem_file_name =  &(argv[argcount+1][0]);


    performTIMAnalysis(&argv[argcount]);

    cout << std::setprecision(3) << std::fixed;

#ifdef STOCHASTICDURATIONS
    setDurationManager(durationManagerString);
#endif

#ifdef TOTALORDERSTATES
    MinimalState::setTransformer(new TotalOrderTransformer());
#else
    if (Globals::totalOrder) {
        MinimalState::setTransformer(new TotalOrderTransformer());
    } else {
        MinimalState::setTransformer(new PartialOrderTransformer());
    }
#endif

#ifdef ENABLE_DEBUGGING_HOOKS
    if (debugPreprocessing) {
        Globals::planFilename = argv[argc - 1];
    }
#endif

#ifdef POPF3ANALYSIS
    const bool realOpt = Globals::optimiseSolutionQuality;
    Globals::optimiseSolutionQuality = (Globals::optimiseSolutionQuality || postHocScheduleToMetric);
#endif

    RPGBuilder::initialise();

#ifdef POPF3ANALYSIS
    Globals::optimiseSolutionQuality = realOpt;
#endif

#ifdef STOCHASTICDURATIONS
    initialiseDistributions();
    setSolutionDeadlineTimeToLatestGoal();
#endif




    if (inputed_problem_name.length() < 1){
        stringstream ss;
        ss << original_problem_file_name;
        if((ss.str().substr(ss.str().length()-5,ss.str().length()).compare(".pddl") != 0) && (ss.str().substr(ss.str().length()-5,ss.str().length()).compare(".PDDL") != 0)) {
            runCommand("cp " + ss.str() + " " + ss.str() + ".pddl");
            original_problem_file_name = ss.str() + ".pddl";

        }
        string problem_name = original_problem_file_name.substr(0,original_problem_file_name.length()-5);
        runCommand("mkdir " + problem_name + "_Arborescence");
        runCommand("mkdir " + problem_name + "_Arborescence"+ node + node_id);
        runCommand("rm -rf " + problem_name + "_Arborescence"+ node + node_id + "/*.*");
        node = problem_name + "_Arborescence" + node + node_id + "/";
        char * problem_name_char = new char[original_problem_file_name.length() + 1];
        strcpy(problem_name_char, original_problem_file_name.substr(0,original_problem_file_name.length()-5).c_str());
        VAL::current_analysis->the_problem->name = problem_name_char;;
    }
    else{
        runCommand("mkdir " + inputed_problem_name.substr(0,inputed_problem_name.length()-5) + "_Arborescence" + node + node_id);
        node = inputed_problem_name.substr(0,inputed_problem_name.length()-5) + "_Arborescence" + node + node_id + "/";
    }





    if (Globals::optimiseSolutionQuality && Globals::givenSolutionQualityDefined) {
        if (RPGBuilder::getMetric()) {
            cout << "Forcing the use of the given solution quality of " << Globals::givenSolutionQuality << endl;
            if (RPGBuilder::getMetric()->minimise) {
                Globals::bestSolutionQuality = (Globals::givenSolutionQuality == 0.0 ? 0.0 : -Globals::givenSolutionQuality);
            } else {
                Globals::bestSolutionQuality = Globals::givenSolutionQuality;
            }

            RPGBuilder::getHeuristic()->metricHasChanged();
        }
    }

    bool reachesGoals;
    /*
     * Reachability analysis
     */





    // Identify dynamic types
    if (reachabilityAnalysis){

        if (active_agents_problem){

        // get operator effects : propositions from ProblemController
        ProblemGenerator* problemGenerator = new ProblemGenerator(VAL::current_analysis->the_domain, VAL::current_analysis->the_problem);
        std::vector<vector<VAL::proposition*>> effects = problemGenerator->getOperatorsEffects();

        // mark the type of the first predicate argument as dynamic
        for(int i = 0; i < effects.size(); i++){
            cout << i << " total: " << effects[i].size() << endl;
            for(int j = 0; j < effects[i].size(); j++){
                if (*effects[i][j]->args->begin() != *effects[i][j]->args->end()) {
                    VAL::parameter_symbol *first_argumet = *effects[i][j]->args->begin();
                    dynamic_types.insert(first_argumet->type->getName());
                    cout << "dynamic type added: " << first_argumet->type->getName() << endl;
                    cout << i << " at: " << j << endl;
                }
            }
        }
        cout << "debug 1" << endl;

        //get subtypes of the identified dynamic types
        for (VAL::pddl_type_list::iterator it = VAL::current_analysis->the_domain->types->begin(); it != VAL::current_analysis->the_domain->types->end(); it++) {
            VAL::pddl_type* type = *it;
            //cout << "analysing: " << type->getName() << endl;
            bool found = false;

            for(set<string>::iterator sit = dynamic_types.begin(); sit != dynamic_types.end(); sit++){
                //cout << "comparing against dynamic type: " << *sit << endl;
                VAL::pddl_type* super_type = type;
                while (super_type->type != NULL){
                    super_type = super_type->type;
                    cout << type->getName() << " has super-type " << super_type->getName() << endl;
                    if(super_type->getName().compare(*sit) == 0){
                        found = true;
                        break;
                    }
                }
            }

            if(found){
                dynamic_types.insert(type->getName());
                //cout << "dynamic sub-type added: " << type->getName() << endl;
            }
        }

        cout << "Domain Dynamic Types:" << endl;
        for(set<string>::iterator sit = dynamic_types.begin(); sit != dynamic_types.end(); sit++){
            cout << (*sit) << endl;
        }

        // get predicates with 2 parameters of same type
        // check if the predicate is in any operator effect
        // what if no map, what if

        // mobile moves between locations
        // least state changes in operator

        // if 2 parameters in the same operator in same operator problem if intermediary ation (suspended state) suspended state

        //match problem objects to dynamic types
        for (VAL::const_symbol_list::iterator oit = VAL::current_analysis->the_problem->objects->begin();
             oit != VAL::current_analysis->the_problem->objects->end(); oit++) {
            VAL::const_symbol* const_symbol_object = *oit;

            //allow only if object is dynamic
            const bool is_in = dynamic_types.find(const_symbol_object->type->getName()) != dynamic_types.end();
            if(is_in) objectsMap[const_symbol_object->type->getName()].push_back(const_symbol_object->getName());


            //cout << const_symbol_object->getName() << " has type " << const_symbol_object->type->getName() << endl;
        }


        // get top level goals
        VAL::conj_goal* conj_goals = dynamic_cast<VAL::conj_goal*>(VAL::current_analysis->the_problem->the_goal);
        for (VAL::pc_list<VAL::goal*>::const_iterator eit = conj_goals->getGoals()->begin(); eit != conj_goals->getGoals()->end(); eit++) {
            VAL::simple_goal *simple_goal = dynamic_cast<VAL::simple_goal *>(const_cast<VAL::goal *>(*eit));
            VAL::proposition *prop = const_cast<VAL::proposition *>(simple_goal->getProp());
            toplevel_goals.push_back(prop);
        }



        //get reachability analysis facts + top level goals
        cout << "RPGBuilder::factLayerNodes size " << RPGBuilder::factLayerNodes.size() << endl;
        int lcount = 0;
        for (map<double, RPGBuilder::ClusterDef >::const_iterator flItr = RPGBuilder::factLayerNodes.begin(); flItr != RPGBuilder::factLayerNodes.end(); flItr++){
            //cout << "Fact layer : " << lcount << endl;
            RPGBuilder::ClusterDef def = flItr->second;
            for (map<string,string>::const_iterator nItr = def.nodes.begin(); nItr != def.nodes.end(); ++nItr) {
                std::string target = nItr->second;
                //cout << target << endl;
                std::size_t pos = target.find("(");
                target = target.replace(0,pos,"");
                //cout << target << endl;
                pos = target.find(")");
                VAL::proposition* prop = stringToProposition(target.substr(0,pos+1));
                //cout << target.substr(0,target.length() - 1) << endl;
                bool exists = false;
                for(int i = 0; i < reachability_facts.size(); i++){
                    if(porpositionToString(prop).compare(porpositionToString(reachability_facts[i])) == 0){
                        exists = true;
                        break;
                    }
                }
                //cout << " exists set" << endl;
                if(!exists){
                    reachability_facts.push_back(prop);
//                    cout << "-" << endl << target.substr(0,pos+1) << endl;
//                    cout << porpositionToString(prop) << endl;
//                    cout << "-" << endl;
                }

            }

            lcount++;
        }





        // iterate and exlcude dynamic types to create type dependencies by performing a reachability analysis for a temp problem which does not contain the excluded type
        int parent = 0;
        for (map <string,std::vector<string>>::iterator oit = objectsMap.begin(); oit != objectsMap.end(); oit++){
            pair <string,std::vector<string>> objects;
            objects.first = oit->first;
            objects.second = oit->second;
            typeMap[parent] = oit->first;
            RPGBuilder::RPtoDOT(objects.second);
            int cluster = 0;

            map<double, RPGBuilder::ClusterDef >::const_iterator flItr = RPGBuilder::factLayerNodes.begin();
            const map<double, RPGBuilder::ClusterDef >::const_iterator flEnd = RPGBuilder::factLayerNodes.end();



            map<double, RPGBuilder::ClusterDef >::const_iterator alItr = RPGBuilder::actionLayerNodes.begin();
            const map<double, RPGBuilder::ClusterDef >::const_iterator alEnd = RPGBuilder::actionLayerNodes.end();

            while (flItr != flEnd && alItr != alEnd) {
                double adouble = alItr->first;
                double fdouble = flItr->first;
                RPGBuilder::ClusterDef adef = alItr->second;
                RPGBuilder::ClusterDef fdef = flItr->second;

                if (adouble - fdouble > 0.0005) {
                    printLayerDef(true, fdouble, cluster, fdef);
                    if (cluster) {
                        //cout << "\tc" << cluster - 1 << " -> c" << cluster << " [ltail=cluster_" << cluster-1 << ", lhead=cluster_" << cluster << "];\n";
                    }
                    ++flItr;
                } else if (fdouble - adouble > 0.0005) {
                    printLayerDef(false, adouble, cluster, adef);
                    if (cluster) {
                        //cout << "\tc" << cluster - 1 << " -> c" << cluster << " [ltail=cluster_" << cluster-1 << ", lhead=cluster_" << cluster << "];\n";
                    }
                    ++alItr;
                } else {
                    printLayerDef(true, fdouble, cluster, fdef);
                    ++flItr;
                    if (cluster) {
                        //cout << "\tc" << cluster - 1 << " -> c" << cluster << " [ltail=cluster_" << cluster-1 << ", lhead=cluster_" << cluster << "];\n";
                    }
                    ++cluster;
                    printLayerDef(false, adouble, cluster, adef);
                    ++alItr;
                    if (cluster) {
                        //cout << "\tc" << cluster - 1 << " -> c" << cluster << " [ltail=cluster_" << cluster-1 << ", lhead=cluster_" << cluster << "];\n";
                    }
                }

                ++cluster;
            }

            for (; flItr != flEnd; ++flItr, ++cluster) {
                double fdouble = flItr->first;
                RPGBuilder::ClusterDef fdef = flItr->second;
                printLayerDef(true, fdouble, cluster, fdef);
                if (cluster) {
                    //cout << "\tc" << cluster - 1 << " -> c" << cluster << " [ltail=cluster_" << cluster-1 << ", lhead=cluster_" << cluster << "];\n";
                }

            }

            for (; alItr != alEnd; ++alItr, ++cluster) {
                double adouble = alItr->first;
                RPGBuilder::ClusterDef adef = alItr->second;
                printLayerDef(false, adouble, cluster, adef);
                if (cluster) {
                    //cout << "\tc" << cluster - 1 << " -> c" << cluster << " [ltail=cluster_" << cluster-1 << ", lhead=cluster_" << cluster << "];\n";
                }

            };

            cout << endl << "removed: " << oit->first << endl;
            // perform reachability analysis
            rpgAnalysys(parent, objects);
            cout << objects.first << " - " << parent << " - type" << endl << endl;
            parent++;
        }




        /*
         * check if dependencies have cycles (and generate priorities if no cycles)
         */
        cout << "dependecies size: " << agent_dependencies.size() << endl;

        if(!processCycles())return 0;





        /*
         * assign priorities to agents based on dependecies
         *
         */
        bool hasChildren = false;
        for(int i = 0; i < agents.size(); i++){
            agents[i].type = typeMap[agents[i].data];
            if(agents[i].parents.size() == 0){
                agents[i].priority = 0;
                for(set<int>::iterator sit = agents[i].children.begin(); sit != agents[i].children.end(); sit++){
                    for(int j = 0; j < agents.size(); j++){
                        if(agents[j].data == *sit){
                            agents[j].priority = 1;
                            cout << typeMap[*sit] << " is child of - " << typeMap[agents[i].data] << " - added initial priority: 1" << endl;
                            hasChildren = true;
                        }
                    }
                }
            }
        }
        if(!hasChildren){
            cout << "Agents are independent" << endl;
            cout << "Nr of agents: " << agents.size() << endl;
            for(int i = 0; i < agents.size(); i++){
                cout << agents[i].data << endl;
            }
            return 0;
        }
        int priority = 1;
        while(hasChildren){
            hasChildren = false;
            for(int i = 0; i < agents.size(); i++){
                if(agents[i].priority == priority){
                    for(set<int>::iterator sit = agents[i].children.begin(); sit != agents[i].children.end(); sit++){
                        cout << "parent: " << typeMap[agents[i].data] << " | child: " << typeMap[*sit] << endl;
                        for(int j = 0; j < agents.size(); j++){
                            if(agents[j].data == *sit){
                                agents[j].priority = priority+1;
                                cout << typeMap[agents[j].data] << " gets assigned priority: " << priority+1 << endl;
                                hasChildren = true;
                            }
                        }
                    }
                }
            }
            priority++;
        }

        //set if parent agent
        for(int i = 0; i < agents.size(); i++){
            if (agents[i].children.size() > 0) agents[i].is_parent = true;
            else agents[i].is_parent = false;
            cout << agents[i].type << " has priority " << agents[i].priority << endl;
        }
        cout << endl;
        for(int i = 0; i < agents.size(); i++){
            agents_structure.push_back(agents[i]);
            dynamic_types_vector.push_back(agents[i].type);
        }

        stringstream problem_name_ss;
        problem_name_ss << VAL::current_analysis->the_problem->name << ".pddl";
        string problem_name = problem_name_ss.str();

        // output to final stats
        const auto  agents_finish_time = std::chrono::system_clock::now();
        std::chrono::duration<double> agent_extraction_and_classification_duration = agents_finish_time - begin;
        ofstream final_stats;
        stringstream stats_ss;
        if(random){stats_ss << "random_";}
        stats_ss << VAL::current_analysis->the_problem->name << ".stats";
        final_stats.open(stats_ss.str(), std::ios_base::app);
        const auto time = std::chrono::system_clock::now();
        const std::time_t time_formated = std::chrono::system_clock::to_time_t(time);
        final_stats << endl << endl << endl << endl << std::ctime(&time_formated);
        final_stats << VAL::current_analysis->the_problem->name << ".pddl" << endl;
        final_stats << "Agents and Dependencies Extraction Duration: " << agent_extraction_and_classification_duration.count() << endl;
        final_stats.close();

        // output to overheads
        ofstream overheads_output;
        stringstream overheads_ss;
        overheads_ss << VAL::current_analysis->the_problem->name << ".overheads";
        overheads_output.open(overheads_ss.str(), std::ios_base::app);
        overheads_output << agent_extraction_and_classification_duration.count();
        overheads_output.close();




            problem_name = node + "active_agents_problem_" + original_problem_file_name;
            problemGenerator = new ProblemGenerator(random, VAL::current_analysis->the_domain, VAL::current_analysis->the_problem, original_problem_file_name, original_domain_file_name, procedure_start_time, agents_structure,dynamic_types_vector);


            //create active agents problem
            cout << "import stage 1a start" << endl;
            problemGenerator->importProblem(original_problem_file_name);
            cout << "import stage 1a end" << endl;
            problemGenerator->generateActiveAgentsProblem(node,false,false,original_problem_file_name);


            //create all dead-end agent goals problem
            cout << "import stage 1b start" << endl;
            problemGenerator->importProblem(node+"active_agents_problem_"+original_problem_file_name);
            cout << "import stage 1b start" << endl;
            problemGenerator->generateActiveAgentsProblem(node,false,true,original_problem_file_name);

            runCommand("rm -rf "+original_problem_file_name.substr(0,original_problem_file_name.length()-5)+".best_plan:*");
            all_dead_end_agent_goals_problem = true;
//            if (all_dead_end_agent_goals_problem){
//                cout << "all active imported" << endl;
//                if (problemGenerator->generateAllProblems(begin, 0, true, true, false, node)){
//
//                    exit(0);
//                    //run parent agent problem
//                }
//                else{
//                    stringstream cmd;
//                    cmd << "/home/nq/stp/temporal-landmarks/release/optic/./optic-clp -{" << node_id << " -V" << procedure_start_time << " -u -1" << " -K" << original_problem_file_name << " " << original_domain_file_name << " " << node << "active_agents_problem_" << original_problem_file_name << " >> " << original_problem_file_name << ".agents_dependencies_log";
//                    cout << cmd.str() << endl;
//                    runCommand(cmd.str());
//                    exit(0);
//                }
//            }

            //add all agents
            stringstream agents_stream;
            for(int i = 0; i < agents.size(); i++){
                agents_stream << " -Y" << agents[i].is_parent << "@" << agents[i].priority << "@" << agents[i].type;
            }

            //add all dynamic types
            stringstream dynamic_types_stream;
            for(set<string>::iterator sit = dynamic_types.begin(); sit != dynamic_types.end(); sit++){
                dynamic_types_stream << "@" << (*sit);
            }

            if (all_dead_end_agent_goals_problem){
                cout << "all active imported" << endl;
                problemGenerator->generateAllProblems(begin, procedure_start_time, true, true, false, node);
                //exit(0);
                cout << "stp start" << endl;
                stringstream cmd;
                std::time_t procedure_current_time = std::time(0);
                double time_left = 1800 - (procedure_current_time - procedure_start_time);
                cmd << "ulimit -t " << time_left << ";/home/nq/RALSTP/release/optic/./optic-clp -{" << node_id << agents_stream.str() << " -V" << procedure_start_time << dynamic_types_stream.str() << " -u -1" << " -K" << original_problem_file_name << " " << original_domain_file_name << " " << node << "active_agents_problem_" << original_problem_file_name << " >> " << original_problem_file_name << ".agents_dependencies_log";
                cout << cmd.str() << endl;
                runCommand(cmd.str());
                exit(0);

            }
            stringstream cmd;
            std::time_t procedure_current_time = std::time(0);
            double time_left = 1800 - (procedure_current_time - procedure_start_time);
            cmd << "ulimit -t " << time_left << ";/home/nq/RALSTP/release/optic/./optic-clp -{" << node_id << agents_stream.str() << " -V" << procedure_start_time << dynamic_types_stream.str() << " -u" << " -K" << original_problem_file_name;
            if (connected_map) { cmd << " -1" ;}
            cmd << " " << original_domain_file_name << " " << node << "active_agents_problem_" << original_problem_file_name << " >> " << original_problem_file_name << ".agents_dependencies_log";
            cout << cmd.str() << endl;
            runCommand(cmd.str());
            exit(0);

        }

        stringstream problem_name_ss;
        problem_name_ss << VAL::current_analysis->the_problem->name << ".pddl";
        string problem_name = problem_name_ss.str();

        // Check if connected map
        if(try_connected_map){
            objectsMap.clear();
            agents = agents_structure;
            for (VAL::const_symbol_list::iterator oit = VAL::current_analysis->the_problem->objects->begin();
                 oit != VAL::current_analysis->the_problem->objects->end(); oit++) {
                VAL::const_symbol* const_symbol_object = *oit;
                for(int i = 0; i < agents.size(); i++){

                    if(agents[i].type == const_symbol_object->type->getName()) {
                        objectsMap[const_symbol_object->type->getName()].push_back(const_symbol_object->getName());
                    }
                }
            }
            cout << "debug agents " << agents.size() << endl;
            cout << "debug objectmap " << objectsMap.size() << endl;
            if(RPGBuilder::hasConnectedMap(objectsMap,agents)){
                connected_map = true;
                cout << "connected map: true" << endl;
                int max = agents[0].priority;
                for(int i = 0; i < agents.size(); i++){
                    if(agents[i].priority > max) max = agents[i].priority;
                }
                ifstream problem_file;
                problem_file.open(problem_name);
                problem_name = node + "single_agents_"+problem_name;
                cout << "opening " << problem_name << endl;
                ofstream single_agents_problem_file;
                cout << "opened " << problem_name << endl;
                single_agents_problem_file.open(problem_name);
                string line;
                bool allow_check = false;
                bool allow_check2 = false;
                while (getline(problem_file, line)) {
                    //TODO fix multiple props/instances on the same line bug (via generate problem) ?? already fixed?
                    if (line.find("(:init", 0) != std::string::npos)allow_check = true;
                    if (line.find("(:goal", 0) != std::string::npos)allow_check2 = true;
                    bool line_valid = true;
                    if (allow_check){
                        for (map <string,std::vector<string>>::iterator oit = objectsMap.begin(); oit != objectsMap.end(); oit++){

//                            int type_priority = RPGBuilder::getPriorityOfType(oit->first,agents);
//                            if(((type_priority == max) || type_priority == -1) ) continue;
                            if(RPGBuilder::isParentType(oit->first,agents) != 0) continue;
                            for(int i = 1; i < oit->second.size(); i++){
                                if ((line.find((" "+oit->second[i]+" "), 0) != std::string::npos) || (line.find((" "+oit->second[i]+")"), 0) != std::string::npos) || (line.find((oit->second[i]), 0) != std::string::npos)){
                                    line_valid = false;
                                    cout << "   erased fact from problem file: " << line << "\n";
                                    break;
                                }
                            }
                            //remove parent agent goals
                            if(allow_check2){
//                                if(((type_priority == max) || type_priority == -1) ) continue;
                                if(RPGBuilder::isParentType(oit->first,agents) != 0) continue;
                                if ((line.find((" "+oit->second[0]+" "), 0) != std::string::npos) || (line.find((" "+oit->second[0]+")"), 0) != std::string::npos)){
                                    line_valid = false;
                                    cout << "   erased goal from problem file: " << line << "\n";
                                    break;
                                }
                            }
                            if(!line_valid)break;
                        }
                    }
                    if(line_valid){
                        single_agents_problem_file << line << endl;
                    }
                }

            }
            else{ cout << "connected map: false" << endl; }

        }


        cout << "RPG analysis finished" << endl;


        stringstream cmd;
//        cmd << "cp " << original_problem_file_name << " " << problem_name << ".pddl";
//        runCommand(cmd.str());
//        cout << "opt " << cmd.str() << endl;
//        cmd.str("");

        //


//
        // remove agent instances and goals from problem file
        std::time_t procedure_current_time = std::time(0);
        double time_left = 1800 - (procedure_current_time - procedure_start_time);
        cmd << "ulimit -t " << time_left << ";/home/nq/RALSTP/release/optic/./optic-clp -{" << node_id << " -u -7";
        if (connected_map) cmd << " -4";
        //add instantiated dynamic types
        cmd << " -K" << inputed_problem_name;

        for(int i = 0; i < agents.size(); i++){
            cmd << " -Y" << agents[i].is_parent << "@" << agents[i].priority << "@" << agents[i].type;
        }





        // port current process duration
        const auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> process_duration = end - begin;
        cmd << " -V" << procedure_start_time;


        //add all dynamic types
//        for(set<string>::iterator sit = dynamic_types.begin(); sit != dynamic_types.end(); sit++){
//            cmd << "@" << (*sit);
//        }
        for(int i = 0; i < dynamic_types_vector.size(); i++){
            cmd << "@" << dynamic_types_vector[i];
        }

        std::cout << endl << "Reachability Analysis Duration: " << process_duration.count() << endl;

        cmd << " " << original_domain_file_name << " " << problem_name << " > " << VAL::current_analysis->the_problem->name << ".solve_log";
        cout << cmd.str() << endl;
        runCommand(cmd.str());
        exit(0);

//
//        cout << "starting import 1" << endl;
//        //importProblem(problem_name + ".pddl");
//        cout << "import 1 succesfull" << endl;
//
//        cout << "starting import 1a" << endl;
//        //importProblem("l0_0active_agents_problem_dlog-5-5-10.pddl");
//        cout << "import 1a succesfull" << endl;


    }



    bool abstract = true;



    temporal_landmarks::TemporalLandmarksAndConstraints temporal_lm;
    if (inputed_problem_name.length() < 1){
//        stringstream problem_name_ss;
//        problem_name_ss << VAL::current_analysis->the_problem->name << ".pddl";
//        temporal_lm.original_problem_file_name = problem_name_ss.str();
        cout << "error: inputed_problem_name not found" << endl;
    }
    else temporal_lm.original_problem_file_name = inputed_problem_name;
    temporal_lm.original_domain_file_name = original_domain_file_name;


    //sort(agents_structure.begin(), agents_structure.end());

    temporal_lm.agents_structure = agents_structure;
    temporal_lm.dynamic_types_vector = dynamic_types_vector;


    const auto generate_landmarks_start = std::chrono::system_clock::now();

    if (!all_dead_end_agent_goals_problem && generate_landmarks) {
        temporal_lm.generate_landmarks(node);
    }


    const auto generate_landmarks_end = std::chrono::system_clock::now();

    std::chrono::duration<double> generate_landmarks_duration = generate_landmarks_end - generate_landmarks_start;
    cout << endl << "Landmarks Generation Duration: " << generate_landmarks_duration.count() << endl << endl;

    //TODO: ??? numeric detection - numeric constraints vs logical structures ???
    // worst case : highlight as a limitation

    //std::ofstream lmfile_lisp("lmfile.lisp");
    //temporal_lm.write_lisp(lmfile_lisp);

    if (!all_dead_end_agent_goals_problem && generate_landmarks) std::cout << "Landmarks generated." << endl << endl;
    //temporal_lm.markDisjunctiveLandmarks();
    if (!all_dead_end_agent_goals_problem && generate_landmarks) temporal_lm.update_if_abstract();


    const auto abstract_landmarks_end = std::chrono::system_clock::now();

    std::stringstream pss;
    if (!all_dead_end_agent_goals_problem && generate_landmarks) {
        if (!connected_map)pss  << original_problem_file_name.substr(0,original_problem_file_name.length()-5) << ".landmarks_disconnected_map";
        else pss << original_problem_file_name.substr(0,original_problem_file_name.length()-5) << ".landmarks_connected_map";
        if (random){pss << "_random";}
    }
    cout << pss.str() << endl;
    std::ofstream lmfile_txt(pss.str());
    temporal_lm.write(cout);
    temporal_lm.write(lmfile_txt);

    cout << "starting import 2" << endl;
    //importProblem("l0_0active_agents_problem_dlog-5-5-10.pddl");
    cout << "import 2 succesfull" << endl;



    if (!all_dead_end_agent_goals_problem && generate_landmarks) { std::cout << "Extracting Duplicate Landmarks..." << endl << endl; }
    temporal_lm.landmark_based_decomposition_into_subgoals(abstract,random,connected_map,lmfile_txt,begin,procedure_start_time,all_dead_end_agent_goals_problem, node);
    if (!all_dead_end_agent_goals_problem && generate_landmarks) { cout << "Duplicate Landmarks extracted: " << endl << endl; }

    const auto end = std::chrono::system_clock::now();
    const std::time_t time_formated = std::chrono::system_clock::to_time_t(end);
    std::chrono::duration<double> process_duration = end - begin;
    std::chrono::duration<double> landmarks_total_duration = generate_landmarks_end - begin;
    std::chrono::duration<double> abstract_landmarks_duration = abstract_landmarks_end - generate_landmarks_end;
    std::chrono::duration<double> decomposition_and_solving_duration = end - abstract_landmarks_end;
    std::cout << endl << "Full Process Duration: " << (process_duration.count()+procedure_start_time) << " s " << endl;

    ofstream final_stats;
    stringstream stats_ss;
    if(random){stats_ss << "random_";}
    stats_ss << VAL::current_analysis->the_problem->name << ".stats";
    final_stats.open(stats_ss.str(), std::ios_base::app);
    final_stats << std::ctime(&time_formated) << endl;
    final_stats << "Random: "; if(random){final_stats << "True";final_stats << endl;} else {final_stats << "False";final_stats << endl;}
    final_stats << "Connected Map: "; if(connected_map)final_stats << "True"; else final_stats << "False";final_stats << endl;
    std::time_t procedure_end_time = std::time(0);
    final_stats << "Agents, Dependencies and ADEAG decomposition/solve: " << (procedure_end_time - procedure_start_time - landmarks_total_duration.count() - abstract_landmarks_duration.count() - decomposition_and_solving_duration.count()) << endl;
    if (!all_dead_end_agent_goals_problem)final_stats << "Landmarks Backchaining operation: " << generate_landmarks_duration.count() << endl;
    if (!all_dead_end_agent_goals_problem)final_stats << "Landmarks Total Duration (removePointlessEffects in RPG analysis + backchaining): " << landmarks_total_duration.count() << endl;
    if (!all_dead_end_agent_goals_problem)final_stats << "Abstract Landmarks Identification Duration: " << abstract_landmarks_duration.count() << endl;
    if (!all_dead_end_agent_goals_problem)final_stats << "Decomposition and Solving: " << decomposition_and_solving_duration.count() << endl;
    final_stats << "Full Process Duration: " << (procedure_end_time - procedure_start_time) << endl;
    final_stats.close();



    ofstream overheads_output;
    stringstream overheads_ss;
    overheads_ss << VAL::current_analysis->the_problem->name << ".overheads";
    overheads_output.open(overheads_ss.str(), std::ios_base::app);
    overheads_output << ", " << (procedure_end_time - procedure_start_time - landmarks_total_duration.count() - abstract_landmarks_duration.count() - decomposition_and_solving_duration.count());
    overheads_output << ", " << landmarks_total_duration.count();
    overheads_output << ", " << abstract_landmarks_duration.count();
    overheads_output << ", " << decomposition_and_solving_duration.count();
    overheads_output << ", " << (procedure_end_time - procedure_start_time);
    overheads_output << ", " << std::ctime(&time_formated);
                            ;
    overheads_output.close();




    //exit(0);

    Solution planAndConstraints;

    list<FFEvent> * & spSoln = planAndConstraints.plan;
    if (readInAPlan) {
        spSoln = readPlan(argv[argc - 1]);
        reachesGoals = true;
#ifndef NDEBUG
        spSoln = FF::doBenchmark(reachesGoals, spSoln, false);
#endif
    } else {
        planAndConstraints = FF::search(reachesGoals);
    }

    if (spSoln) {

        for (int pass = 0; pass < 2; ++pass) {
            if (pass) {
                if (!postHocScheduleToMetric) break;
#ifndef TOTALORDERSTATES
                if (!spSoln->empty()) {
                    if (Globals::totalOrder && !postHocTotalOrder) {
                        MinimalState::setTransformer(new PartialOrderTransformer());
                        Globals::totalOrder = false;
                        FF::tsChecking = false;
                    }
                    assert(planAndConstraints.constraints);
                    spSoln = FF::reprocessPlan(spSoln, planAndConstraints.constraints);
                    planAndConstraints.constraints = 0;
                }
                cout << ";;;; Post-hoc optimised solution\n";
#endif
            } else {
                cout << ";;;; Solution Found\n";
                cout << "; States evaluated: " << RPGHeuristic::statesEvaluated << endl;
                cout << "; Cost: " << planAndConstraints.quality << endl;
            }

            FFEvent::printPlan(*spSoln);

        }

        if (benchmark) {
            FF::doBenchmark(reachesGoals, spSoln);
        }

        return 0;
    } else {
        cout << ";; Problem unsolvable!\n";
        tms refReturn;
        times(&refReturn);
        double secs = ((double)refReturn.tms_utime + (double)refReturn.tms_stime) / ((double) sysconf(_SC_CLK_TCK));

        int twodp = (int)(secs * 100.0);
        int wholesecs = twodp / 100;
        int centisecs = twodp % 100;

        cout << "; Time " << wholesecs << ".";
        if (centisecs < 10) cout << "0";
        cout << centisecs << "\n";
        return 1;
    }


}

extern int yyparse();
extern int yydebug;

void split(const int & insAt, list<FFEvent>::iterator insStart, const list<FFEvent>::iterator & insItr, const list<FFEvent>::iterator & insEnd)
{

    {
        for (; insStart != insItr; ++insStart) {
            int & currPWS = insStart->pairWithStep;
            if (currPWS != -1) {
                if (currPWS >= insAt) {
                    ++currPWS;
                }
            }
        }
    }
    {
        list<FFEvent>::iterator insPost = insItr;
        for (; insPost != insEnd; ++insPost) {
            int & currPWS = insPost->pairWithStep;
            if (currPWS != -1) {
                if (insPost->time_spec == Planner::E_AT_START) {
                    ++currPWS;
                } else if (insPost->time_spec == Planner::E_AT_END) {
                    if (currPWS >= insAt) {
                        ++currPWS;
                    }
                }
            }
        }
    }
}

namespace VAL
{
    extern yyFlexLexer* yfl;
};

list<FFEvent> * readPlan(char* filename)
{
    static const bool debug = true;

    ifstream * const current_in_stream = new ifstream(filename);
    if (!current_in_stream->good()) {
        cout << "Exiting: could not open plan file " << filename << "\n";
        exit(1);
    }

    VAL::yfl = new yyFlexLexer(current_in_stream, &cout);
    yyparse();

    VAL::plan * const the_plan = dynamic_cast<VAL::plan*>(top_thing);

    delete VAL::yfl;
    delete current_in_stream;



    if (!the_plan) {
        cout << "Exiting: failed to load plan " << filename << "\n";
        exit(1);
    };

    if (!theTC->typecheckPlan(the_plan)) {
        cout << "Exiting: error when type-checking plan " << filename << "\n";
        exit(1);
    }

    list<FFEvent> * const toReturn = new list<FFEvent>();

    pc_list<plan_step*>::const_iterator planItr = the_plan->begin();
    const pc_list<plan_step*>::const_iterator planEnd = the_plan->end();

    for (int idebug = 0, i = 0; planItr != planEnd; ++planItr, ++i, ++idebug) {
        plan_step* const currStep = *planItr;

        instantiatedOp * const currOp = instantiatedOp::findInstOp(currStep->op_sym, currStep->params->begin(), currStep->params->end());
        if (!currOp) {
            const instantiatedOp * const debugOp = instantiatedOp::getInstOp(currStep->op_sym, currStep->params->begin(), currStep->params->end());
            cout << "Exiting: step " << idebug << " in the input plan uses the action " << *(debugOp) << ", which the instantiation code in the planner does not recognise.\n";
            exit(1);
        }
        const int ID = currOp->getID();

        if (RPGBuilder::getRPGDEs(ID).empty()) {// non-durative action
            FFEvent toInsert(currOp, 0.001, 0.001);
            const double ts = currStep->start_time;
            if (debug) cout << "; input " << ts << ": " << *currOp << " (id=" << ID << "), non-temporal";
            toInsert.lpTimestamp = ts;
            toInsert.lpMinTimestamp = ts;
            int insAt = 0;
            list<FFEvent>::iterator insItr = toReturn->begin();
            const list<FFEvent>::iterator insEnd = toReturn->end();
            for (; insItr != insEnd; ++insItr, ++insAt) {
                if (ts < insItr->lpTimestamp) {
                    split(insAt, toReturn->begin(), insItr, insEnd);
                    toReturn->insert(insItr, toInsert);
                    break;
                }
            }
            if (insItr == insEnd) {
                toReturn->push_back(toInsert);
            }
            if (debug) cout << " putting at step " << insAt << "\n";
        } else {
            int startIdx = -1;
            list<FFEvent>::iterator startIsAt = toReturn->end();
            const double actDur = currStep->duration;
            for (int pass = 0; pass < 2; ++pass) {
                if (pass) assert(startIdx >= 0);
                const double ts = (pass ? currStep->start_time + actDur : currStep->start_time);
                if (debug) {
                    cout << "; input " << ts << ": " << *currOp;
                    if (pass) {
                        cout << " end";
                    } else {
                        cout << " start";
                    }
                    cout << " (id=" << ID << ")";
                }
                FFEvent toInsert = (pass ? FFEvent(currOp, startIdx, actDur, actDur) : FFEvent(currOp, actDur, actDur));
                toInsert.lpTimestamp = ts;
                toInsert.lpMinTimestamp = ts;

                list<FFEvent>::iterator insItr = toReturn->begin();
                const list<FFEvent>::iterator insEnd = toReturn->end();
                int insAt = 0;
                for (; insItr != insEnd; ++insItr, ++insAt) {
                    if (ts < insItr->lpTimestamp) {
                        split(insAt, toReturn->begin(), insItr, insEnd);
                        const list<FFEvent>::iterator dest = toReturn->insert(insItr, toInsert);
                        if (pass) {
                            startIsAt->pairWithStep = insAt;
                            if (debug) cout << " putting at step " << insAt << ", pairing with " << startIdx << "\n";
                        } else {
                            startIsAt = dest;
                            startIdx = insAt;
                            if (debug) cout << " putting at step " << insAt << "\n";
                        }
                        break;
                    }
                }
                if (insItr == insEnd) {
                    toReturn->push_back(toInsert);
                    if (pass) {
                        startIsAt->pairWithStep = insAt;
                        if (debug) cout << " putting at step " << insAt << ", pairing with " << startIdx << "\n";
                    } else {
                        startIsAt = toReturn->end();
                        --startIsAt;
                        startIdx = insAt;
                        if (debug) cout << " putting at step " << insAt << "\n";
                    }
                }

            }
        }
    }

    const vector<RPGBuilder::FakeTILAction*> & tils = RPGBuilder::getNonAbstractedTILVec();
    const int tilCount = tils.size();

    for (int t = 0; t < tilCount; ++t) {
        FFEvent toInsert(t);
        const double tilTS = tils[t]->duration;
        toInsert.lpMaxTimestamp = tilTS;
        toInsert.lpMinTimestamp = tilTS;
        toInsert.lpTimestamp = tilTS;

        if (debug) {
            cout << "TIL " << toInsert.divisionID << " goes at " << tilTS << endl;
        }

        list<FFEvent>::iterator insItr = toReturn->begin();
        const list<FFEvent>::iterator insEnd = toReturn->end();
        for (int insAt = 0; insItr != insEnd; ++insItr, ++insAt) {
            if (tilTS < insItr->lpTimestamp) {
                split(insAt, toReturn->begin(), insItr, insEnd);
                toReturn->insert(insItr, toInsert);
                break;
            }
        }
        if (insItr == insEnd) {
            toReturn->push_back(toInsert);
        }
    }

    if (debug) {
        list<FFEvent>::iterator insItr = toReturn->begin();
        const list<FFEvent>::iterator insEnd = toReturn->end();

        for (int i = 0; insItr != insEnd; ++insItr, ++i) {
            cout << i << ": ";
            if (insItr->action) {
                cout << *(insItr->action);
                if (insItr->time_spec == Planner::E_AT_START) {
                    cout << " start\n";
                } else {
                    cout << " end\n";
                }
            } else {
                cout << "TIL " << insItr->divisionID << endl;
            }
        }
    }

    return toReturn;
};

