//
// Created by Dorian Buksz on 01/05/2020.
//

#include "ProblemGenerator.h"
#include <algorithm>
#include <random>
#include <chrono>
#include <unistd.h>
#include <thread>
#include <iomanip>
#include <future>

namespace VAL
{
    extern yyFlexLexer* yfl;
    extern TypeChecker *theTC;
};

extern int yyparse();
extern int yydebug;

namespace strategic_tactical {
//TODO remove unnecessary stringstreams as you have an ofstream (all over this class and others)



    int SubgoalsSet::getGoalsNoInSubgoals(){
        int goals = 0;
        for (int i = 0; i < getSet().size(); i++){
            goals = goals + getSet()[i].size();
        }
        return goals;
    }
    int SubgoalsSet::getMaxSubgoalSize(){
        int max = 0;
        for (int i = 0; i < getSet().size(); i++){
            if (getSet()[i].size() > max)max = getSet()[i].size();
        }
        return max;
    }



    /*
     * main
     */

//    char * name;
//    char * domain_name;
//    pddl_req_flag req;
//    pddl_type_list* types;
//    const_symbol_list* objects;
//    effect_lists* initial_state;
//    goal* the_goal;
//    con_goal *constraints;
//    metric_spec* metric;
//    length_spec* length;
//

    bool ProblemGenerator::isSameProposition(VAL::proposition* origin, VAL::proposition* target){
        stringstream origin_ss;
        origin_ss << origin->head->getName();
        for (VAL::parameter_symbol_list::iterator pit = origin->args->begin(); pit != origin->args->end(); pit++){
            VAL::parameter_symbol* param = *pit;
            origin_ss << " " << param->getName();
        }
        stringstream target_ss;
        target_ss << target->head->getName();
        for (VAL::parameter_symbol_list::iterator pit = target->args->begin(); pit != target->args->end(); pit++){
            VAL::parameter_symbol* param = *pit;
            target_ss << " " << param->getName();
        }
        //cout << "   checking: " << origin_ss.str() << " - " << target_ss.str() << endl;
        if(origin_ss.str().compare(target_ss.str()) == 0){
            //cout << "       found: " << origin_ss.str() << " - " << target_ss.str() << endl;
            return true;
        }
        return false;
    }


    SubgoalsSet ProblemGenerator::randomizer(SubgoalsSet current_decomposition){
        vector<int> goal_positions;
        for(int i = 0; i < goals.size(); i++){
            if(!isParentAgentProposition(goals[i])){
                goal_positions.push_back(i);
            }
        }

        for(int i = 0; i < goal_positions.size(); i++){
            srand(time(NULL));
            int j = i + rand() % (goal_positions.size() - i);
            swap(goal_positions[i], goal_positions[j]);
        }
        map<int,vector<int>> randomized_sigma;

        int count = 0;
        for (int i = 0; i < current_decomposition.getSet().size(); i++){
            vector<int> old_set = current_decomposition.getSet()[i];
            vector<int> new_set;
            for (int j = count; j < count+old_set.size(); j++){
                new_set.push_back(goal_positions[j]);
            }
            randomized_sigma[i] = new_set;
            count = count + old_set.size();
        }
        SubgoalsSet* randomSet = new SubgoalsSet(randomized_sigma);
        return *randomSet;
    }

    int ProblemGenerator::getAgentPairs(){
        int agent_pairs = 1;
        vector<int> parent_agent_groups_size;
        for(int j = 0; j < agent_groups.size(); ++j){
            if(!agent_groups[j].has_parent_parent_dynamic_type()) continue;
            parent_agent_groups_size.push_back(agent_groups[j].size());

        }
        sort(parent_agent_groups_size.begin(), parent_agent_groups_size.end());
        for(int j = 0; j < parent_agent_groups_size.size(); ++j){
            if (parent_agent_groups_size[j] > agent_pairs) {
                cout << "max agent pairs: " << parent_agent_groups_size[j] << endl;
                return parent_agent_groups_size[j]; }
        }
        cout << "max agent pairs 1" << endl;
        return agent_pairs;
    }

    bool ProblemGenerator::is_duplicate_subgoal(SubgoalsSet new_set){
        map<int,vector<int>> new_map = new_set.getSet();
        for (int i = 0; i < sigma.size(); i++){
            map<int,vector<int>> old_map = sigma[i].getSet();
            if (new_map.size() == old_map.size()){
                for (int j = 0; j < new_map.size(); j++){
                    if (new_map[j] != old_map[j]){
                        return false;
                    }
                }
                return true;
            }
        }
        return false;
    }

    bool ProblemGenerator::addSubgoalsSet(SubgoalsSet subgoals_set_){
        if (!is_duplicate_subgoal(subgoals_set_)){
            sigma.push_back(subgoals_set_);
            return true;
        }
        return false;
    }


    void ProblemGenerator::create_increasing_agent_pair_goals(){
        goals.clear();
        problem->the_goal->visit(this);
        random = "random_";
        vector<int> goal_positions;
        for(int i = 0; i < goals.size(); i++){
            if(!isParentAgentProposition(goals[i])){
                goal_positions.push_back(i);
            }
        }

        for(int i = 0; i < goal_positions.size(); i++){
            srand(time(NULL));
            int j = i + rand() % (goal_positions.size() - i);
            swap(goal_positions[i], goal_positions[j]);
        }


        vector<SubgoalsSet> randomized_sigma;
        for (int i = 1; i < 50; i++){
            map<int,vector<int>> randomized_subgoals_set;
            int max_goals = 0;
            if ((goal_positions.size() % i) == 0){
                max_goals = goal_positions.size() / i;
            }
            else{
                float float_max_goals = goal_positions.size() / (i * 1.00);
                max_goals = round(float_max_goals + 0.5);
            }
            int count = 0;
            int deduct_count = 0;
            for (int k = 0; k < i; k++){
                vector<int> new_set;
                //cout << (int)(goal_positions.size() - (count + max_goals)) << endl;
                if ((int)(goal_positions.size() - (count + max_goals)) < 0){
                    deduct_count = (int)(goal_positions.size() - (count + max_goals));
                }
                for (int j = count; j < count+max_goals+deduct_count; j++) {
                    new_set.push_back(goal_positions[j]);
                    //cout << "       debug increasing " << goal_positions[j] << endl;
                }
                randomized_subgoals_set[k] = new_set;
                count+= max_goals;
            }
            SubgoalsSet* randomSet = new SubgoalsSet(randomized_subgoals_set);
            randomized_sigma.push_back(*randomSet);
        }
        sigma = randomized_sigma;
    }


    SubgoalsSet ProblemGenerator::createNO_LMsubgoals(SubgoalsSet subgoals_set_, int agent_pairs, bool lm_tactical){


        //get lm_goals
        vector<int> lm_goals;
        //cout << "set size " << subgoals_set_.getSet().size() << endl;

        int lm_tactical_problems = subgoals_set_.getSet().size();


        if(!lm_tactical){
            //cout << "Landmarks problem: false";
            lm_tactical_problems = 0;
            subgoals_set_.clear();
        }


        for (int i = 0; i < lm_tactical_problems; i++) {
            if(goals.size() == lm_goals.size())break;
            for (int j = 0; j < goals.size(); j++) {
                for (int k = 0; k < subgoals_set_.getSet()[i].size(); k++) {
                    //cout << "in loop" << j << " - " << k << endl;
                    if ((j == subgoals_set_.getSet()[i][k]) && (!isParentAgentProposition(goals[j]))  ) {

                        lm_goals.push_back(j);
                    }
                }
            }
        }

        //get no_lm goals
        vector<int> no_lm_goals;
        //cout << "goals no " << initial_goals.size() << " lm_goals " << lm_goals.size() << endl;
        for (int j = 0; j < initial_goals.size(); j++) {
            bool generated = false;
            if(isParentAgentProposition(initial_goals[j])){continue;};
            for(int k = 0; k < lm_goals.size(); k++) {
                if (j == lm_goals[k]){
                    generated = true;
                    break;
                }
            }
            if(!generated) {
                cout << propositionToString(initial_goals[j]) << endl;
                no_lm_goals.push_back(j);
            }
        }
        srand(unsigned(time(NULL)));
        std::random_shuffle(no_lm_goals.begin(), no_lm_goals.end());

        /*
         *
         */
        //get number of no_lm tactical problems
        if (no_lm_goals.size() == 0)return subgoals_set_;
        cout << "agent_pairs " << agent_pairs << " lm_tactical_problems " << lm_tactical_problems << " no_lm_goals.size() " << no_lm_goals.size()  << endl << endl;
        int no_lm_tactical_problems, max_size;
        if((agent_pairs - lm_tactical_problems) > 1){
            if(no_lm_goals.size() <= (agent_pairs - lm_tactical_problems)){
                no_lm_tactical_problems = no_lm_goals.size();
                max_size = 1;
                cout << " case A " << 1 << endl;
            }
            else{
                no_lm_tactical_problems = agent_pairs - lm_tactical_problems;
                if ((no_lm_goals.size() / no_lm_tactical_problems) == (ceil(static_cast<double>(no_lm_goals.size()) / no_lm_tactical_problems))){
                    max_size = no_lm_goals.size() / no_lm_tactical_problems + 1;
                }
                else{
                    max_size = ceil(static_cast<double>(no_lm_goals.size()) / no_lm_tactical_problems);
                }
                cout << " case B max size = " << max_size << " | " << no_lm_goals.size() << " / " << no_lm_tactical_problems << " = " << static_cast<double>(no_lm_goals.size()) / no_lm_tactical_problems << endl;
            }
        }
        else{
            if(no_lm_goals.size() <= agent_pairs){
                no_lm_tactical_problems = no_lm_goals.size();
                max_size = 1;
                cout << " case C " << 1 << endl;
            }
            else{
                no_lm_tactical_problems = agent_pairs;
                max_size = ceil(static_cast<double>(no_lm_goals.size()) / no_lm_tactical_problems);
                cout << " case D max size = " << max_size << " | " << no_lm_tactical_problems << " = " << static_cast<double>(no_lm_goals.size()) / no_lm_tactical_problems << endl;
            }
        }

        //create no_lm subgoal(s) and add to subgoals set

        //cout << "agent_pairs " << agent_pairs << " lm_tactical_problems " << lm_tactical_problems << " no_lm_goals.size() " << no_lm_goals.size() << " no_lm_tactical_problems " << no_lm_tactical_problems << " max sizeeee " << max_size << endl << endl;
        int remaining_goals = no_lm_goals.size();
        int remaining_problems = no_lm_tactical_problems;
        int goal_index = 0;
        for (int i = 0; i < no_lm_tactical_problems; i++){
            vector<int> subgoal;
            if ((remaining_goals % remaining_problems) != 0){
                for (int j = 0; j < max_size; j++){
                    subgoal.push_back(no_lm_goals[goal_index]);
                    goal_index++;
                    remaining_goals--;
                }
                cout << "subgoal added A. size: " << subgoal.size() << " i: " << i << endl;
            }
            else{
                int adjustest_size = max_size;
                if (max_size == 1) {adjustest_size = 2;}
                for (int j = 0; j < adjustest_size-1; j++){
                    subgoal.push_back(no_lm_goals[goal_index]);
                    goal_index++;
                    remaining_goals--;
                }
                cout << "subgoal added B. size: " << subgoal.size() << " i: " << i << endl;
            }
            subgoals_set_.addSubgoal(subgoal);
            remaining_problems--;
        }


//        int max_goal = max_size;
//        for (int i = 0; i < no_lm_goals.size(); i++){
//            subgoal.push_back(no_lm_goals[i]);
//            //cout << "pushed: " << no_lm_goals[i] << " i: " << i << endl;
//            if(i == no_lm_goals.size()-1){
//                subgoals_set_.addSubgoal(subgoal);
//                //cout << "subgoal added B. size: " << subgoal.size() << " i: " << i << endl;
//                break;
//            }
//            if (i+1 == max_goal){
//                subgoals_set_.addSubgoal(subgoal);
//                //cout << "subgoal added A. size: " << subgoal.size() << " i: " << i << endl;
//                subgoal.clear();
//                max_goal = max_goal + max_size;
//            }
//        }
        return subgoals_set_;
    }

    std::vector<vector<VAL::proposition*>> ProblemGenerator::getOperatorsParameters(){
        std::vector<vector<VAL::proposition*>> operatorsPreconditions;
        for (VAL::operator_list::iterator oit = domain->ops->begin(); oit != domain->ops->end(); oit++) {
            VAL::operator_ *operator_ = *oit;
            last_operator_propositions.clear();
            last_operator_time_specs.clear();
            last_negative_effects.clear();
            operator_->precondition->visit(this);
            operatorsPreconditions.push_back(last_operator_propositions);
        }
        return operatorsPreconditions;
    }

    std::vector<vector<VAL::proposition*>> ProblemGenerator::getOperatorsPreconditions(){
        std::vector<vector<VAL::proposition*>> operatorsPreconditions;
        for (VAL::operator_list::iterator oit = domain->ops->begin(); oit != domain->ops->end(); oit++) {
            VAL::operator_ *operator_ = *oit;
            last_operator_propositions.clear();
            last_operator_time_specs.clear();
            last_negative_effects.clear();
            operator_->precondition->visit(this);
            operatorsPreconditions.push_back(last_operator_propositions);
        }
        return operatorsPreconditions;
    }

    std::vector<vector<VAL::proposition*>> ProblemGenerator::getOperatorsEffects(){
        std::vector<vector<VAL::proposition*>> operatorsEffects;
        for (VAL::operator_list::iterator oit = domain->ops->begin(); oit != domain->ops->end(); oit++) {
            VAL::operator_ *operator_ = *oit;
            last_operator_propositions.clear();
            last_operator_time_specs.clear();
            last_negative_effects.clear();
            operator_->effects->visit(this);
            operatorsEffects.push_back(last_operator_propositions);
        }
        return operatorsEffects;
    }

    void ProblemGenerator::randomizeSets(){
        goals.clear();
        problem->the_goal->visit(this);
        random = "random_";
        vector<SubgoalsSet> randomized_sigma;
        for (int i = 0; i < sigma.size(); i++){
            randomized_sigma.push_back(randomizer(sigma[i]));
        }
        sigma = randomized_sigma;
    }

    //TODO fix bug for goals with multiple parameters and bug for agent location
    bool ProblemGenerator::isRejectedFact(VAL::proposition* fact){

        //check if parent agent fact
        VAL::parameter_symbol_list::iterator pit = fact->args->begin();
        VAL::parameter_symbol *parameter_symbol_instance = *pit;
        for (pit; pit != fact->args->end(); pit++){
            parameter_symbol_instance = *pit;
            if(isParentAgent(parameter_symbol_instance->getName())){
                //cout << "added parent " << propositionToString(fact) << endl;
                return false;
            }
        }

        //check if static fact
        bool is_static_fact = true;
        pit = fact->args->begin();
        for (pit; pit != fact->args->end(); pit++){
            parameter_symbol_instance = *pit;
            if((isParentAgent(parameter_symbol_instance->getName())) || (isDeadEndAgent(parameter_symbol_instance->getName()))){
                is_static_fact =  false;
            }
        }
        if (is_static_fact) {
//            cout << "added static " << propositionToString(fact) << endl;
            return false;
        }

        //get dead-end agents in dead-end agent goals from current dead-end agent goal-set
        set<string> current_dead_end_agents;
        for (int i = 0; i < current_dead_end_agent_goal_set.size(); i++){
            for (VAL::parameter_symbol_list::iterator git = initial_goals[current_dead_end_agent_goal_set[i]]->args->begin(); git != initial_goals[current_dead_end_agent_goal_set[i]]->args->end(); git++) {
                VAL::parameter_symbol *parameter_symbol_instance = *git;
                if (isDeadEndAgent(parameter_symbol_instance->getName())){
                    current_dead_end_agents.insert(parameter_symbol_instance->getName());
                }
            }
        }
//        stringstream ss;
//        ss << "current dead end agents. size: " << current_dead_end_agents.size();
//        for (set<string>::iterator sit = current_dead_end_agents.begin(); sit != current_dead_end_agents.end(); sit++){
//            ss << " " << (*sit);
//        }


        //check if dead-end agent facts has dead-end agent which is in dead-end agent goals from current dead-end agent goal-set
        bool has_target_dead_end_agent = false;
        for (set<string>::iterator sit = current_dead_end_agents.begin(); sit != current_dead_end_agents.end(); sit++){
            string dead_end_agent = (*sit);
            pit = fact->args->begin();
            for (pit; pit != fact->args->end(); pit++){
                parameter_symbol_instance = *pit;
                if (dead_end_agent.compare(parameter_symbol_instance->getName()) == 0) {
                    //cout << "added " << ss.str()  << " " << propositionToString(fact) << endl;
                    return false;
                }
            }
        }
       // cout << "discarded " << ss.str()  << " " << propositionToString(fact) << endl;

        return true;
    }

    // check if the object is a parent agent //TODO uncomment compare line after isParentAgentPropositions fix
    bool ProblemGenerator::isParentAgent(string name){
        //cout << "debug isParentAgent " << name << endl;
        for(int i = 0; i < agent_groups.size(); i++){
            if(!agent_groups[i].has_parent_parent_dynamic_type()) continue;
            if(getObjectType(name)->getName().compare(agent_groups[i].getType()) == 0){
                //cout << "isParentAgent 1 "<< name << " " << agent_groups[i].getType() << endl;
                return true;
            }
            if((agent_groups[i].getSuperType(0).length() > 0) && (name.compare(agent_groups[i].getSuperType(0)) == 0)){
                //cout << "isParentAgent 2 "<< name << " " << agent_groups[i].getSuperType(0) << endl;
                return true;
            }

        }
        //cout << "isNOTAgent "<< name << " - " << getObjectType(name)->getName() << endl;
        return false;
    }

    //TODO method is bugged, dependent on instance name containing the type, fix after tests
    bool ProblemGenerator::isParentAgentProposition(VAL::proposition* proposition){
        for(VAL::parameter_symbol_list::iterator pit = proposition->args->begin(); pit != proposition->args->end(); pit++){
            VAL::parameter_symbol* parameter_symbol_instance = *pit;
            if(isParentAgent(parameter_symbol_instance->getName()))return true;
        }
        return false;
    }

    // check if the object is a parent agent //TODO uncomment compare line after isParentAgentPropositions fix
    bool ProblemGenerator::isDeadEndAgent(string name){
        //cout << "debug isParentAgent " << name << endl;
        for(int i = 0; i < agent_groups.size(); i++){
            if(agent_groups[i].has_parent_parent_dynamic_type()) continue;
//            if(agents_structure[i].first == agents_structure[agents_structure.size()-1].first)continue;
            if(getObjectType(name)->getName().compare(agent_groups[i].getType()) == 0){
                //cout << "isParentAgent "<< name << endl;
                return true;
            }
            if((agent_groups[i].getSuperType(0).length() > 0) && (name.compare(agent_groups[i].getSuperType(0)) == 0)){
                //cout << "isParentAgent "<< name << endl;
                return true;
            }

        }
        //cout << "isNOTAgent "<< name << " - " << getObjectType(name)->getName() << endl;
        return false;
    }

    //TODO method is bugged, dependent on instance name containing the type, fix after tests
    bool ProblemGenerator::isDeadEndAgentProposition(VAL::proposition* proposition){
        for(VAL::parameter_symbol_list::iterator pit = proposition->args->begin(); pit != proposition->args->end(); pit++){
            VAL::parameter_symbol* parameter_symbol_instance = *pit;
            if(isDeadEndAgent(parameter_symbol_instance->getName()))return true;
        }
        return false;
    }

    // method to get passed domain details loading bug
    VAL::pddl_type* ProblemGenerator::getObjectType(string name){

        for (int i = 0; i < objects.size(); i++) {
            if (name.compare(objects[i].getName()) == 0) {
                if (objects[i].type != NULL){
                    cout << name << " has clean1 type " <<  objects[i].type->getName() << endl;
                    return objects[i].type;
                }
            }
        }

        //added to fix constant without type bug
        for (int i = 0; i < objects.size(); i++) {
            if (name.find(objects[i].type->getName(), 0) != std::string::npos){
                cout << name << " has patched2 type " <<  objects[i].type->getName() << endl;
                return objects[i].type;
            }
        }


        for(VAL::pddl_type_list::iterator it = domain->types->begin(); it != domain->types->end(); it++){
            VAL::pddl_type* temp_type = *it;
            //cout << "t " << temp_type->getName() << endl;
            if (name.compare(temp_type->getName()) == 0) {
                cout << name << " has clean2 type " <<  temp_type->getName() << endl;
                return temp_type;
            }
            else{
                for (int i = 0; i < supertypes.size(); i++) {
                    //cout << "s " << supertypes[i].getName() << endl;
                    if (name.compare(supertypes[i].getName()) == 0) {
                        cout << name << " has patched2 type " << endl;
                        return &supertypes[i];
                    }
                }
            }
        }
        cout << "ProblemGenerator::getObjectType - type not found:" << " " << name << endl;
        exit;
    }

//    bool ProblemGenerator::isInstanceInInitialFacts(string name){
//        for (int i = 0; i < initial_facts.size(); i++) {
//            for (VAL::parameter_symbol_list::iterator pit = initial_facts[i]->args->begin();
//                 pit != initial_facts[i]->args->end(); pit++) {
//                VAL::parameter_symbol *parameter_symbol_instance = *pit;
//                if(parameter_symbol_instance->getName().compare(name) == 0){
//                    return true;
//                }
//            }
//        }
//        return false;
//    }



//    for ( int k = 0; k < agent_groups[i].getAgents()[j].getOutputFacts().size(); k++){
//    VAL::proposition* parent_agent_fact = agent_groups[i].getAgents()[j].getOutputFacts()[k];
//    for (VAL::parameter_symbol_list::iterator pit2 = parent_agent_fact->args->begin();
//    pit2 != parent_agent_fact->args->end(); pit2++) {
//    VAL::parameter_symbol *parameter_symbol_instance2 = *pit2;
//}
//}

    //check if fact contains agents and update if agent has goal facts
    //TODO modify to make it work for multiple agents in same fact
//    VAL::proposition* ProblemGenerator::updateIfAgentGoalState(VAL::proposition* fact){
//        for (VAL::parameter_symbol_list::iterator pit1 = fact->args->begin();
//             pit1 != fact->args->end(); pit1++) {
//            VAL::parameter_symbol *parameter_symbol_instance = *pit1;
//            for (int i = 0; i < agent_groups.size(); ++i){
//                for(int j = 0; j < agent_groups[i].getAgents().size(); j++){
//                    if(parameter_symbol_instance->getName().compare(agent_groups[i].getAgents()[j]->getName()) == 0){
//                        if((agent_groups[i].getAgents()[j]->hasGoalState()) && (!agent_groups[i].getAgents()[j]->isPriority())){
//                            for ( int k = 0; k < agent_groups[i].getAgents()[j]->getOutputFacts().size(); k++){
//                                VAL::proposition* parent_agent_fact = agent_groups[i].getAgents()[j]->getOutputFacts()[k];
//                                if(fact->head->getName().compare(parent_agent_fact->head->getName()) == 0){
//                                if(fact->head->getName().compare(parent_agent_fact->head->getName()) == 0){
//                                    return parent_agent_fact;
//                                }
//                            }
//                        }
//                    }
//                }
//            }
//        }
//        return fact;
//    }

    //TODO fix type bug ?? only if you need types further in the soft
    void ProblemGenerator::importProblem(string problem_name){
        yydebug = 0;

        VAL::analysis* old_analysis = VAL::current_analysis;
        //VAL::analysis val_analysis;
        //VAL::current_analysis = &val_analysis;
        stringstream ss;
        ss << problem_name;
        vector<string> f_names;
        f_names.push_back(original_domain_file_name);
        f_names.push_back(ss.str());
        ifstream *current_in_stream;
        VAL::yfl = new yyFlexLexer;

        // Loop over given args
        for (int a = 0; a < f_names.size(); a++) {
            string current_filename = f_names[a];
            cout << "File: " << current_filename << '\n';
            current_in_stream = new ifstream(current_filename);
            if (!current_in_stream->good()) {
                // Output a message now
                cout << "Failed to open" << current_filename << "\n";
                // Log an error to be reported in summary later
                line_no = 0;
            } else {
                line_no = 1;
                // Switch the tokeniser to the current input stream
                VAL::yfl->switch_streams(current_in_stream, &cout);
                yyparse();
                // Output syntax tree
                // if (top_thing) top_thing->display(0);
            }
            delete current_in_stream;
        }

//        VAL::current_analysis->error_list.report();
//        delete VAL::yfl;
//        cout << endl << "import 0" << endl;
//        VAL::TypeChecker tc(VAL::current_analysis);
//        cout << endl << "import 1" << endl;
//        VAL::theTC = &tc;
//        cout << endl << "import 2" << endl;
//        VAL::TypePredSubstituter a;
//        cout << endl << "import 3" << endl;
//        VAL::current_analysis->the_domain->visit(&a);
//        cout << endl << "import 4" << endl;
//        VAL::current_analysis->the_problem->visit(&a);
//        cout << endl << "import 5" << endl;
//        VAL::Analyser aa;
//        cout << endl << "import 6" << endl;
//        VAL::current_analysis->the_domain->visit(&aa);
//        cout << endl << "import 7" << endl;
//        //VAL::current_analysis->the_problem->visit(&aa);
//        cout << endl << "import 8" << endl;
//        VAL::current_analysis->the_domain->predicates->visit(&aa);
//        cout << endl << "import 9" << endl;
//        if (domain->functions)
        // domain->functions->visit(&aa);


        //VAL::current_analysis->the_domain = domain;
        //domain = VAL::current_analysis->the_domain;
        problem = VAL::current_analysis->the_problem;
        //VAL::current_analysis = old_analysis;


        //clear facts an goals
        goals.clear();
        facts.clear();

        //clear functions vectors
        values.clear();
        func_name.clear();
        func_args.clear();
        operator_.clear();

        //visit imported problem
        problem->initial_state->visit(this);
        problem->metric->visit(this);
        problem->the_goal->visit(this);

        for (int i = 0; i < initial_goals.size(); i++) {
            bool goal_solved = true;
            for (int j = 0; j < goals.size(); j++) {
                if(propositionToString(initial_goals[i]).compare(propositionToString(goals[j])) == 0){
                    goal_solved = false;
                }
            }
            if(goal_solved){
                //cout << "solved goal a " << i << endl;
                goals_in_initial_state.push_back(i);
            }
        }
//TODO        fix goals in initial state removal
//TODO        fix lm goals



        cout << "importProblem succesfull" << endl;

    }

    //TODO merge with importProblem method
    void ProblemGenerator::importProblemAndDomain(string problem_name, string domain_name){
        cout << "attempting to import " << problem_name << " " << domain_name << endl;
        yydebug = 0;

        VAL::analysis* old_analysis = VAL::current_analysis;
        //VAL::analysis val_analysis;
        //VAL::current_analysis = &val_analysis;
        stringstream ss;
        ss << problem_name;
        vector<string> f_names;
        f_names.push_back(domain_name);
        f_names.push_back(ss.str());
        ifstream *current_in_stream;
        VAL::yfl = new yyFlexLexer;

        // Loop over given args
        for (int a = 0; a < f_names.size(); a++) {
            string current_filename = f_names[a];
            cout << "File: " << current_filename << '\n';
            current_in_stream = new ifstream(current_filename);
            if (!current_in_stream->good()) {
                // Output a message now
                cout << "Failed to open2 " << current_filename << "\n";
                // Log an error to be reported in summary later
                line_no = 0;
            } else {
                line_no = 1;
                // Switch the tokeniser to the current input stream
                VAL::yfl->switch_streams(current_in_stream, &cout);
                yyparse();
                // Output syntax tree
                // if (top_thing) top_thing->display(0);
            }
            delete current_in_stream;
        }
        cout << "importProblemDomain succesfull" << endl;

    }


    // check if non-agent goals strategic plan is valid
    std::string ProblemGenerator::create_all_dead_end_agent_goals_plan_from_strategic_plan(string all_dead_end_agent_goals_strategic_plan, string node){
        cout << "create_all_dead_end_agent_goals_plan_from_strategic_plan from " << all_dead_end_agent_goals_strategic_plan << endl;


        struct plan_line{
            string action_and_duration;
            double start_time;
            string parent_plan_name;
            int parent_plan_id;
            bool operator<(const plan_line& a) const
            {
                return start_time < a.start_time;
            }
        };


        //extract purely tactical plan from non-agent goals strategic plan
        vector<plan_line> plan;
        string line;
        ifstream all_dead_end_agent_goals_strategic_plan_file;
        all_dead_end_agent_goals_strategic_plan_file.open(all_dead_end_agent_goals_strategic_plan);
        double bug_extra_time = 0;
        string epsilon = "0.001";
        while (getline(all_dead_end_agent_goals_strategic_plan_file, line)) {
            plan_line action;
            size_t pos = line.find(":");
            size_t pos2 = line.find(".");
            int precision = static_cast<int>(pos) - static_cast<int>(pos2);
            cout << " eeepsilon 1 " << pos << " " << pos2 << " " << precision << endl;
            if (precision == 5){epsilon = "0.0001";}
            string time_string = line.substr(0,pos);
            double strategic_action_start_time = stod(time_string);
            //TODO fix bug
            //double strategic_action_start_time = stod(time_string) + bug_extra_time;
            //bug_extra_time += 0.05;
            action.start_time = strategic_action_start_time;
            action.action_and_duration = line.substr(pos,line.size());
            action.parent_plan_name = all_dead_end_agent_goals_strategic_plan;
            bool strategic_action = true;
            for (int i = 0; i < tactical_problem_names.size(); i++){
                string target = tactical_problem_names[i];
                //cout << target << endl;

                if (line.find(target+")", 0) != std::string::npos){
                    strategic_action = false;
                    ifstream tactical_plan_file;
                    string line_tactical;
                    tactical_plan_file.open(node+target +".plan.clean");
                    while (getline(tactical_plan_file, line_tactical)) {
                        //cout << line_tactical << endl;
                        if((line_tactical.find("(complete_tactical_mission)", 0) != string::npos) || (line_tactical.find("(stp_", 0) != string::npos))continue;
                        pos = line_tactical.find(":");
                        time_string = line_tactical.substr(0,pos);
//                        if ((strategic_action_start_time + stod(time_string)-0.001) >= 0){
//                            action.start_time = strategic_action_start_time + stod(time_string)-0.001;
//                        }
//                        else { action.start_time = strategic_action_start_time + stod(time_string);}
                        action.start_time = strategic_action_start_time + stod(time_string);
                        action.action_and_duration = line_tactical.substr(pos,line_tactical.size());
                        action.parent_plan_name = target;
                        action.parent_plan_id = i;
                        plan.push_back(action);
                    }
                    break;
                }
            }
            if(strategic_action)plan.push_back(action);
        }

        // create purely tactical plan file
        sort( plan.begin(), plan.end());
        ofstream purely_tactical_plan;
        purely_tactical_plan.open(all_dead_end_agent_goals_strategic_plan + ".purley_tactical");
        for (int i = 0; i < plan.size(); i++){
            //purely_tactical_plan << "                                                                                                       ;" << plan[i].parent_plan_name << endl;
            purely_tactical_plan << setprecision(15) << plan[i].start_time << plan[i].action_and_duration << "    ; " << plan[i].parent_plan_id << " tp" << endl;
        }

        //validate purely tactical plan
        bool isValid = false;
        stringstream cmd;
        cmd << "rm -rf validation.outcome";
        runCommand(cmd.str());
        cmd.str("");
        cmd << "/home/nq/RALSTP/VAL_latest/build/linux64/release/bin/./Validate -v -t " << epsilon << " " << original_domain_file_name << " " << node << "all_dead_end_agent_goals_" << original_problem_file_name << " " << all_dead_end_agent_goals_strategic_plan << ".purley_tactical >> validation.outcome";
        cout << "dead-end " << cmd.str();
        runCommand(cmd.str());
        ifstream validation_file;
        validation_file.open("validation.outcome");
        string line_validation;
        stringstream validation_log;
        validation_log << cmd.str() << endl;
        string output = "";
        while (getline(validation_file, line_validation)) {
            validation_log << line_validation << endl;
            if (line_validation.find("Plan valid", 0) != std::string::npos){
                output = "Plan valid";
                break;
            }
        }
        if (output == ""){
            output = validation_log.str();
        }
        return output;
    }

    // check if non-agent goals strategic plan is valid
    std::string ProblemGenerator::create_initial_problem_plan(string all_dead_end_agent_goals_strategic_plan, string all_parent_agent_goals_plan, double all_dead_end_agent_goals_plan_makespan, string node){


        cout << "create_initial_problem_plan" << endl;

        struct plan_line{
            string action_and_duration;
            double start_time;
            string parent_plan_name;
            bool operator<(const plan_line& a) const
            {
                return start_time < a.start_time;
            }
        };

        vector<plan_line> plan;
        string line;


        //add dead end agent goals plan and update its action start times
        ifstream initial_planning_problem_plan;
        initial_planning_problem_plan.open(all_dead_end_agent_goals_strategic_plan);
        string epsilon = "0.001";
        while (getline(initial_planning_problem_plan, line)) {
            plan_line action;
            size_t pos = line.find(":");
            size_t pos2 = line.find(".");
            int precision = static_cast<int>(pos) - static_cast<int>(pos2);
            cout << " eeepsilon 2 " << pos << " " << pos2 << " " << precision << endl;
            if (precision == 5){epsilon = "0.0001";}
            string time_string = line.substr(0,pos);
            action.start_time = stod(time_string);
            action.action_and_duration = line.substr(pos,line.size());
            plan.push_back(action);
        }

        //add parent agent goals plan and update its action start times
        ifstream agent_plan_file;
        agent_plan_file.open(all_parent_agent_goals_plan);
        while (getline(agent_plan_file, line)) {
            plan_line action;
            size_t pos = line.find(":");
            size_t pos2 = line.find(".");
            int precision = static_cast<int>(pos) - static_cast<int>(pos2);
            cout << " eeepsilon 3 " << pos << " " << pos2 << " " << precision << endl;
            if (precision == 5){epsilon = "0.0001";}
            string time_string = line.substr(0,pos);
            action.start_time = stod(time_string)+all_dead_end_agent_goals_plan_makespan+0.0002;
            action.action_and_duration = line.substr(pos,line.size());
            plan.push_back(action);
        }

        // create purely tactical plan file
        sort( plan.begin(), plan.end());
        ofstream purely_tactical_plan;
        purely_tactical_plan.open(all_dead_end_agent_goals_strategic_plan + ".initial");
        for (int i = 0; i < plan.size(); i++){
            //purely_tactical_plan << "                                                                                                       ;" << plan[i].parent_plan_name << endl;
            purely_tactical_plan << setprecision(15) << plan[i].start_time << plan[i].action_and_duration << endl;
        }

        //validate purely tactical plan
        bool isValid = false;
        stringstream cmd;
        cmd << "rm -rf validation.outcome";
        runCommand(cmd.str());
        cmd.str("");
        cmd << "/home/nq/RALSTP/VAL_latest/build/linux64/release/bin/./Validate -v -t " << epsilon << " " << original_domain_file_name << " " << original_problem_file_name << " " << all_dead_end_agent_goals_strategic_plan << ".initial >> validation.outcome";
        cout << cmd.str() << endl;
        runCommand(cmd.str());
        ifstream validation_file;
        validation_file.open("validation.outcome");
        string line_validation;
        stringstream validation_log;
        validation_log << cmd.str() << endl;
        string output = "";
        while (getline(validation_file, line_validation)) {
            validation_log << line_validation << endl;
            if (line_validation.find("Plan valid", 0) != std::string::npos){
                output = "Plan valid";
                break;
            }
        }
        if (output == ""){
            output = validation_log.str();
        }
        return output;
    }

    set<std::string> SimpleAgent::all_agent_names;
    ProblemGenerator::result ProblemGenerator::generateProblem(SubgoalsSet subgoals_set_, ostream & stats, double pre_makespan, double pre_plan_time, bool lm_tactical, int lm_subgoals, string node, bool all_dead_end_agent_goals) {

        cout << "generateProblem start" << current_decomposition << endl;
        result new_result;
        new_result.makespan = 99999999999999;
        new_result.plan_time = 99999999999999;
        new_result.max_planning_time = 0;
        new_result.valid = false;

        // clear used goals to identify non-lm goals
        solved_goals.clear();
        solved_goals_old.clear();

//        /*
//         * set agents
//         */
        //clear agents
        for(int i = 0; i < agent_groups.size(); ++i){
            agent_groups[i].clear();
        }
        //see if object is agent and create agent
        for (VAL::const_symbol_list::iterator sit = problem->objects->begin(); sit != problem->objects->end(); sit++){
            VAL::const_symbol* const_symbol_object = *sit;
            const_symbol_object->type = getObjectType(const_symbol_object->getName());
            //cout << "checking "<< const_symbol_object->getName() << " " << const_symbol_object->type->getName() << endl;
            if(isParentAgent(const_symbol_object->type->getName()) || isDeadEndAgent(const_symbol_object->type->getName())) {
                SimpleAgent* simpleAgent = new SimpleAgent(const_symbol_object,initial_goals,facts,values,func_name,func_args,operator_);

                // add agent to its group
                for(int j = 0; j < agent_groups.size(); ++j){
                    if(const_symbol_object->type->getName().compare(agent_groups[j].getType()) == 0){
                        agent_groups[j].addSimpleAgent(simpleAgent);
                        //cout << "added "<< const_symbol_object->getName() << " to " << agent_groups[j].getType() << endl;
                    }
                }
            }
        }

        //cout << "added " << endl;



        // initiate strategic domain generation
        stringstream strategic_domain_name;
        strategic_domain_name << node << "context_" << current_decomposition << "_strategic_domain_" << random << problem->name << ".pddl";
        ofstream strategic_domain(strategic_domain_name.str());

        /*
             * generate strategic domain
             */
        strategic_domain << "(define (domain " << domain->name << ")" << endl;
        strategic_domain << "  (:requirements " << VAL::pddl_req_flags_string(domain->req) << ")" << endl;
        strategic_domain << "  (:types";
        for(int i = 0; i < types.size(); i++){
            strategic_domain << " " <<  types[i].getName();
            if(supertypes[i].getName().compare("") != 0)strategic_domain << " - " << supertypes[i].getName();
        }
        strategic_domain << " mission)" << endl;



        // add predicates
        strategic_domain << "  (:predicates ";
        // add initial domain predicates
        for (VAL::pred_decl_list::iterator pit = domain->predicates->begin(); pit != domain->predicates->end(); pit++){
            VAL::pred_decl* predicate = *pit;
            //check if predicate is operator
            bool isOperator = false;
            for (VAL::operator_list::iterator oit = domain->ops->begin(); oit != domain->ops->end(); oit++){
                VAL::operator_* operator_ = *oit;
                if(predicate->getPred()->getName().compare(operator_->name->getName()) == 0){
                    isOperator = true;
                }
            }
            if(isOperator)continue;
            strategic_domain << endl << "    (" << predicate->getPred()->getName();
            for (VAL::var_symbol_list::const_iterator vit = predicate->getArgs()->begin();
                 vit != predicate->getArgs()->end(); vit++) {
                VAL::var_symbol* var_symbol_instance = *vit;
                strategic_domain << " ?" << var_symbol_instance->getName() << " - ";
                if(var_symbol_instance->type != NULL)strategic_domain << var_symbol_instance->type->getName();
                if(var_symbol_instance->either_types != NULL){
                    strategic_domain << "(either";
                    for(VAL::pddl_type_list::iterator it = var_symbol_instance->either_types->begin(); it != var_symbol_instance->either_types->end(); it++) {
                        VAL::pddl_type *temp_type = *it;
                        strategic_domain << " " << temp_type->getName();
                    }
                    strategic_domain << ")";
                }
            }
            strategic_domain << ")";
        }
        strategic_domain << endl;

        // add stp predicates
        for(int j = 0; j < agents_structure.size(); ++j){
            if (!agents_structure[j].is_parent) continue;
            strategic_domain << "    (stp_free_" << agents_structure[j].type << " ?" << agents_structure[j].type << " - " << agents_structure[j].type << ")" << endl;

            //TODO refactor code to for unlimited number of supertypes - current code only works for one supertype
            if(agent_groups[j].getSuperType(0).length() > 0){
                for (int k = 0; k < dynamic_types_vector.size(); k++){
                    if (agent_groups[j].getSuperType(0).compare(dynamic_types_vector[k]) == 0){
                        strategic_domain << "    (stp_free_" << agent_groups[j].getSuperType(0) << " ?" << agent_groups[j].getSuperType(0) << " - " << agent_groups[j].getSuperType(0) << ")" << endl;
                        break;
                    }
                }
            }
        }
        strategic_domain << "    (stp_complete_mission ?mission - mission)" << endl;
        strategic_domain << "  )" << endl << endl;

        //TODO add only parent agents as constants
        //add constants
        strategic_domain << "  (:constants " << endl;
        for (int q = 0; q < objects.size(); q++) {
            VAL::const_symbol* const_symbol_instance = &objects[q];
            if (isDeadEndAgent(const_symbol_instance->getName())){continue;}
            strategic_domain << "    " << const_symbol_instance->getName() << " - " << getObjectType(const_symbol_instance->getName())->getName() << endl;
        }
        for(int j = 0; j < subgoals_set_.getSet().size()+1; j++){
            strategic_domain << "mission" << j << " - mission" << endl;
        }
        strategic_domain << "  )" << endl;
        
        //add functions
        if(domain->functions != NULL){
            strategic_domain << "  (:functions ";

            for (VAL::func_decl_list::iterator fit = domain->functions->begin(); fit != domain->functions->end(); fit++){
                VAL::func_decl* function = *fit;
                strategic_domain << endl << "    (" << function->getFunction()->getName();
                for (VAL::var_symbol_list::const_iterator vit = function->getArgs()->begin();
                     vit != function->getArgs()->end(); vit++) {
                    VAL::var_symbol* var_symbol_instance = *vit;
                    strategic_domain << " ?" << var_symbol_instance->getName() << " - " << var_symbol_instance->type->getName();
                }
                strategic_domain << ")";
            }
            strategic_domain << "  )" << endl << endl;
        }

        // add initial problem operators
        for (VAL::operator_list::iterator oit = domain->ops->begin(); oit != domain->ops->end(); oit++){
            VAL::operator_* operator_ = *oit;
            strategic_domain << "(:durative-action " << operator_->name->getName() << endl;
            strategic_domain << "  :parameters(";
            for (VAL::var_symbol_list::iterator vit = operator_->parameters->begin();
                 vit != operator_->parameters->end(); vit++) {
                VAL::var_symbol* var_symbol_instance = *vit;
                strategic_domain << " ?" << var_symbol_instance->getName() << " - " << var_symbol_instance->type->getName();
            }
            strategic_domain << ")" << endl;
            strategic_domain << "  :duration ";
            last_expression.str("");
            operator_->visit(this);
            strategic_domain << last_expression.str() << endl;
            //cout << last_expression.str() << endl;
            strategic_domain << "  :condition\n    (and" << endl;

            //add stp conditions
            for (VAL::var_symbol_list::iterator vit = operator_->parameters->begin();
                 vit != operator_->parameters->end(); vit++) {
                VAL::var_symbol* var_symbol_instance = *vit;
                if(isParentAgent(var_symbol_instance->type->getName())){
                    strategic_domain << "    (at start (stp_free_" << var_symbol_instance->type->getName() << " ?" <<var_symbol_instance->getName() << "))" << endl;
                }
            }
            //add predefined conditions
            last_operator_propositions.clear();
            last_operator_time_specs.clear();
            last_negative_effects.clear();
            last_operator_string_propositions.clear();
            operator_->precondition->visit(this);
            for(int j = 0; j < last_operator_string_propositions.size(); j++){
                //cout << "    here str cond: " << last_operator_string_propositions[j] << endl;
                strategic_domain << "    (" << last_operator_time_specs[j] << " ";
                if(last_negative_effects[j])strategic_domain << " (not";
                strategic_domain << last_operator_string_propositions[j];
                if(last_negative_effects[j])strategic_domain << ")";
                strategic_domain << ")" << endl;
            }
            strategic_domain << "    )" << endl;




            strategic_domain << "  :effect\n    (and" << endl;

            //add stp effects
            for (VAL::var_symbol_list::iterator vit = operator_->parameters->begin();
                 vit != operator_->parameters->end(); vit++) {
                VAL::var_symbol* var_symbol_instance = *vit;
                if(isParentAgent(var_symbol_instance->type->getName())){
                    strategic_domain << "    (at start (not (stp_free_" << var_symbol_instance->type->getName() << " ?" <<var_symbol_instance->getName() << ")))" << endl;
                    strategic_domain << "    (at end (stp_free_" << var_symbol_instance->type->getName() << " ?" <<var_symbol_instance->getName() << "))" << endl;
                }
            }
            //add predefined effects
            last_operator_propositions.clear();
            last_operator_time_specs.clear();
            last_negative_effects.clear();
            last_operator_string_propositions.clear();
            operator_->effects->visit(this);
            for(int j = 0; j < last_operator_string_propositions.size(); j++){
                //cout << "    here str eff: " << last_operator_string_propositions[j] << endl;
                strategic_domain << "    (" << last_operator_time_specs[j] << " ";
                if(last_negative_effects[j])strategic_domain << "(not ";
                strategic_domain << last_operator_string_propositions[j];
                if(last_negative_effects[j])strategic_domain << ")";
                strategic_domain << ")" << endl;
            }
            strategic_domain << "    )" << endl;
            strategic_domain << "  )" << endl << endl;
        }


        /*
         * generate tactical subproblems of the current strategic decomposition
         */

        // output initial subgoal set stats
        int subgoals_size = subgoals_set_.getSet().size();
//        if((goals.size() - subgoals_set_.getGoalsNoInSubgoals()) > 0)subgoals_size++;
        stats << "Total Parent Agent + Dead-end Agent Goals: " << goals.size() << " Total Dead-end Agent Subgoals: " << subgoals_size << " Max-subgoal-size: " << subgoals_set_.getMaxSubgoalSize() << " Leftover goals: " << dead_end_agent_goals - subgoals_set_.getGoalsNoInSubgoals() << endl << endl;
        current_goals_in_all_subgoals = subgoals_set_.getGoalsNoInSubgoals();

        // loop through the subgoals in the set
        vector<bool> usefull_subgoals;
        string line;
        stats << "Total top level goals: " << goals.size() << endl;
        stats << "No of subgoals: " << subgoals_set_.getSet().size() << endl;
        bool backup_planner = only_backup_planner;

        for (int i = 0; i < subgoals_set_.getSet().size(); i++) {
            if (std::time(0) - procedure_start_time > max_time) {
                stats << "Abandoned: Not starting Tactical Problem. Total-Time OVER: " << max_time;
                break;
            }

            usefull_subgoals.push_back(false);
            if ((i < lm_subgoals) && (lm_tactical)) stats << "Landmark Subgoal" << endl;
            else stats << "NON-Landmark Subgoal" << endl;
            int threshold = 0;
            solved_goals = solved_goals_old;
            if(goals.size() == solved_goals.size())break;

            /*
             * prepare goals
            */
            vector<int> subproblem_goals;
            current_dead_end_agent_goal_set.clear();
            stringstream goals_ss;
            threshold = 120;
            for (int j = 0; j < initial_goals.size(); j++) {
                if (isParentAgentProposition(initial_goals[j])) {continue;}
                bool goal_solved = false;
                for (int q = 0; q < goals_in_initial_state.size(); q++){
                    if(j == goals_in_initial_state[q]) { goal_solved = true;break;}
                }
                for(int k = 0; k < subgoals_set_.getSet()[i].size(); k++){
                    if((j == subgoals_set_.getSet()[i][k]) && !goal_solved){
                        solved_goals.push_back(j);
                        subproblem_goals.push_back(j);

                        stringstream prop_ss;
                        for (VAL::parameter_symbol_list::iterator git = initial_goals[j]->args->begin();
                             git != initial_goals[j]->args->end(); git++) {
                            VAL::parameter_symbol *parameter_symbol_instance = *git;
                            prop_ss << " " << parameter_symbol_instance->getName();
                        }
                        break;
                    }
                }
            }
            if (subproblem_goals.size() == 0){
                stats << "   Solved. Goal state is achieved in the initial state " << endl << endl;
                continue;
            }


            //add parent agent goals to parent agents
            for(int j = 0; j < agent_groups.size(); ++j){
                if(!agent_groups[j].has_parent_parent_dynamic_type()) continue;
                for(int k = 0; k < agent_groups[j].getAgents().size(); k++){

                    agent_groups[j].getAgents()[k]->clear();
                    agent_groups[j].getAgents()[k]->setSubproblemGoals(subproblem_goals);
                    //process agent constraints if required
                    agent_groups[j].getAgents()[k]->processConstraints();
                }
            }
            current_dead_end_agent_goal_set = subproblem_goals;


            //determine goals not part of the current dead-end agent goal-set
//            for(int j = 0; j < goals.size(); ++j){
//                bool used = false;
//                for(int k = 0; k < subproblem_goals.size(); k++){
//                    if(j == subproblem_goals[k]){
//                        used = true;
//                        break;
//                    }
//                }
//                if(!used){
//                    irelevant_goals.push_back(j);
//                }
//            }

            /*
             * generate tactical domain
             */
//            //TODO generate to an indipendet domain for particular problem
            stringstream tactical_domain_name;


            if(i == subgoals_set_.getSet().size())tactical_domain_name << node << "context_" << current_decomposition << "_goalset_no_" << i << "_goalset_size_" << "NO_LM_" << subproblem_goals.size() << "_tactical_domain_" << random << problem->name;
            else tactical_domain_name << node << "context_" << current_decomposition << "_goalset_no_" << i << "_goalset_size_" << subgoals_set_.getSet()[i].size() << "_tactical_domain_" << random << problem->name;

            ofstream tactical_domain(tactical_domain_name.str()+".pddl");



            tactical_domain << "(define (domain " << domain->name << ")" << endl;
            tactical_domain << "  (:requirements " << VAL::pddl_req_flags_string(domain->req) << ")" << endl;
            tactical_domain << "  (:types";
            for(int i = 0; i < types.size(); i++){
                tactical_domain << " " <<  types[i].getName();
                if(supertypes[i].getName().compare("") != 0)tactical_domain << " - " << supertypes[i].getName();
            }
            tactical_domain << ")" << endl;
            tactical_domain << "  (:predicates ";
            // add initial domain predicates
            for (VAL::pred_decl_list::iterator pit = domain->predicates->begin(); pit != domain->predicates->end(); pit++){
                VAL::pred_decl* predicate = *pit;
                //check if predicate is operator
                bool isOperator = false;
                for (VAL::operator_list::iterator oit = domain->ops->begin(); oit != domain->ops->end(); oit++){
                    VAL::operator_* operator_ = *oit;
                    if(predicate->getPred()->getName().compare(operator_->name->getName()) == 0){
                        isOperator = true;
                    }
                }
                if(isOperator)continue;
                tactical_domain << endl << "    (" << predicate->getPred()->getName();
                for (VAL::var_symbol_list::const_iterator vit = predicate->getArgs()->begin();
                     vit != predicate->getArgs()->end(); vit++) {
                    VAL::var_symbol* var_symbol_instance = *vit;
                    tactical_domain << " ?" << var_symbol_instance->getName() << " - ";
                    if(var_symbol_instance->type != NULL)tactical_domain << var_symbol_instance->type->getName();
                    if(var_symbol_instance->either_types != NULL){
                        tactical_domain << "(either";
                        for(VAL::pddl_type_list::iterator it = var_symbol_instance->either_types->begin(); it != var_symbol_instance->either_types->end(); it++) {
                            VAL::pddl_type *temp_type = *it;
                            tactical_domain << " " << temp_type->getName();
                        }
                        tactical_domain << ")";
                    }
                }
                tactical_domain << ")";
            }
            tactical_domain << endl;
            // add stp predicates
            for(int j = 0; j < agents_structure.size(); ++j){
                if (!agents_structure[j].is_parent) continue;
                tactical_domain << "    (stp_available_" << agents_structure[j].type << " ?" << agents_structure[j].type << " - " << agents_structure[j].type << ")" << endl;
                tactical_domain << "    (stp_selected_" << agents_structure[j].type << " ?" << agents_structure[j].type << " - " << agents_structure[j].type << ")" << endl;
                tactical_domain << "    (stp_not_selected_" << agents_structure[j].type << ")" << endl;

                //TODO refactor code to for unlimited number of supertypes - current code only works for one supertype
                if(agent_groups[j].getSuperType(0).length() > 0){
                    for (int k = 0; k < dynamic_types_vector.size(); k++){
                        if (agent_groups[j].getSuperType(0).compare(dynamic_types_vector[k]) == 0){
                            tactical_domain << "    (stp_selected_" << agent_groups[j].getSuperType(0) << " ?" << agent_groups[j].getSuperType(0) << " - " << agent_groups[j].getSuperType(0) << ")" << endl;
                            break;
                        }
                    }
                }

                for (int k = 0; k < agent_groups[j].getAgents()[0]->getOutputFactsAttributes().size(); k++){

                    string attribute_type = agent_groups[j].getAgents()[0]->getOutputFactsAttributes()[k]->type->getName();
                    tactical_domain << "    (stp_target_" << agents_structure[j].type << "_" << attribute_type << " ?" << attribute_type << " - " << attribute_type << ")" << endl;
                }
                tactical_domain << "    (stp_" << agents_structure[j].type << "_complete)" << endl;
            }
            tactical_domain << "    (stp_complete_mission)" << endl;

            tactical_domain << "  )" << endl << endl;

            // add functions

            tactical_domain << "  (:functions ";
            if(domain->functions != NULL){
                for (VAL::func_decl_list::iterator fit = domain->functions->begin(); fit != domain->functions->end(); fit++){
                    VAL::func_decl* function = *fit;
                    //check if predicate is operator
                    bool isOperator = false;
                    for (VAL::operator_list::iterator oit = domain->ops->begin(); oit != domain->ops->end(); oit++){
                        VAL::operator_* operator_ = *oit;
                        if(function->getFunction()->getName().compare(operator_->name->getName()) == 0){
                            isOperator = true;
                        }
                    }
                    if(isOperator)continue;
                    tactical_domain << endl << "    (" << function->getFunction()->getName();
                    for (VAL::var_symbol_list::const_iterator vit = function->getArgs()->begin();
                         vit != function->getArgs()->end(); vit++) {
                        VAL::var_symbol* var_symbol_instance = *vit;
                        tactical_domain << " ?" << var_symbol_instance->getName() << " - " << var_symbol_instance->type->getName();
                    }
                    tactical_domain << ")";
                }
                tactical_domain << endl;

            }

            tactical_domain << "  )" << endl << endl;


            // add constants
            tactical_domain << "  (:constants ";
            for (int q = 0; q < objects.size(); q++) {
                VAL::const_symbol* const_symbol_object = &objects[q];
                const_symbol_object->type = getObjectType(const_symbol_object->getName());
//                if (isDeadEndAgent(const_symbol_object->type->getName())){
//                    bool found = false;
//                    for (int q = 0; q < subproblem_goals.size(); q++){
//                        if (found == true) {break;}
//                        int goal_index = subproblem_goals[q];
//                        for (VAL::parameter_symbol_list::iterator git = goals[goal_index]->args->begin();
//                             git != goals[goal_index]->args->end(); git++) {
//                            VAL::parameter_symbol *parameter_symbol_instance = *git;
//                            if (parameter_symbol_instance->getName().compare(const_symbol_object->getName()) ==0){
//                                tactical_domain << endl << "    " << const_symbol_object->getName() << " - " << const_symbol_object->type->getName();
//                                found = true;
//                                break;
//                            }
//                        }
//                    }
//                }
//                else{
//                    tactical_domain << endl << "    " << const_symbol_object->getName() << " - " << const_symbol_object->type->getName();
//                }
                tactical_domain << endl << "    " << const_symbol_object->getName() << " - " << const_symbol_object->type->getName();
            }
            tactical_domain << endl << "  )" << endl << endl;



            // add initial problem operators
            for (VAL::operator_list::iterator oit = domain->ops->begin(); oit != domain->ops->end(); oit++){
                VAL::operator_* operator_ = *oit;
                tactical_domain << "(:durative-action " << operator_->name->getName() << endl;
                //cout << "!!! START ACTION    " << operator_->name->getName() << endl;
                tactical_domain << "  :parameters(";
                for (VAL::var_symbol_list::iterator vit = operator_->parameters->begin();
                     vit != operator_->parameters->end(); vit++) {
                    VAL::var_symbol* var_symbol_instance = *vit;
                    tactical_domain << " ?" << var_symbol_instance->getName() << " - " << var_symbol_instance->type->getName();
                }
                tactical_domain << ")" << endl;
                tactical_domain << "  :duration ";
                last_expression.str("");
                operator_->visit(this);
                tactical_domain << last_expression.str() << endl;
                //cout << last_expression.str() << endl;
                tactical_domain << "  :condition\n    (and" << endl;

                //add stp conditions
                for (VAL::var_symbol_list::iterator vit = operator_->parameters->begin();
                     vit != operator_->parameters->end(); vit++) {
                    VAL::var_symbol* var_symbol_instance = *vit;
                    if(isParentAgent(var_symbol_instance->type->getName())){
                        tactical_domain << "    (at start (stp_selected_" << var_symbol_instance->type->getName() << " ?" <<var_symbol_instance->getName() << "))" << endl;
                    }
                }


                last_operator_propositions.clear();
                last_operator_time_specs.clear();
                last_negative_effects.clear();
                last_operator_string_propositions.clear();
                operator_->precondition->visit(this);
                for(int j = 0; j < last_operator_string_propositions.size(); j++){
                    //cout << "    here cond: " << last_operator_string_propositions[j] << endl;
                    tactical_domain << "    (" << last_operator_time_specs[j] << " ";
                    if(last_negative_effects[j])tactical_domain << " (not";
                    tactical_domain << last_operator_string_propositions[j];
                    if(last_negative_effects[j])tactical_domain << ")";
                    tactical_domain << ")" << endl;
                }
                tactical_domain << "    )" << endl;



                tactical_domain << "  :effect\n    (and" << endl;
                last_operator_propositions.clear();
                last_operator_time_specs.clear();
                last_negative_effects.clear();
                last_operator_string_propositions.clear();
                operator_->effects->visit(this);
                for(int j = 0; j < last_operator_string_propositions.size(); j++){
                    //cout << "    here eff: " << last_operator_string_propositions[j] << endl;
                    tactical_domain << "    (" << last_operator_time_specs[j] << " ";
                    if(last_negative_effects[j])tactical_domain << "(not ";
                    tactical_domain << last_operator_string_propositions[j];
                    if(last_negative_effects[j])tactical_domain << ")";
                    tactical_domain << ")" << endl;
                }
                tactical_domain << "    )" << endl;
                tactical_domain << "  )" << endl << endl;
            }

            // add PDDL "if else" equivalent for agents in tactical domain
            for (int j = 0; j < agent_groups.size(); j++) {
                if(!agent_groups[j].has_parent_parent_dynamic_type()) continue;

                // add stp_select action

                /*
                 * Durative Action Version
                 */
                tactical_domain << "(:durative-action stp_select_" << agent_groups[j].getType() << endl;
                tactical_domain << "  :parameters(?" << agent_groups[j].getType() << " - " << agent_groups[j].getType() << ")" << endl;
                tactical_domain << "  :duration(= ?duration 0)" << endl;
                tactical_domain << "  :condition\n    (and" << endl;
                //TODO add numeric condition to allow multiple agents (gradually increasing)
                    tactical_domain << "      (at start (stp_not_selected_" << agent_groups[j].getType() << "))" << endl;
                tactical_domain << "      (at start (stp_available_" << agent_groups[j].getType() << " ?" << agent_groups[j].getType() << "))" << endl;
                tactical_domain << "    )" << endl;
                tactical_domain << "  :effect\n    (and" << endl;
                //TODO replace with decrease effect to allow multiple agents
                    tactical_domain << "      (at start (not (stp_not_selected_" << agent_groups[j].getType() << ")))" << endl;
                tactical_domain << "      (at start (stp_selected_" << agent_groups[j].getType() << " ?" << agent_groups[j].getType() << "))" << endl;

                //TODO add only supertypes that are parent dynamic types
                if(agent_groups[j].getSuperType(0).length() > 0) {
                    for (int k = 0; k < dynamic_types_vector.size(); k++){
                        if (agent_groups[j].getSuperType(0).compare(dynamic_types_vector[k]) == 0) {
                            tactical_domain << "      (at start (stp_selected_" << agent_groups[j].getSuperType(0)
                                            << " ?" << agent_groups[j].getType() << "))" << endl;
                            break;
                        }
                    }
                }

                tactical_domain << "    )" << endl;
                tactical_domain << "  )" << endl << endl << endl;

                /*
                 * Classical Action Version
                 */
//                tactical_domain << "(:action stp_select_" << agent_groups[j].getType() << endl;
//                tactical_domain << "  :parameters(?" << agent_groups[j].getType() << " - " << agent_groups[j].getType() << ")" << endl;
//                tactical_domain << "  :precondition\n    (and" << endl;
//                tactical_domain << "      (stp_not_selected_" << agent_groups[j].getType() << ")" << endl;
//                tactical_domain << "      (stp_available_" << agent_groups[j].getType() << " ?" << agent_groups[j].getType() << ")" << endl;
//                tactical_domain << "    )" << endl;
//                tactical_domain << "  :effect\n    (and" << endl;
//                tactical_domain << "      (not (stp_not_selected_" << agent_groups[j].getType() << "))" << endl;
//                tactical_domain << "      (stp_selected_" << agent_groups[j].getType() << " ?" << agent_groups[j].getType() << ")" << endl;
//
//                //TODO add only supertypes that are parent dynamic types
//                if(agent_groups[j].getSuperType(0).length() > 0){
//                    tactical_domain << "      (stp_selected_" << agent_groups[j].getSuperType(0) << " ?" << agent_groups[j].getType() << ")" << endl;
//                }
//
//                tactical_domain << "    )" << endl;
//                tactical_domain << "  )" << endl << endl << endl;


                for(int k = 0; k < agent_groups[j].size(); k++){
                    if(!agent_groups[j].has_parent_parent_dynamic_type()) continue;
                    agent_groups[j].getAgents()[k]->agentCaseAction(tactical_domain);
                }
                //               }


            }

            // add complete_tactical_mission action to tactical domain

            /*
             * Durative Action Version
             */
            tactical_domain << "(:durative-action complete_tactical_mission" << endl;
            tactical_domain << "  :parameters( ";
//            for(int j = 0; j < agents_structure.size(); ++j){
//                tactical_domain << "?" << agents_structure[j].type << " - " << agents_structure[j].type << " ";
//                for (int k = 0; k < agent_groups[j].getAgents()[0]->getOutputFactsAttributes().size(); k++){
//                    string attribute_type = agent_groups[j].getAgents()[0]->getOutputFactsAttributes()[k]->type->getName();
//                    tactical_domain << "?" << agents_structure[j].type << "_" << attribute_type << " - " << attribute_type << " ";
//                }
//            }
            tactical_domain << ")" << endl;
            tactical_domain << "  :duration(= ?duration 0)" << endl;
            tactical_domain << "  :condition\n    (and" << endl;

            // non-agent subproblem goals  //TODO Analyse this for the thesis, as without it the problem is much more harder to solve
            for (int j = 0; j < subproblem_goals.size(); ++j){
                stringstream prop_ss;
                for (VAL::parameter_symbol_list::iterator git = initial_goals[subproblem_goals[j]]->args->begin();
                     git != initial_goals[subproblem_goals[j]]->args->end(); git++) {
                    VAL::parameter_symbol *parameter_symbol_instance = *git;
                    prop_ss << " " << parameter_symbol_instance->getName();
                }
                tactical_domain << "      (at start (" << initial_goals[subproblem_goals[j]]->head->getName() << prop_ss.str() << "))" << endl;
            }

            for(int j = 0; j < agents_structure.size(); ++j){
                if (!agents_structure[j].is_parent) continue;
                tactical_domain << "      (at start (stp_" << agents_structure[j].type << "_complete))" << endl;


                // add stp_selected conditions
//                tactical_domain << "      (at start (stp_selected_" << agents_structure[j].type << " ?" << agents_structure[j].type << "))" << endl;

                // add output facts conditions
//                for (int k = 0; k < agent_groups[j].getAgents()[0]->getOutputFacts().size(); k++){
//                    VAL::proposition* parent_agent_fact = agent_groups[j].getAgents()[0]->getOutputFacts()[k];
//                    tactical_domain << "      (at start (" << parent_agent_fact->head->getName() << " ";
//                    for (VAL::parameter_symbol_list::iterator pit = parent_agent_fact->args->begin();
//                         pit != parent_agent_fact->args->end(); pit++) {
//                        VAL::parameter_symbol* parameter_symbol_instance = *pit;
//
//                        //TODO super super types
//
//                        // add attribute conditions
//                        bool type_match = false;
//                        for (int q = 0; q < agent_groups[j].getSuperTypes().size(); q++){
//                            if((parameter_symbol_instance->type->getName().compare(agent_groups[j].getType()) == 0) || (parameter_symbol_instance->type->getName().compare(agent_groups[j].getSuperType(q)) == 0)){
//                                tactical_domain << "?" << agent_groups[j].getType() << " ";
//                                type_match = true;
//                                break;
//                            }
//                        }
//                        if (type_match)continue;
//                        else{
//                            std::cout << "xtype: " << parameter_symbol_instance->type->getName() << " " << agent_groups[j].getType() << " " << agent_groups[j].getSuperType(0) << endl;
//                            std::cout << "supertype 2 " << agent_groups[j].getSuperType(1) << endl;
//                            tactical_domain << "?" << agent_groups[j].getType() << "_" << parameter_symbol_instance->type->getName() << " ";
//                            std::cout << "?" << agent_groups[j].getType() << "_" << parameter_symbol_instance->type->getName() << " " << endl;
//                        }
//                    }
//                    tactical_domain << "))" << endl;
//                }

                // add stp_target conditions
//                for (int k = 0; k < agent_groups[j].getAgents()[0]->getOutputFactsAttributes().size(); k++){
//                    string attribute_type = agent_groups[j].getAgents()[0]->getOutputFactsAttributes()[k]->type->getName();
//                    tactical_domain << "      (at start (stp_target_" << agents_structure[j].type << "_" << attribute_type << " ?" << agents_structure[j].type << "_" << attribute_type << "))" << endl;
//                }

                // add initial single attribute agent facts
//                for (int k = 0; k < agent_groups[j].getAgents()[0]->getNonGoalInitialStateFacts().size(); k++){
//                    tactical_domain << "      (at start (" << agent_groups[j].getAgents()[0]->getNonGoalInitialStateFacts()[k] << " ?" << agents_structure[j].type << "))" << endl;
//                }


            }
            tactical_domain << "    )" << endl;

            // add effects
            tactical_domain << "  :effect\n    (and" << endl;
            tactical_domain << "      (at start (stp_complete_mission))" << endl;
            tactical_domain << "    )" << endl << "  )" << endl;


            /*
             * Classical Action Version
             */
//            tactical_domain << "(:action complete_tactical_mission" << endl;
//            tactical_domain << "  :parameters( ";
//            tactical_domain << ")" << endl;
//            tactical_domain << "  :precondition\n    (and" << endl;
//
//            // non-agent subproblem goals  //TODO Analyse this for the thesis, as without it the problem is much more harder to solve
//            for (int j = 0; j < subproblem_goals.size(); ++j){
//                stringstream prop_ss;
//                for (VAL::parameter_symbol_list::iterator git = goals[subproblem_goals[j]]->args->begin();
//                     git != goals[subproblem_goals[j]]->args->end(); git++) {
//                    VAL::parameter_symbol *parameter_symbol_instance = *git;
//                    prop_ss << " " << parameter_symbol_instance->getName();
//                }
//                tactical_domain << "      (" << goals[subproblem_goals[j]]->head->getName() << prop_ss.str() << ")" << endl;
//            }
//
//            for(int j = 0; j < agents_structure.size(); ++j){
//                if (!agents_structure[j].is_parent) continue;
//                tactical_domain << "      (stp_" << agents_structure[j].type << "_complete)" << endl;
//
//            }
//            tactical_domain << "    )" << endl;
//
//            // add effects
//            tactical_domain << "  :effect\n    (and" << endl;
//            tactical_domain << "      (stp_complete_mission)" << endl;
//            tactical_domain << "    )" << endl << "  )" << endl;

            /*
             * Durative Action Version
             */
            // add mission_already_completed action to tactical domain
//            tactical_domain << "(:durative-action mission_already_completed" << endl;
//            tactical_domain << "  :parameters()" << endl;
//            tactical_domain << "  :duration(= ?duration 0)" << endl;
//            tactical_domain << "  :condition\n    (and" << endl;
//
//            // non-agent subproblem goals as conditions
//            for (int j = 0; j < subproblem_goals.size(); ++j){
//                stringstream prop_ss;
//                for (VAL::parameter_symbol_list::iterator git = goals[subproblem_goals[j]]->args->begin();
//                     git != goals[subproblem_goals[j]]->args->end(); git++) {
//                    VAL::parameter_symbol *parameter_symbol_instance = *git;
//                    prop_ss << " " << parameter_symbol_instance->getName();
//                }
//                tactical_domain << "      (at start (" << goals[subproblem_goals[j]]->head->getName() << prop_ss.str() << "))" << endl;
//            }
//            // stp conditions
//            for(int j = 0; j < agents_structure.size(); ++j) {
//                if (!agents_structure[j].is_parent) continue;
//                tactical_domain << "      (at start (stp_not_selected_" << agent_groups[j].getType() << "))" << endl;
//            }
//            tactical_domain << "    )" << endl;
//            tactical_domain << "  :effect\n    (and" << endl;
//            tactical_domain << "      (at start (stp_complete_mission))" << endl;
//            tactical_domain << "    )" << endl << "  )" << endl;


            /*
             * Classical Action Version
             */
            // add mission_already_completed action to tactical domain
//            tactical_domain << "(:action mission_already_completed" << endl;
//            tactical_domain << "  :parameters()" << endl;
//            tactical_domain << "  :precondition\n    (and" << endl;
//
//            // non-agent subproblem goals as conditions
//            for (int j = 0; j < subproblem_goals.size(); ++j){
//                stringstream prop_ss;
//                for (VAL::parameter_symbol_list::iterator git = goals[subproblem_goals[j]]->args->begin();
//                     git != goals[subproblem_goals[j]]->args->end(); git++) {
//                    VAL::parameter_symbol *parameter_symbol_instance = *git;
//                    prop_ss << " " << parameter_symbol_instance->getName();
//                }
//                tactical_domain << "      (" << goals[subproblem_goals[j]]->head->getName() << prop_ss.str() << ")" << endl;
//            }
//            // stp conditions
//            for(int j = 0; j < agents_structure.size(); ++j) {
//                if (!agents_structure[j].is_parent) continue;
//                tactical_domain << "      (stp_not_selected_" << agent_groups[j].getType() << ")" << endl;
//            }
//            tactical_domain << "    )" << endl;
//            tactical_domain << "  :effect\n    (and" << endl;
//            tactical_domain << "      (stp_complete_mission)" << endl;
//            tactical_domain << "    )" << endl << "  )" << endl;









            tactical_domain << ")" << endl << endl << endl;


            /*
             * generate subproblem
             */
            stringstream problem_name;
            if(i == subgoals_set_.getSet().size())problem_name << "context_" << current_decomposition << "_goalset_no_" << i << "_goalset_size_" << "NO_LM_" << subproblem_goals.size() << "_tactical_problem_" << random << problem->name;
            else problem_name << "context_" << current_decomposition << "_goalset_no_" << i << "_goalset_size_" << subgoals_set_.getSet()[i].size() << "_tactical_problem_" << random << problem->name;
            string t_corrected_name = problem_name.str();
            problem_name.str(node+problem_name.str());
            ofstream tactical_problem(problem_name.str()+".pddl");
            tactical_problem << "(define (problem " << t_corrected_name << ")" << endl;
            tactical_problem << "   (:domain " << problem->domain_name << ")" << endl;

            //add instances
            tactical_problem << "   (:objects" << endl;
            vector<string> lm_agents;
//            for (VAL::const_symbol_list::iterator sit = problem->objects->begin(); sit != problem->objects->end(); sit++){
//                VAL::const_symbol* const_symbol_object = *sit;
//                const_symbol_object->type = getObjectType(const_symbol_object->getName());

//               if(!isInstanceInInitialFacts(problem->objects[j].getName()))continue;
//                if(isParentAgent(objects[j].type->getName())) {
//                    SimpleAgent* simpleAgent = new SimpleAgent(objects[j],subproblem_goals,goals,facts,values,func_name,func_args,operator_);
//                }


//                if(isParentAgent(const_symbol_object->type->getName())) {
//                    bool lm_agent_added = false;
//                    for(int k = 0; k < lm_agents.size(); k++){
//                        if (const_symbol_object->type->getName().compare(getObjectType(lm_agents[k])->getName()) == 0){
//                            lm_agent_added = true;
//                        }
//                    }
//                    if(!lm_agent_added){
//                        lm_agents.push_back(const_symbol_object->getName());
//                    }
//                    else{
//                        continue;
//                    }
//                }



//            }
            tactical_problem << "   )" << endl;

            //if agent has goal state and priority was solved, replace initial state stuff with goal state stuff in tactical problem facts and functions


            //add facts
            stringstream agent_goals_ss;
            tactical_problem << "   (:init" << endl;

            for (int j = 0; j < facts.size(); j++) {
                //check if fact contains agents and update if agent has goal facts
//                VAL::proposition* fact = updateIfAgentGoalState(facts[j]);
                VAL::proposition* fact = facts[j];
//                o << "      (" << fact->head->getName();
                stringstream prop_ss;
                bool parent_agent_fact = false;
                if(!isRejectedFact(fact)){
                    for (VAL::parameter_symbol_list::iterator pit = fact->args->begin();
                         pit != fact->args->end(); pit++) {
                        VAL::parameter_symbol *parameter_symbol_instance = *pit;
                        prop_ss << " " << parameter_symbol_instance->getName();

                        if(isParentAgent(parameter_symbol_instance->getName())) {
                            parent_agent_fact = true;
                        }

                    }
                    tactical_problem << "      (" << fact->head->getName() << prop_ss.str() << ")" << endl;
                }
                if(parent_agent_fact){
                    agent_goals_ss << "      (" << fact->head->getName() << prop_ss.str() << ")" << endl;
                }
            }


            //add functions
            for (int j = 0; j < func_name.size(); j++) {
                tactical_problem << "      (" << operator_[j] << " (" << func_name[j];
                bool irelevant = false;
                for (VAL::parameter_symbol_list::iterator fit = func_args[j]->begin();
                     fit != func_args[j]->end(); fit++) {
                    VAL::parameter_symbol *parameter_symbol_instance = *fit;
                    tactical_problem << " " << parameter_symbol_instance->getName();
                }
                tactical_problem << ") " << values[j] << ")" << endl;
            }

            //add stp agent type facts
            for (int j = 0; j < agents_structure.size(); j++) {
                if (!agents_structure[j].is_parent) continue;
                tactical_problem << "      (stp_not_selected_" << agents_structure[j].type << ")" << endl;
            }


            //add stp agent instance facts and output facts
            stringstream before_available_ss;
            int parent_agent_groups_index = 0;
            for (int j = 0; j < agent_groups.size(); j++) {
                vector<int> available_agents;
                if(!agent_groups[j].has_parent_parent_dynamic_type()) {continue;}
                for(int k = 0; k < agent_groups[j].size(); k++){
//                    if(agent_constraints){
                    if(agent_contstraints_vector[parent_agent_groups_index]){
                        if(agent_groups[j].getRemainingPriorityAgents() > 0){
                            if(agent_groups[j].getAgents()[k]->isPriority()){
                                available_agents.push_back(k);
                                before_available_ss << agent_groups[j].getAgents()[k]->getName() << " ";
                                tactical_problem << "      (stp_available_" << agent_groups[j].getAgents()[k]->getType() << " " << agent_groups[j].getAgents()[k]->getName() << ")" << endl;
                            }
                        }
                        else{
                            if(agent_groups[j].getAgents()[k]->isAvailable()){
                                available_agents.push_back(k);
                                before_available_ss << agent_groups[j].getAgents()[k]->getName() << " ";
                                tactical_problem << "      (stp_available_" << agent_groups[j].getAgents()[k]->getType() << " " << agent_groups[j].getAgents()[k]->getName() << ")" << endl;
                            }
                        }
                    }
                        //allow all agents to compete if no agent constraints and not reused(agent constraints are removed if no plan found with constraints)
                    else{
                        available_agents.push_back(k);
                        //if(!agent_groups[j].getAgents()[k]->isReused()){
                        tactical_problem << "      (stp_available_" << agent_groups[j].getAgents()[k]->getType() << " " << agent_groups[j].getAgents()[k]->getName() << ")" << endl;
                        //}
                    }
                }
                //int random_agent = available_agents[rand() % available_agents.size()]; tactical_problem << "      (stp_available_" << agent_groups[j].getAgents()[random_agent]->getType() << " " << agent_groups[j].getAgents()[random_agent]->getName() << ")" << endl;
                //int random_agent = available_agents[0]; tactical_problem << "      (stp_available_" << agent_groups[j].getAgents()[random_agent]->getType() << " " << agent_groups[j].getAgents()[random_agent]->getName() << ")" << endl;
                parent_agent_groups_index++;
            }
            //stats << "Available Parent Agents: " << before_available_ss.str() << endl;
            tactical_problem << "   )" << endl;

            //add goals
            tactical_problem << "   (:goal (and" << endl;
//            for (int j = 0; j < subproblem_goals.size(); ++j){
//                stringstream prop_ss;
//                for (VAL::parameter_symbol_list::iterator git = goals[subproblem_goals[j]]->args->begin();
//                     git != goals[subproblem_goals[j]]->args->end(); git++) {
//                    VAL::parameter_symbol *parameter_symbol_instance = *git;
//                    prop_ss << " " << parameter_symbol_instance->getName();
//                }
//                o << "      (" << goals[subproblem_goals[j]]->head->getName() << prop_ss.str() << ")" << endl;
//            }

            //cout << subproblem_goals.size() << endl;
            for (int j = 0; j < subproblem_goals.size(); ++j){
                stringstream prop_ss;
                for (VAL::parameter_symbol_list::iterator git = initial_goals[subproblem_goals[j]]->args->begin();
                     git != initial_goals[subproblem_goals[j]]->args->end(); git++) {
                    VAL::parameter_symbol *parameter_symbol_instance = *git;
                    prop_ss << " " << parameter_symbol_instance->getName();
                }
                //cout << "inserted in goals      (" << initial_goals[subproblem_goals[j]]->head->getName() << prop_ss.str() << ")" << endl;
                tactical_problem << "      ;(" << initial_goals[subproblem_goals[j]]->head->getName() << prop_ss.str() << ")" << endl;
            }

            //o << goals_ss.str();
            //o << agent_goals_ss.str();

            tactical_problem << "      (stp_complete_mission)" << endl;

            tactical_problem << "   ))" << endl;

            //add metric
            tactical_problem << "   (:metric minimize (total-time))" << endl;
            tactical_problem << ")" << endl;




            //run optic
//            if(agent_constraints) {
            if(agent_constraints_in_vector(agent_contstraints_vector)){
                if (backup_planner){
                    stats << "Attempting " << agents_structure[0].priority << "-TP " << i << " " << "threshold: " << threshold << " " << endl <<  tactical_domain_name.str() << ".pddl" << endl <<  problem_name.str() << ".pddl" << endl;

                }else{
                    stats << "Attempting " << agents_structure[0].priority << "-TP " << i << " " << "threshold: " << threshold << " " << endl <<  tactical_domain_name.str() << ".pddl" << endl <<  problem_name.str() << ".pddl" << endl;

                }
                for (int j = 0; j < subproblem_goals.size(); ++j){
                    stringstream prop_ss;
                    for (VAL::parameter_symbol_list::iterator git = initial_goals[subproblem_goals[j]]->args->begin();
                         git != initial_goals[subproblem_goals[j]]->args->end(); git++) {
                        VAL::parameter_symbol *parameter_symbol_instance = *git;
                        prop_ss << " " << parameter_symbol_instance->getName();
                    }
                    stats << "   top_level_goal: " << subproblem_goals[j] << " - (" << initial_goals[subproblem_goals[j]]->head->getName() << prop_ss.str() << "))" << endl;
                    //cout << "   top_level_goal: " << subproblem_goals[j] << " - (" << initial_goals[subproblem_goals[j]]->head->getName() << prop_ss.str() << "))" << endl;
                }
            }
            else stats << "Retrying with no constraints " << agents_structure[0].priority << "-TP " << i << " " << endl <<  tactical_domain_name.str() << ".pddl" << endl <<  problem_name.str() << ".pddl" << endl;
            stringstream cmd;
            const auto solve_start = std::chrono::system_clock::now();

            // check if solvable
            //TODO replace with clean reachability analysis method
            if (!backup_planner){
//                stringstream import_problem_name;
//                import_problem_name << problem_name.str()  << ".pddl ";
//                stringstream import_domain_name;
//                import_domain_name << tactical_domain_name.str() << ".pddl ";
//                runCommand("rm -rf import_problem import_domain; cp "+import_problem_name.str()+" import_problem; cp "+import_domain_name.str()+" import_domain");

//
//                importProblemAndDomain("import_problem","import_domain");
//                unsolvable = RPGBuilder::isUnreachable();







                unsolvable = true;
                runCommand("rm -rf solve_log");
                cmd.str("");
                cmd << "ulimit -t 60 ;/home/nq/RALSTP/./optic-cplex -N " << tactical_domain_name.str() << ".pddl " << problem_name.str() << ".pddl" << " >> solve_log";
                runCommand("echo \'"+ cmd.str() + "\' > solve_log");
                std::thread t1(&ProblemGenerator::runCommand, this, cmd.str());
                auto t1_handle = t1.native_handle();
                t1.detach();

                std::time_t checker_start_time = std::time(0);
                bool log_searchable = true;
                while (log_searchable) {
                    ifstream solve_log;
                    solve_log.open("solve_log");
                    if (solve_log){
                        while (getline(solve_log, line)) {
                            if (line.find("1)b", 0) != std::string::npos) {log_searchable = false; unsolvable = false;}
                            if (line.find("2)b", 0) != std::string::npos) {log_searchable = false; unsolvable = false;}
                            if (line.find("3)b", 0) != std::string::npos) {log_searchable = false; unsolvable = false;}
                            if (line.find("4)b", 0) != std::string::npos) {log_searchable = false; unsolvable = false;}
                            if (line.find("5)b", 0) != std::string::npos) {log_searchable = false; unsolvable = false;}
                            if (line.find("6)b", 0) != std::string::npos) {log_searchable = false; unsolvable = false;}
                            if (line.find("7)b", 0) != std::string::npos) {log_searchable = false; unsolvable = false;}
                            if (line.find("8)b", 0) != std::string::npos) {log_searchable = false; unsolvable = false;}
                            if (line.find("9)b", 0) != std::string::npos) {log_searchable = false; unsolvable = false;}
                            if (line.find(";;;; Solution Found", 0) != std::string::npos) {log_searchable = false; unsolvable = false; }
                            if (line.find("; Plan found with metric", 0) != std::string::npos) {log_searchable = false; unsolvable = false; }
                            if (line.find(";; Problem unsolvable!", 0) != std::string::npos) { log_searchable = false; stats << "failed reachability"  << endl;}
                            if (line.find("...cannot be found either in the initial state", 0) != std::string::npos) { log_searchable = false; stats << "failed reachability"  << endl;}

                            std::time_t procedure_current_time = std::time(0);
                            if ((procedure_current_time - checker_start_time) > 1) {log_searchable = false; unsolvable = false; stats << "checker timeout"  << endl;}

                            if (!log_searchable){
                                runCommand("killall -9 optic-cplex");
                                pthread_cancel(t1_handle);
                                break;
                            }
                        }
                    }
                }

//                unsolvable = false;

            }
            if (!unsolvable) {
                if (backup_planner) {

                    cmd << "ulimit -t " << threshold << ";/home/nq/RALSTP/./optic-cplex " << tactical_domain_name.str() << ".pddl "
                        << problem_name.str() << ".pddl" << " > " << problem_name.str() << ".plan";
                    stats << "   Planner: Optic" << endl;


                    std::thread t1(&ProblemGenerator::runCommand, this, cmd.str());
                    auto t1_handle = t1.native_handle();
                    t1.detach();

                    std::time_t planner_start_time = std::time(0);
                    bool plan_found = false;
                    while (!plan_found) {
                        usleep(100000);
                        std::time_t procedure_current_time = std::time(0);
                        if ((procedure_current_time - planner_start_time) > threshold) {
                            runCommand("killall -9 optic-cplex");
                            pthread_cancel(t1_handle);
                            plan_found = true;
                        }
                        ifstream plan;
                        plan.open((problem_name.str() + ".plan").c_str());
                        if (plan) {
                            while (getline(plan, line)) {
                                if (line.find(";; Problem unsolvable!", 0) != std::string::npos) {
                                    runCommand("killall -9 optic-cplex");
                                    pthread_cancel(t1_handle);
                                    plan_found = true;
                                }
                                if (line.find(";;;; Solution Found", 0) != std::string::npos) {
                                    pthread_cancel(t1_handle);
                                    runCommand("killall -9 optic-cplex");
                                    plan_found = true;
                                }
                                if ((line.find("; Plan found with metric", 0) != std::string::npos)) {
                                    usleep(5000000);
                                    pthread_cancel(t1_handle);
                                    runCommand("killall -9 optic-cplex");
                                    plan_found = true;
                                }
                            }
                        }

                    }
                } else {
                    char buffer[PATH_MAX]; //create string buffer to hold path
                    getcwd(buffer, sizeof(buffer));
                    string current_working_dir(buffer);
                    runCommand("rm -rf /home/nq/RALSTP/TPSHE/bin/stp_temp.*");
                    runCommand("rm -rf /home/nq/RALSTP/TPSHE/bin/tmp_stp_temp.*");


                    //create and move tpshe files to tpshe path
                    cmd.str("");
                    cmd
                            << "/home/nq/RALSTP/TPSHE/bin/./plan.py she --time 0 --memory 4000000000000 --plan-file /home/nq/RALSTP/TPSHE/bin/stp_temp "
                            << current_working_dir << "/" << tactical_domain_name.str() << ".pddl "
                            << current_working_dir << "/" << problem_name.str() << ".pddl";
                    runCommand(cmd.str());

                    runCommand("mv dom.pddl /home/nq/RALSTP/TPSHE/bin/");
                    runCommand("mv tdom.pddl /home/nq/RALSTP/TPSHE/bin/");
                    runCommand("mv tins.pddl /home/nq/RALSTP/TPSHE/bin/");




                    runCommand("rm -rf solve_log");
                    cmd.str("");
                    cmd << "/home/nq/RALSTP/TPSHE/bin/./plan.py she --time "
                        << threshold
                        << " --memory 4000000000000 --plan-file /home/nq/RALSTP/TPSHE/bin/stp_temp "
                        << current_working_dir << "/" << tactical_domain_name.str() << ".pddl " << current_working_dir
                        << "/" << problem_name.str() << ".pddl >> solve_log";
                    stats << "     Planner: Tpshe" << endl;


                    std::thread t1(&ProblemGenerator::runCommand, this, cmd.str());
                    auto t1_handle = t1.native_handle();
                    t1.detach();
                    bool parsing_error = false;

                    std::time_t planner_start_time = std::time(0);
                    bool plan_found = false;
                    while (!plan_found && !parsing_error) {
                        usleep(100000);
                        ifstream solve_log;
                        solve_log.open("solve_log");
                        while (getline(solve_log, line)) {
                            if (line.find("No solution to be converted into temporal has been found", 0) != std::string::npos) {
                                //stats << "No solution to be converted into temporal has been found" << endl;
                                pthread_cancel(t1_handle);
                                runCommand("killall -9 python downward");
                                parsing_error = true;
                                break;
                            }
                        }
                        std::time_t procedure_current_time = std::time(0);
                        ifstream plan;
                        plan.open("/home/nq/RALSTP/TPSHE/bin/stp_temp.1");

                        if (plan || ((procedure_current_time - planner_start_time) > threshold)) {
                            usleep(5000000);
                            pthread_cancel(t1_handle);
                            runCommand("killall -9 python downward");
                            plan_found = true;
                        }
                    }
                    if (parsing_error) {
                        stats << "tactical problem unsolvable with Tpshe" << endl;
                        backup_planner = true;
                        i--;
                        continue;
                    }


                    bool parsed_all_plans = false;
                    int plan_no = 1;
                    while (!parsed_all_plans) {
                        ifstream plan;
                        plan.open("/home/nq/RALSTP/TPSHE/bin/stp_temp." +
                                  to_string(plan_no));
                        if (plan) {
                            cmd.str("");
                            cmd
                                    << "/home/nq/RALSTP/TPSHE/bin/./planSchedule /home/nq/RALSTP/TPSHE/bin/tdom.pddl /home/nq/RALSTP/TPSHE/bin/dom.pddl "
                                    << current_working_dir << "/" << problem_name.str()
                                    << ".pddl /home/nq/RALSTP/TPSHE/bin/stp_temp."
                                    << plan_no
                                    << " >> /home/nq/RALSTP/TPSHE/bin/tmp_stp_temp."
                                    << plan_no;
                            runCommand(cmd.str());
                            cmd.str("");

                            bool temporal_plan_ready = false;
                            while (!temporal_plan_ready) {
                                ifstream temporal_plan;
                                temporal_plan.open(
                                        "/home/nq/RALSTP/TPSHE/bin/tmp_stp_temp." +
                                        to_string(plan_no));
                                if (temporal_plan) { temporal_plan_ready = true; }
                                else { usleep(2000000); }
                            }

                            plan_no++;
                        } else { parsed_all_plans = true; }
                    }

                }
            }

            const auto solve_end = std::chrono::system_clock::now();
            tactical_plan_solve_time = solve_end - solve_start;


            //parse plan and clean it
            double makespan = 999999999;
            double plan_time = threshold + 0.0001;
            ifstream plan_file;
            if (backup_planner) {
                plan_file.open((problem_name.str() + ".plan").c_str());
                vector<string> best_plan;
                while (getline(plan_file, line)) {
                    if ((line.find("; Plan found with metric", 0) != std::string::npos)) {
                        best_plan.clear();
                    }
                    if (line.find(";;;; Solution Found", 0) != std::string::npos) {
                        best_plan.clear();
                    }
                    best_plan.push_back(line);
                }
                plan_file.close();
                ofstream temp_plan;
                temp_plan.open(problem_name.str() + ".plan");
                for (int j = 0; j < best_plan.size(); j++){
                    temp_plan << best_plan[j] << endl;
                }
                temp_plan.close();
                plan_file.open((problem_name.str() + ".plan").c_str());
            }
            else {
                cmd.str("");
                cmd << "touch " << problem_name.str() << ".plan";
                runCommand(cmd.str());
                cmd.str("");
                cmd << "cat /home/nq/RALSTP/TPSHE/bin/tmp_stp_temp.* >> " << problem_name.str() << ".plan";
                runCommand(cmd.str());
                runCommand("rm -rf /home/nq/RALSTP/TPSHE/bin/stp_temp.*");
                runCommand("rm -rf /home/nq/RALSTP/TPSHE/bin/tmp_stp_temp.*");

                plan_file.open((problem_name.str() + ".plan").c_str());
                vector<pair<double,string>> actions;
                double max_end_time = 0;
                while (getline(plan_file, line)) {
                    if ((line.find(";;;; init: (action) [duration]", 0) != std::string::npos)){
                        max_end_time = 0;
                        actions.clear();
                        continue;
                    }
                    if (line.find(": (", 0) != std::string::npos){
                        if (line.find(".0") != std::string::npos){
                            std::size_t pos = line.find(".0");
                            //line.replace(pos,2,".");
                        }
                        std::transform(line.begin(), line.end(), line.begin(),
                                       [](unsigned char c){ return std::tolower(c); });
                        pair<double,string> action;
                        action.first = stod(line.substr(0,line.find(":")));
                        action.second = line;
                        actions.push_back(action);
                        double duration = stod(line.substr(line.find("[")+1,line.find("]")));
                        if (max_end_time < (action.first + duration)) { max_end_time = action.first + duration ;}
                    }
                }
                plan_file.close();
                sort(actions.begin(), actions.end());
                stringstream best_plan;
                for (int l = 0; l < actions.size(); l++){
                    best_plan << actions[l].second << endl;
                }
                if (actions.size() > 0 ) {best_plan << "; TPSHE-cost " << std::setprecision(4) << std::fixed << max_end_time << endl;}
                else {
                    //stats << actions.size() << " " << tactical_plan_solve_time.count() << endl;
                    if (tactical_plan_solve_time.count() < threshold) {unsolvable = true;}
                }

//                actions.insert(actions.begin(), best_plan.str());

                ofstream best_plan_file;
                best_plan_file.open(problem_name.str() + ".plan");
                best_plan_file << best_plan.str();
                best_plan_file.close();
                plan_file.open((problem_name.str() + ".plan").c_str());

            }
            ofstream clean_plan;
            clean_plan.open(problem_name.str() + ".plan.clean");
            bool found = false;
            bool empty_plan = false;
            vector<string> used_agents;

            //get problem available agents
            stringstream available_agents, priority_agents, reused_agents;
            for (int j = 0; j < agents_structure.size(); j++){
                if (!agents_structure[j].is_parent) continue;
                for (int k = 0; k < agent_groups[j].getAgents().size(); k++) {
                    if(agent_groups[j].getAgents()[k]->isAvailable()){
                        available_agents << agent_groups[j].getAgents()[k]->getName() << " ";
                    }
                    if(agent_groups[j].getAgents()[k]->isPriority()){
                        priority_agents << agent_groups[j].getAgents()[k]->getName() << " ";
                    }
//                    if(!agent_groups[j].getAgents()[k]->isReused()){
//                        reused_agents << agent_groups[j].getAgents()[k]->getName() << " ";
//                    }
                }
            }



//            //TODO refactor reused agent bugfix - multti node availability (2 layers or based on no of agents?) - now there are no all agent available options constraints solutions
//            // identify agents from plan and remove priority and/or availability
            for (int j = 0; j < agents_structure.size(); j++){
                if (!agents_structure[j].is_parent) continue;
                for (int k = 0; k < agent_groups[j].getAgents().size(); k++) {
                    ifstream plan_file_temp;
                    plan_file_temp.open((problem_name.str() + ".plan").c_str());
                    while (getline(plan_file_temp, line)) {
                        if (line.find(": (", 0) != std::string::npos){
                            if ((line.find(agent_groups[j].getAgents()[k]->getName()+" ", 0) != std::string::npos) || (line.find(agent_groups[j].getAgents()[k]->getName()+")", 0) != std::string::npos)){
                                used_agents.push_back(agent_groups[j].getAgents()[k]->getName());
                                if(agent_groups[j].getAgents()[k]->isPriority()){
                                    agent_groups[j].getAgents()[k]->setPriority(false);
                                    stats << "        priority(" << agent_groups[j].getAgents()[k]->getName() << ") set to \'false\'" << endl;
                                }
                                agent_groups[j].getAgents()[k]->setAvailable(false);
                                stats << "        available(" << agent_groups[j].getAgents()[k]->getName() << ") set to \'false\'" << endl;
//                                if(!agent_constraints){
//                                    agent_groups[j].getAgents()[k]->setReused(true);
//                                    if(agent_groups[j].getRemainingAvailableAgents().size() == agent_groups[j].getRemainingNotReUsedAgents().size())agent_groups[j].makeAllAgentsNotReUsed();
//                                }
                                if(agent_groups[j].getRemainingAvailableAgents().size() == 0){
                                    agent_groups[j].makeAllAgentsAvailable();
                                    stats << "        all available(" << agent_groups[j].getAgents()[k]->getType() << ") constraints have been set to \'true\' " << endl;
                                }
                                break;
                            }
                        }
                    }
                    plan_file_temp.close();
                }
            }

            while (getline(plan_file, line)) {
                //optic -N & popf
                if (line.find("; Plan found with metric", 0) != std::string::npos){
                    makespan = stod(line.substr(24, line.length()-1));
                    found = true;
                }
                if (line.find("; Cost", 0) != std::string::npos){
                    makespan = stod(line.substr(8, line.length()-1));
                    found = true;
                }
                //yahsp3
                if (line.find("; Makespan", 0) != std::string::npos){
                    makespan = stod(line.substr(11, line.length()-1));
                    found = true;
                }
                //tpshe
                if (line.find("; TPSHE-cost ", 0) != std::string::npos){
                    makespan = stod(line.substr(13, line.length()-1));
                    plan_time = tactical_plan_solve_time.count();
                    found = true;
                }
                //optic & popf & yahsp3
                if (line.find("; Time", 0) != std::string::npos){
                    plan_time = stod(line.substr(7, line.length()-1));
                }

                if (line.find(": (", 0) != std::string::npos){
                    clean_plan << line << endl;
                }
                //optic & popf & yahsp3
                if ((line.find("; Cost: 0.000", 0) != std::string::npos) || (line.find("; Makespan: 0\n", 0) != std::string::npos)){
                    empty_plan = true;
                }


                // case RPG of problem not found
                if ((!found) && ((line.find(";; Problem unsolvable!", 0) != std::string::npos) || (line.find("unsolvable.", 0) != std::string::npos))){
//                    if(agent_constraints){
                    unsolvable = true;
                }
            }

            //mitigate constraints
            if(unsolvable){
                if (agent_constraints_in_vector(agent_contstraints_vector)){
                    //reattempt problem with no agent constrainte
                    stats << "   Problem unsolvable with: " << endl;
                    stats << "      Priority agents: " << priority_agents.str() << endl;
                    stats << "      Available agents: " << available_agents.str() << endl;
//                        stats << "      Not-reused agents: " << reused_agents.str() << endl << endl;

                    int parent_agent_groups_index = 0;
                    for (int q = 0; q < agent_groups.size(); q++) {
                        if(!agent_groups[q].has_parent_parent_dynamic_type()) {continue;}
                        if (parent_agent_groups_index == agent_constraints_index){
                            stats << "      Removing constraint for: " << agent_groups[q].getType() << endl << endl;
                            break;
                        }
                        parent_agent_groups_index++;
                    }

                    i--;
//                        agent_constraints = false;            // replace with an agent
                    if(agent_constraints_index == 0){
                        agent_contstraints_vector[0] = false;
                    }
                    if((agent_constraints_index > 0) && (agent_constraints_index < agent_contstraints_vector.size()-1)){
                        agent_contstraints_vector[agent_constraints_index-1] = true;
                        agent_contstraints_vector[agent_constraints_index] = false;
                    }
                    if(agent_constraints_index == agent_contstraints_vector.size()-1){
                        set_all_constraints_in_vector(false);
                    }
                    agent_constraints_index++;
                }
                else{
                    agent_constraints_index++;
                    stats << "      Failed. No of Goals: " << subproblem_goals.size() << endl;
                    stats << "      Priority agents: " << priority_agents.str() << endl;
                    stats << "      Available agents: " << available_agents.str() << endl << endl << endl;;
//                        stats << "      Not-reused agents: " << reused_agents.str() << endl << endl;
                    return new_result;
                }
            }


//            if(found) agent_constraints = true;
            if(found){
                agent_constraints = true;
                set_all_constraints_in_vector(true);
                agent_constraints_index = 0;
            }
            else{
                if((agent_constraints_index <= agent_contstraints_vector.size()) && unsolvable){
                    continue;
                }
            }




            //dedupe used agents
            sort( used_agents.begin(), used_agents.end() );
            used_agents.erase( unique( used_agents.begin(), used_agents.end() ), used_agents.end() );
            stringstream used_agents_ss;
            for (int j = 0; j < used_agents.size(); j++){
                used_agents_ss << used_agents[j] << " ";
            }

            subgoals_set_.addSubgoalTime(plan_time);
            subgoals_set_.addToTotalTime(plan_time);
            subgoals_set_.addSubgoalMakespan(makespan);
            subgoals_set_.addToTotalMakespan(makespan);

            if(!found){
                stats << "   Failed. No of Goals: " << subproblem_goals.size() << " " << "Plan Time: " << plan_time << " Makespan: " << makespan << " Used Agents: " << used_agents_ss.str() << endl << endl;
//
//                if (!backup_planner){
//                    stats << "   Attempting with backup planner..." << endl << endl;
//                    backup_planner = true;
//                    i--;
//                    continue;
//                }

                stats << "Abandoned. Total-Time OVER: " << subgoals_set_.getTotalTime() << " Total-Sequential-Makespan: " << subgoals_set_.getTotalMakespan() << endl << endl << endl << endl;
                backup_planner = only_backup_planner;

                subgoals_plan_time.push_back(plan_time);
                subgoals_makespan.push_back(99999999);

                subgoals_max_size.push_back(subgoals_set_.getMaxSubgoalSize());
                subgoals_no_of_sets.push_back(subgoals_size);
                subgoals_lm_sets.push_back(subgoals_set_.lm_subgoals);
                subgoals_goals_in_lm_subgoals.push_back(subgoals_set_.goals_in_lm_subgoals);
                subgoals_goals_in_all_subgoals.push_back(current_goals_in_all_subgoals);
                subgoals_decomposition.push_back(current_decomposition);
                new_result.plan_time = plan_time;
                return new_result;
            }

            if(!empty_plan ){
//                agent_constraints = true;
                agent_constraints_index = 0;
                set_all_constraints_in_vector(true);
                usefull_subgoals[i] = true;

                /*
                 * create macro action
                 */
                cmd.str("");
                ifstream clean_plan_file;
                clean_plan_file.open((problem_name.str() + ".plan.clean").c_str());
                string epsilon = "0.001";
                while (getline(clean_plan_file, line)) {
                    size_t pos = line.find(":");
                    size_t pos2 = line.find(".");
                    int precision = static_cast<int>(pos) - static_cast<int>(pos2);
                    cout << " eeepsilon 0 " << pos << " " << pos2 << " " << precision << endl;
                    if (precision == 5){epsilon = "0.0001";}
                }
                stats << "epsilon " << epsilon << endl;
                cmd << tactical_domain_name.str() << ".pddl " << problem_name.str() << ".pddl " << problem_name.str() << ".plan.clean";
                vector<string> validation_output = runCommandReturnOutput("/home/nq/RALSTP/VAL_latest/build/linux64/release/bin/./Validate -v -t " + epsilon + " " + cmd.str());
                map<string,bool> happening_outcomes;
                for (int l = 0; l < validation_output.size(); l++){
                    if (line.find("Failed plans:", 0) != std::string::npos){
                        stats << "Invalid Tactical Plan" << endl;
                        exit(0);
                    }
                    string line = validation_output[l];
                    if (line.find("Adding (", 0) != std::string::npos){
                        line = line.substr(7,line.length()-1);
                        happening_outcomes[line] = true;
                    }
                    if (line.find("Deleting (", 0) != std::string::npos){
                        line = line.substr(9,line.length()-1);
                        happening_outcomes[line] = false;
                    }
                }
                stringstream macro_action_conditions;
                stringstream macro_action_effects;
                stringstream tests;
                vector<string> stp_free_lits;
                for (map<string,bool>::iterator mit = happening_outcomes.begin(); mit != happening_outcomes.end(); mit++ ){
                    string current_lit = (*mit).first;
                    current_lit = current_lit.substr(0,current_lit.length()-1);
                    if(current_lit.find("stp_") != string::npos){continue;}
                    VAL::proposition* current_lit_prop = stringToProposition(current_lit);
                    if(isDeadEndAgentProposition(current_lit_prop)){continue;}
                    bool is_true = (*mit).second;
                    bool is_initial_state_fact = false;
                    bool is_function = false;
                    bool is_initial_state_func = false;

                    //stp_free
                    for (int l = 0; l < agent_groups.size(); l++) {
                        for(int k = 0; k < agent_groups[l].size(); k++){
                            if (!agent_groups[l].has_parent_parent_dynamic_type()){continue;}
                            string type = agent_groups[l].getType();
                            string agent = agent_groups[l].getAgents()[k]->getName();
                            string search_lit = current_lit + " ";
                            if((search_lit.find((agent+" "), 0) != std::string::npos) || (search_lit.find((agent+")"), 0) != std::string::npos)){
                                stp_free_lits.push_back("stp_free_"+type+" "+agent);
                            }
                        }
                    }


                    //positive effects
                    if(is_true){
                        macro_action_effects << "        (at end "  << current_lit << ")" << endl;
                    }

                    //conditions and negative effects
                    for (int l = 0; l < facts.size(); l++){
                        string fact = propositionToString(facts[l]);
                        fact.erase(std::remove(fact.begin(), fact.end(), '?'), fact.end());
                        if ((current_lit.compare(fact) == 0)){
                            macro_action_conditions << "        (at start " << current_lit << ")" << endl;
                        }
                        if ((current_lit.compare(fact) == 0) && !is_true){
                            macro_action_effects << "        (at end (not " << current_lit << "))" << endl;
                        }
                        if ((current_lit.compare(fact)) == 0){
                            is_initial_state_fact = true;
                        }
                    }
                    tests << facts.size() << " " << func_name.size() << endl;
                    for (int l = 0; l < func_name.size(); l++) {
                        stringstream func;
                        func << "(" << operator_[l] << " (" << func_name[l];
                        for (VAL::parameter_symbol_list::iterator fit = func_args[l]->begin();
                             fit != func_args[l]->end(); fit++) {
                            VAL::parameter_symbol *parameter_symbol_instance = *fit;
                            func << " " << parameter_symbol_instance->getName();
                        }
                        func << ") " << values[l] << ")" << endl;
                        tests << " " << func.str();
                        if ((current_lit.compare(func.str()) == 0) && !is_true){
                            is_function = true;
                            macro_action_conditions << "        (at start " << current_lit << ")" << endl;
                            macro_action_effects << "        (at end (not " << current_lit << "))" << endl;
                        }
                        if (current_lit.compare(func.str()) == 0){
                            is_initial_state_func = true;
                        }
                    }

                }
                sort( stp_free_lits.begin(), stp_free_lits.end() );
                stp_free_lits.erase( unique( stp_free_lits.begin(), stp_free_lits.end() ), stp_free_lits.end() );
                for (int l = 0; l < stp_free_lits.size(); l++) {
                    macro_action_conditions << "        (at start (" << stp_free_lits[l] << "))" << endl;
                    macro_action_effects << "        (at start (not (" << stp_free_lits[l] << ")))" << endl << "        (at end (" << stp_free_lits[l] << "))" << endl;
                }
                strategic_domain << "(:durative-action " << t_corrected_name << endl;
                //strategic_domain << tests.str() << endl;
                strategic_domain << "    :parameters ()" << endl;
                strategic_domain << "    :duration (= ?duration " << makespan << ")" << endl;
                strategic_domain << "    :condition (and" << endl << macro_action_conditions.str() << ")" << endl;
                strategic_domain << "    :effect (and" << endl << macro_action_effects.str();
                strategic_domain << "        (at end (stp_complete_mission mission" << i << "))" << endl <<  "    )" << endl << ")" << endl << endl;



            }
            solved_goals_old = solved_goals;
            tactical_problem_names.push_back(t_corrected_name);
            backup_planner = only_backup_planner;
            stats << "   Solved. No of Goals " << subproblem_goals.size() << " " << "Plan Time: " << plan_time << " Makespan: " << makespan << " Used Agents: " << used_agents_ss.str() << endl << endl;
            if(new_result.max_planning_time < plan_time)new_result.max_planning_time = plan_time;
        }

        strategic_domain << endl << ")" << endl;

        //append to stats file
        stats << "Total-Time: " << subgoals_set_.getTotalTime() << " Total-Sequential-Makespan: " << subgoals_set_.getTotalMakespan() << " Usefull Subgoals: " << usefull_subgoals.size() << " Average Makespan: " << subgoals_set_.getTotalMakespan() / subgoals_size << endl << endl;


        //for the random test
//        string isSmaller = " no";
//        if(subgoals_set_.getTotalMakespan() < 1383)isSmaller = " YES";
//        stats << isSmaller;






        //TODO own method////////////////////////////
        /*
         * run domain against strategic problem
         */
        stringstream strategic_problem_name;
        strategic_problem_name << node << "context_" << current_decomposition << "_strategic_problem_" << problem->name;
        ofstream strategic_problem_targeted;
        strategic_problem_targeted.open((strategic_problem_name.str()+".pddl"));
        string s_corrected_name = strategic_problem_name.str();
        replace(s_corrected_name.begin(), s_corrected_name.end(), '/', '_');
        strategic_problem_targeted << "(define (problem " << s_corrected_name << ") (:domain " << domain->name << ")\n(:objects)" << endl;

        //add facts
        strategic_problem_targeted << "   (:init" << endl;
        for (int i = 0; i < facts.size(); i++) {
            if(isDeadEndAgentProposition(facts[i])){continue;}
            strategic_problem_targeted << "      (" << facts[i]->head->getName();
            for (VAL::parameter_symbol_list::iterator pit = facts[i]->args->begin();
                 pit != facts[i]->args->end(); pit++) {
                VAL::parameter_symbol *parameter_symbol_instance = *pit;
                strategic_problem_targeted << " " << parameter_symbol_instance->getName();
            }
            strategic_problem_targeted << ")" << endl;
        }

        //add functions
        for (int j = 0; j < func_name.size(); j++) {
            //TODO check if dead-end agent function and do not allow
            strategic_problem_targeted << "      (" << operator_[j] << " (" << func_name[j];
            for (VAL::parameter_symbol_list::iterator fit = func_args[j]->begin();
                 fit != func_args[j]->end(); fit++) {
                VAL::parameter_symbol *parameter_symbol_instance = *fit;
                strategic_problem_targeted << " " << parameter_symbol_instance->getName();
            }
            strategic_problem_targeted << ") " << values[j] << ")" << endl;
        }

        //add stp parent agent facts
        for (int j = 0; j < agent_groups.size(); j++) {
            if(!agent_groups[j].has_parent_parent_dynamic_type()) continue;
            for (int k = 0; k < agent_groups[j].size(); k++) {
                strategic_problem_targeted << "    (stp_free_"<< agent_groups[j].getAgents()[k]->getType() << " " << agent_groups[j].getAgents()[k]->getName() << ")" << endl;
            }
        }
        strategic_problem_targeted << "   )" << endl;

//
//        while (getline(strategic_problem_mold, line)) {
//            strategic_problem_targeted << line << endl;
//        }

        strategic_problem_targeted << "   (:goal (and" << endl;
        for(int i = 0; i < usefull_subgoals.size() ; i++){
            if(usefull_subgoals[i]) strategic_problem_targeted << "    (stp_complete_mission mission" << i <<")\n";
        }
        strategic_problem_targeted << "   ))\n    (:metric minimize (total-time))\n)" << endl;
        stringstream plan_file_name;
        plan_file_name << node << "context_" << current_decomposition << "_strategic_plan_" << random << problem->name;





        std::time_t procedure_current_time = std::time(0);
        int time_since_start = procedure_current_time - procedure_start_time;
        if ((max_time - time_since_start) < 0) {
            stats << "\"Strategic non-agent problem out of time. Total-Time OVER: " << max_time << endl;
            return new_result;
        }
        else{
//            // check if solvable
//            const auto solve_start = std::chrono::system_clock::now();
//            unsolvable = false;
//            runCommand("rm -rf solve_log");
//            stringstream cmd;
//            cmd.str("");
//            cmd << "ulimit -t 1 ;/home/nq/RALSTP/./optic-cplex " << strategic_domain_name.str() << " " << strategic_problem_name.str() << ".pddl" << " > solve_log";
//            runCommand(cmd.str());
//            cmd.str("");
//            ifstream solve_log;
//            solve_log.open("solve_log");
//            while (getline(solve_log, line)) {
//                if (line.find("...cannot be found either in the initial state", 0) != std::string::npos) {
//                    unsolvable = true;
//                }
//            }
            const auto solve_start = std::chrono::system_clock::now();
                stats << "     Total Time left: "
                      << (max_time - time_since_start) << endl << endl;
                int threshold = 120;
                char buffer[PATH_MAX]; //create string buffer to hold path
                getcwd(buffer, sizeof(buffer));
                string current_working_dir(buffer);
                runCommand("rm -rf /home/nq/RALSTP/TPSHE/bin/stp_temp.*");
                runCommand("rm -rf /home/nq/RALSTP/TPSHE/bin/tmp_stp_temp.*");

                //create and move tpshe files to tpshe path
                stringstream cmd;
                cmd
                        << "/home/nq/RALSTP/TPSHE/bin/./plan.py she --time 0 --memory 4000000000000 --plan-file /home/nq/RALSTP/TPSHE/bin/stp_temp "
                        << current_working_dir << "/" << strategic_domain_name.str() << " "
                        << current_working_dir << "/" << strategic_problem_name.str() << ".pddl";
                runCommand(cmd.str());
                runCommand("mv dom.pddl /home/nq/RALSTP/TPSHE/bin/");
                runCommand("mv tdom.pddl /home/nq/RALSTP/TPSHE/bin/");
                runCommand("mv tins.pddl /home/nq/RALSTP/TPSHE/bin/");

                runCommand("rm -rf solve_log");
                cmd.str("");
                cmd << "/home/nq/RALSTP/TPSHE/bin/./plan.py she --time "
                    << threshold
                    << " --memory 4000000000000 --plan-file /home/nq/RALSTP/TPSHE/bin/stp_temp "
                    << current_working_dir << "/" << strategic_domain_name.str() << " " << current_working_dir
                    << "/" << strategic_problem_name.str() << ".pddl >> solve_log";
                stats << "     Planner: Tpshe" << endl;

                stats << cmd.str() << endl;
                std::thread t1(&ProblemGenerator::runCommand, this, cmd.str());
                auto t1_handle = t1.native_handle();
                t1.detach();
                bool parsing_error = false;

                std::time_t planner_start_time = std::time(0);
                bool plan_found = false;
                while (!plan_found && !parsing_error) {
                    usleep(100000);
                    ifstream solve_log;
                    solve_log.open("solve_log");
                    while (getline(solve_log, line)) {

                        if (line.find("No solution to be converted into temporal has been found", 0) !=
                            std::string::npos) {
                            parsing_error = true;
                            pthread_cancel(t1_handle);
                            runCommand("killall -9 python downward");
                            break;
                        }
                    }
                    std::time_t procedure_current_time = std::time(0);
                    ifstream plan;
                    plan.open("/home/nq/RALSTP/TPSHE/bin/stp_temp.1");

                    if (plan || ((procedure_current_time - planner_start_time) > threshold)) {
                        usleep(5000000);
                        pthread_cancel(t1_handle);
                        runCommand("killall -9 python downward");
                        plan_found = true;
                    }
                }

                bool parsed_all_plans = false;
                int plan_no = 1;
                while (!parsed_all_plans && !parsing_error) {
                    ifstream plan;
                    plan.open("/home/nq/RALSTP/TPSHE/bin/stp_temp." +
                              to_string(plan_no));
                    if (plan) {
                        cmd.str("");
                        cmd
                                << "/home/nq/RALSTP/TPSHE/bin/./planSchedule /home/nq/RALSTP/TPSHE/bin/tdom.pddl /home/nq/RALSTP/TPSHE/bin/dom.pddl "
                                << current_working_dir << "/" << strategic_problem_name.str()
                                << ".pddl /home/nq/RALSTP/TPSHE/bin/stp_temp."
                                << plan_no
                                << " >> /home/nq/RALSTP/TPSHE/bin/tmp_stp_temp."
                                << plan_no;
                        runCommand(cmd.str());
                        cmd.str("");

                        bool temporal_plan_ready = false;
                        while (!temporal_plan_ready) {
                            ifstream temporal_plan;
                            temporal_plan.open(
                                    "/home/nq/RALSTP/TPSHE/bin/tmp_stp_temp." +
                                    to_string(plan_no));
                            if (temporal_plan) { temporal_plan_ready = true; }
                            else { usleep(2000000); }
                        }

                        plan_no++;
                    } else { parsed_all_plans = true; }
                }
                const auto solve_end = std::chrono::system_clock::now();
                strategic_plan_solve_time = solve_end - solve_start;
                if (parsing_error){
                    stats << "strategic problem unsolvable with Tpshe" << endl;
                    stats << "     Planner: Optic" << endl;
                    stringstream cmd;
                    cmd << "ulimit -t " << threshold << ";/home/nq/RALSTP/./optic-cplex " << strategic_domain_name.str() << " "
                        << strategic_problem_name.str() << ".pddl > " << plan_file_name.str() << ".plan";

                    std::thread t1(&ProblemGenerator::runCommand, this, cmd.str());
                    auto t1_handle = t1.native_handle();
                    t1.detach();

                    std::time_t planner_start_time = std::time(0);
                    bool plan_found = false;
                    while (!plan_found) {
                        usleep(100000);
                        std::time_t procedure_current_time = std::time(0);
                        if ((procedure_current_time - planner_start_time) > threshold) {
                            runCommand("killall -9 optic-cplex");
                            pthread_cancel(t1_handle);
                            plan_found = true;
                        }
                        ifstream plan;
                        plan.open(plan_file_name.str() + ".plan");
                        if (plan) {
                            while (getline(plan, line)) {
                                if (line.find(";;;; Solution Found", 0) != std::string::npos) {
                                    pthread_cancel(t1_handle);
                                    runCommand("killall -9 optic-cplex");
                                    plan_found = true;
                                }
                                if ((line.find("; Plan found with metric", 0) != std::string::npos)) {
                                    usleep(5000000);
                                    pthread_cancel(t1_handle);
                                    runCommand("killall -9 optic-cplex");
                                    plan_found = true;
                                }
                            }
                        }
                    }
                    ifstream plan_file;
                    plan_file.open(plan_file_name.str()+".plan");
                    vector<string> best_plan;
                    while (getline(plan_file, line)) {
                        if ((line.find("; Plan found with metric", 0) != std::string::npos)) {
                            best_plan.clear();
                        }
                        if (line.find(";;;; Solution Found", 0) != std::string::npos) {
                            best_plan.clear();
                        }
                        best_plan.push_back(line);
                    }
                    plan_file.close();
                    ofstream temp_plan;
                    temp_plan.open(plan_file_name.str()+".plan");
                    for (int j = 0; j < best_plan.size(); j++){
                        temp_plan << best_plan[j] << endl;
                    }
                    temp_plan.close();
                    plan_file.close();
                }
                else{
                    cmd.str("");
                    cmd << "touch " << plan_file_name.str() << ".plan";
                    runCommand(cmd.str());
                    cmd.str("");
                    cmd << "cat /home/nq/RALSTP/TPSHE/bin/tmp_stp_temp.* >> " << plan_file_name.str() << ".plan";
                    runCommand(cmd.str());
                    runCommand("rm -rf /home/nq/RALSTP/TPSHE/bin/stp_temp.*");
                    runCommand("rm -rf /home/nq/RALSTP/TPSHE/bin/tmp_stp_temp.*");
                    ifstream plan_file;
                    plan_file.open((plan_file_name.str() + ".plan").c_str());
                    vector<pair<double,string>> actions;
                    double max_end_time = 0;
                    while (getline(plan_file, line)) {
                        if ((line.find(";;;; init: (action) [duration]", 0) != std::string::npos)){
                            max_end_time = 0;
                            actions.clear();
                            continue;
                        }
                        if (line.find(": (", 0) != std::string::npos){
                            if (line.find(".0") != std::string::npos){
                                std::size_t pos = line.find(".0");
                                //line.replace(pos,2,".");
                            }
                            std::transform(line.begin(), line.end(), line.begin(),
                                           [](unsigned char c){ return std::tolower(c); });
                            pair<double,string> action;
                            action.first = stod(line.substr(0,line.find(":")));
                            action.second = line;
                            actions.push_back(action);
                            double duration = stod(line.substr(line.find("[")+1,line.find("]")));
                            if (max_end_time < (action.first + duration)) { max_end_time = action.first + duration ;}
                        }
                    }
                    plan_file.close();
                    sort(actions.begin(), actions.end());
                    stringstream best_plan;
                    for (int l = 0; l < actions.size(); l++){
                        best_plan << actions[l].second << endl;
                    }
                    if (actions.size() > 0 ) {best_plan << "; TPSHE-cost " << std::setprecision(4) << std::fixed << max_end_time << endl;}
                    else {
                        //stats << actions.size() << " " << strategic_plan_solve_time.count() << endl;
                        if (strategic_plan_solve_time.count() < threshold) {unsolvable = true;}
                    }

//                actions.insert(actions.begin(), best_plan.str());

                    ofstream best_plan_file;
                    best_plan_file.open(plan_file_name.str() + ".plan");
                    best_plan_file << best_plan.str();
                    best_plan_file.close();
                }

        }




        // end of solving strategic problem////////////////



        //TODO clean repetitive code
        /*
        * parse plan and create priority agent goals problem
        */
        ifstream plan_file;
        plan_file.open(plan_file_name.str()+".plan");
        string all_dead_end_agent_goals_strategic_plan = plan_file_name.str() + ".plan.clean";
        ofstream clean_plan;
        clean_plan.open(all_dead_end_agent_goals_strategic_plan);
        bool found = false;
        double makespan = 9999999;
        double plan_time = 9999999;
        while (getline(plan_file, line)) {
            //tpshe
            if (line.find("; TPSHE-cost ", 0) != std::string::npos){
                makespan = stod(line.substr(13, line.length()-1));
                plan_time = tactical_plan_solve_time.count();
                found = true;
            }
            if (line.find("; Plan found with metric", 0) != std::string::npos){
                makespan = stod(line.substr(24, line.length()-1));
                found = true;
            }
            if (line.find("; Cost", 0) != std::string::npos){
                makespan = pre_makespan+stod(line.substr(8, line.length()-1));
                found = true;
            }
            if (line.find("; Time", 0) != std::string::npos){
                plan_time = stod(line.substr(7, line.length()-1));
            }
            if (line.find(": (", 0) != std::string::npos){
                clean_plan << line << endl;
            }
        }
        if(!found){
            stats << node << " Dead-end Agent Goals Problem NOT solved" << endl << endl << endl << endl;
            return new_result;
        }

        stats << node << endl << " Concurrent Makespan without parent agent goals: " << makespan << " Planning Time without parent agent goals: " << pre_plan_time+subgoals_set_.getTotalTime()+plan_time << endl << endl;
//        double all_dead_end_agent_goals_plan_makespan = makespan;
//        stats << "missing agent at end" << endl << endl;
//        stats << "agent enforced at end" << endl << endl;
        //return new_result;

//        cmd.str("");cmd << "/home/nq/stp/temporal-landmarks/release/optic/./optic-clp " << original_domain_file_name << " " << allagents/p1.pddl -j -u";
//        stats << endl << cmd.str() << endl;
//        runCommand(cmd.str());

        //check if any goals left
        stats << " Solved goals: " << solved_goals.size() << " Remaining goals: " << (goals.size()-solved_goals.size()) << endl << endl;
        ProblemGenerator::parent_agents_result parent_agents_result;

        string validation_result = create_all_dead_end_agent_goals_plan_from_strategic_plan(all_dead_end_agent_goals_strategic_plan, node);
        if(validation_result.compare("Plan valid") == 0) {
            if(goals.size() == solved_goals.size()){
                stats << "All Dead End Agent Goals Plan is Also Parent Agent Goals Plan and is Valid" << endl << endl;
                new_result.makespan = makespan;
                new_result.max_planning_time = pre_plan_time+subgoals_set_.getTotalTime()+plan_time;
                new_result.plan_file_name = all_dead_end_agent_goals_strategic_plan+".purley_tactical";
                new_result.valid = true;
                return new_result;
            }
            else{
                stats << "All Dead End Agent Goals Plan is Valid" << endl << endl;
                parent_agents_result = generateParentAgentGoalsProblem(plan_file_name.str(), to_string(current_decomposition), stats, current_decomposition, node);
                new_result.agent_plan_time = parent_agents_result.plan_time;
                new_result.makespan = makespan + parent_agents_result.makespan;
            }

        }else{
            stats << node << " All Dead End Agent Goals Plan NOT valid 2" << endl;
            stats << validation_result << endl << endl;
            return new_result;
        }
        //stp

        // get all parent agent goals plan

        //create initial planning problem plan from all dead end agent goals plan and all parent agent goals plan and check if it is valid
        validation_result = create_initial_problem_plan(all_dead_end_agent_goals_strategic_plan+".purley_tactical",parent_agents_result.plan_file_name,makespan, node);
        if(validation_result.compare("Plan valid") != 0) {
            stats << node << " Initial Plannig Problem Plan NOT valid - STP" << endl;
            stats << validation_result << endl << endl << endl << endl;
            return new_result;
        }
        else { stats << "Initial Plannig Problem Plan Valid" << endl << endl; }

        stats << "Final Concurrent Makespan: "  << new_result.makespan << " Total Planning Time: " << new_result.agent_plan_time + new_result.plan_time << " Plan File Name: " << all_dead_end_agent_goals_strategic_plan << ".purley_tactical.initial" << endl << endl << endl << endl;

        subgoals_plan_time.push_back(subgoals_set_.getTotalTime()+new_result.agent_plan_time);
        subgoals_makespan.push_back(new_result.makespan);

        subgoals_max_size.push_back(subgoals_set_.getMaxSubgoalSize());
        subgoals_no_of_sets.push_back(subgoals_size);
        subgoals_lm_sets.push_back(subgoals_set_.lm_subgoals);
        subgoals_goals_in_lm_subgoals.push_back(subgoals_set_.goals_in_lm_subgoals);
        subgoals_goals_in_all_subgoals.push_back(current_goals_in_all_subgoals);
        subgoals_decomposition.push_back(current_decomposition);

        new_result.plan_time = subgoals_set_.getTotalTime() + new_result.agent_plan_time;
        new_result.plan_file_name = all_dead_end_agent_goals_strategic_plan+".purley_tactical.initial";
        new_result.valid = true;
        return new_result;

        //}

        //TODO highest priority parent agents goals become new dead-end agents which mark new dead-end agent goals
//        else{
//
//            stats << "Generating next STP layer..." << endl << endl;
//
//            //return pair<string,double> orig_problem_name (new initial state + remaining goals), start duration
//            cmd.str("");
//            cmd << "/home/nq/stp/temporal-landmarks/release/optic/./optic-clp -j -u -V" << makespan << " -K" <<macro_file_name.str() << ".pddl" << agents_structure[0].priority << " ";
//
//            for (int j = 0 ; j < agents_structure.size(); j++){
//                if (!agents_structure[j].is_parent) continue;
//                cmd << "-Y" << agents_structure[j].first << agents_structure[j].type << " ";
//
//            }
//            cmd << original_domain_file_name << " " << macro_file_name.str() << "_lm.pddl" << agents_structure[0].priority;
//
//            // stats << cmd.str() << endl;
//            runCommand(cmd.str());
//
//            /*
//             * /home/nq/stp/temporal-landmarks/release/optic/./optic-clp -j -u -Kdlog-6-6-13.pddl " << original_domain_file_name << " " << allagents/p2.pddl
//             */
//
//        }        else{
//
//            stats << "Generating next STP layer..." << endl << endl;
//
//            //return pair<string,double> orig_problem_name (new initial state + remaining goals), start duration
//            cmd.str("");
//            cmd << "/home/nq/stp/temporal-landmarks/release/optic/./optic-clp -j -u -V" << makespan << " -K" <<macro_file_name.str() << ".pddl" << agents_structure[0].priority << " ";
//
//            for (int j = 0 ; j < agents_structure.size(); j++){
//                if (!agents_structure[j].is_parent) continue;
//                cmd << "-Y" << agents_structure[j].first << agents_structure[j].type << " ";
//
//            }
//            cmd << original_domain_file_name << " " << macro_file_name.str() << "_lm.pddl" << agents_structure[0].priority;
//
//            // stats << cmd.str() << endl;
//            runCommand(cmd.str());
//
//
//        }

    }

//    void ProblemGenerator::generatePDDLProblem(ofstream o, string problem_name, string domain_name, vecto<string> objects, vecto<string> init_state, vecto<string> goal_state){
//
//    }
//
//    void ProblemGenerator::generatePDDLDomain(ofstream o, string domain_name, vecto<string> requirements, vecto<string> types, vecto<string> constants, vecto<string> actions){
//
//    }
    void ProblemGenerator::runCommand(string cmd){
        string data;
        FILE *stream;
        char buffer[1000];
        stream = popen(cmd.c_str(), "r");
        while ( fgets(buffer, 1000, stream) != NULL )
            data.append(buffer);
        pclose(stream);
    }

    vector<string> ProblemGenerator::runCommandReturnOutput(string cmd){
        vector<string> data;
        FILE *stream;
        char buffer[1000];
        stream = popen(cmd.c_str(), "r");
        while ( fgets(buffer, 1000, stream) != NULL )
            data.push_back(buffer);
        pclose(stream);
        return data;
    }

    void ProblemGenerator::runCommandDetectOutput(string cmd, string target, std::promise<bool>& p){
        string data;
        FILE *stream;
        char buffer[1000];
        stream = popen(cmd.c_str(), "r");
        while ( fgets(buffer, 1000, stream) != NULL ){
            data.append(buffer);
            if (data.find(target, 0) != std::string::npos){
//                cout << "!!!!!!!!!!!!!!! output found !!!!!!!!!!!" << endl;
                 pclose(stream);
                p.set_value(true);
                break;
            }
        }
    }

    string ProblemGenerator::generateActiveAgentsProblem(string node, bool execute_problem, bool all_dead_end_agent_goals_problem, string original_problem_file_name){

        string problem_name = original_problem_file_name.substr(0,original_problem_file_name.size()-5);
        if (all_dead_end_agent_goals_problem) { problem_name = node+"all_dead_end_agent_goals_"+problem_name; }
        else { problem_name = node+"active_agents_problem_"+problem_name; }

        ofstream active_agents_problem;
        active_agents_problem.open(problem_name+".pddl");

        active_agents_problem << "(define (problem " << original_problem_file_name.substr(0,original_problem_file_name.size()-5) << ")" << endl;
        active_agents_problem << "   (:domain " << problem->domain_name << ")" << endl;

        //add instances
        active_agents_problem << "   (:objects" << endl;
        for (VAL::const_symbol_list::iterator sit = problem->objects->begin(); sit != problem->objects->end(); sit++){
            VAL::const_symbol* const_symbol_instance = *sit;
            active_agents_problem << "    " << const_symbol_instance->getName() << " - " << getObjectType(const_symbol_instance->getName())->getName() << endl;
        }
        active_agents_problem << "   )" << endl;

        //add initial state facts and functions
        active_agents_problem << "   (:init" << endl;
        active_agents_problem << "   ;" << facts.size() << endl;
        for (int j = 0; j < facts.size(); j++) {
            VAL::proposition* fact = facts[j];
            stringstream prop_ss;
            for (VAL::parameter_symbol_list::iterator pit = fact->args->begin();
                 pit != fact->args->end(); pit++) {
                VAL::parameter_symbol *parameter_symbol_instance = *pit;
                prop_ss << " " << parameter_symbol_instance->getName();
            }
            active_agents_problem << "      (" << fact->head->getName() << prop_ss.str() << ")" << endl;
        }
        for (int j = 0; j < func_name.size(); j++) {
            active_agents_problem << "      (" << operator_[j] << " (" << func_name[j];
            for (VAL::parameter_symbol_list::iterator fit = func_args[j]->begin();
                 fit != func_args[j]->end(); fit++) {
                VAL::parameter_symbol *parameter_symbol_instance = *fit;
                active_agents_problem << " " << parameter_symbol_instance->getName();
            }
            active_agents_problem << ") " << values[j] << ")" << endl;
        }
        active_agents_problem << "   )" << endl;

        //add dead end agent goals
        active_agents_problem << "   (:goal (and" << endl;
        for (int j = 0; j < goals.size(); j++) {
            VAL::proposition* goal = goals[j];
            if(isParentAgentProposition(goal) && all_dead_end_agent_goals_problem) { continue; }
            stringstream prop_ss;
            for (VAL::parameter_symbol_list::iterator pit = goal->args->begin();
                 pit != goal->args->end(); pit++) {
                VAL::parameter_symbol *parameter_symbol_instance = *pit;
                prop_ss << " " << parameter_symbol_instance->getName();
            }
            active_agents_problem << "      (" << goal->head->getName() << prop_ss.str() << ")" << endl;
        }
        active_agents_problem << "   ))" << endl;

        //add metric
        active_agents_problem << "   (:metric minimize (total-time))" << endl;
        active_agents_problem << ")" << endl;

        active_agents_problem.close();

        return problem_name+".pddl";

    }


//    string ProblemGenerator::generateActiveAgentsProblem(string node, bool execute_problem, bool all_dead_end_agent_goals_problem, string original_problem_file_name){
//
//        //mark goals not solved in the initial state
//        cout << "unsolved goals start" << endl;
//        vector<VAL::proposition*> unsolved_goals;
//        for (int i = 0; i < goals.size(); i++) {
//            VAL::proposition* goal = goals[i];
//
//            bool is_solved_goal = false;
//            for (int j = 0; j < facts.size(); j++) {
//                VAL::proposition* fact = facts[j];
//                if (isSameProposition(goal,fact) && isDeadEndAgentProposition(goal)) { is_solved_goal = true; break; }
//            }
//            if(!is_solved_goal) {
//                unsolved_goals.push_back(goal);
//            }
//        }
//        //goals = unsolved_goals;
//
//        cout << "unsolved goals passed" << endl;
//
//        string problem_name = original_problem_file_name.substr(0,original_problem_file_name.size()-5);
//        if (all_dead_end_agent_goals_problem) { problem_name = node+"all_dead_end_agent_goals_"+problem_name; }
//        else { problem_name = node+"active_agents_problem_"+problem_name; }
//
//        ofstream active_agents_problem;
//        active_agents_problem.open(problem_name+".pddl");
//
//        active_agents_problem << "(define (problem " << original_problem_file_name.substr(0,original_problem_file_name.size()-5) << ")" << endl;
//        active_agents_problem << "   (:domain " << problem->domain_name << ")" << endl;
//
//        //add instances
//        active_agents_problem << "   (:objects" << endl;
//        for (VAL::const_symbol_list::iterator sit = problem->objects->begin(); sit != problem->objects->end(); sit++){
//            VAL::const_symbol* const_symbol_instance = *sit;
//            active_agents_problem << "    " << const_symbol_instance->getName() << " - " << getObjectType(const_symbol_instance->getName())->getName() << endl;
//        }
//        active_agents_problem << "   )" << endl;
//
//        //add initial state facts and functions
//        active_agents_problem << "   (:init" << endl;
//        active_agents_problem << "   ;" << facts.size() << endl;
//        for (int j = 0; j < facts.size(); j++) {
//            bool agent_in_goal_state = false;
//            bool dead_end_agent_fact = false;
//            VAL::proposition* fact = facts[j];
//            stringstream prop_ss;
//            for (VAL::parameter_symbol_list::iterator pit = fact->args->begin();
//                 pit != fact->args->end(); pit++) {
//                VAL::parameter_symbol *parameter_symbol_instance = *pit;
//                prop_ss << " " << parameter_symbol_instance->getName();
//
//                // mark all inactive dead-end agent facts
//                if(isDeadEndAgent(parameter_symbol_instance->getName())){
//                    dead_end_agent_fact = true;
//                    bool common_parameter = false;
//                    for (int q = 0; q < unsolved_goals.size(); q++) {
//                        VAL::proposition* goal = unsolved_goals[q];
//                        if(isDeadEndAgentProposition(goal)){
//                            stringstream prop_ss;
//                            for (VAL::parameter_symbol_list::iterator pit = goal->args->begin();
//                                 pit != goal->args->end(); pit++) {
//                                VAL::parameter_symbol *parameter_symbol_instance2 = *pit;
//                                if (parameter_symbol_instance2->getName().compare(parameter_symbol_instance->getName()) == 0) { common_parameter = true; break; }
//                            }
//                        }
//                    }
//                    if (common_parameter) { agent_in_goal_state = true;}
//                }
//            }
//            if(dead_end_agent_fact){
//                if(agent_in_goal_state) { active_agents_problem << "      (" << fact->head->getName() << prop_ss.str() << ")" << endl; }
//            }
//            else { active_agents_problem << "      (" << fact->head->getName() << prop_ss.str() << ")" << endl; }
//        }
//        //TODO check if removal needed
//        for (int j = 0; j < func_name.size(); j++) {
//            active_agents_problem << "      (" << operator_[j] << " (" << func_name[j];
//            for (VAL::parameter_symbol_list::iterator fit = func_args[j]->begin();
//                 fit != func_args[j]->end(); fit++) {
//                VAL::parameter_symbol *parameter_symbol_instance = *fit;
//                active_agents_problem << " " << parameter_symbol_instance->getName();
//            }
//            active_agents_problem << ") " << values[j] << ")" << endl;
//        }
//        active_agents_problem << "   )" << endl;
//
//
//        //add dead end agent goals
//        active_agents_problem << "   (:goal (and" << endl;
//        for (int j = 0; j < unsolved_goals.size(); j++) {
//            VAL::proposition* goal = unsolved_goals[j];
//            if(isParentAgentProposition(goal) && all_dead_end_agent_goals_problem) { continue; }
//            stringstream prop_ss;
//            for (VAL::parameter_symbol_list::iterator pit = goal->args->begin();
//                 pit != goal->args->end(); pit++) {
//                VAL::parameter_symbol *parameter_symbol_instance = *pit;
//                prop_ss << " " << parameter_symbol_instance->getName();
//            }
//            active_agents_problem << "      (" << goal->head->getName() << prop_ss.str() << ")" << endl;
//        }
//        active_agents_problem << "   ))" << endl;
//
//        //add metric
//        active_agents_problem << "   (:metric minimize (total-time))" << endl;
//        active_agents_problem << ")" << endl;
//
//        active_agents_problem.close();
//
//        return problem_name+".pddl";
//
//    }

    ProblemGenerator::parent_agents_result ProblemGenerator::generateParentAgentGoalsProblem(string plan_file_name, string domain_identifier, ostream & stats, int current_decomposition, string node){
        /////// create parent agent plannig problem from all dead-end agent goals plan

        plan_file_name = plan_file_name;
        parent_agents_result parent_agents_result;

        //execute PlanToValStep to create script
        stringstream cmd;
        cmd.str("");
        cmd << "/home/nq/RALSTP/VAL_latest/build/linux64/release/bin/./PlanToValStep " << plan_file_name << ".plan.clean > " << plan_file_name << ".valstep";
        // stats << cmd.str() << endl;
        runCommand(cmd.str());

        //add problem file name to script
        ofstream script_file;
        script_file.open((plan_file_name + ".valstep"), std::ios_base::app);
        stringstream macro_file_name;
        macro_file_name << node << "context_" << current_decomposition << "_parent_agents_" << random << problem->name;
        script_file << "w " << macro_file_name.str() << ".macro " << endl << "q" << endl;

        //run ValStep on script to output strategic agents problem template
        cmd.str("");
        if (domain_identifier.compare(original_domain_file_name) != 0) {
            cmd << "/home/nq/RALSTP/VAL_latest/build/linux64/release/bin/./ValStep -i " << plan_file_name << ".valstep " << node << "context_" << domain_identifier << "_strategic_domain_" << original_problem_file_name << " " << node << "context_" << domain_identifier << "_strategic_problem_" << original_problem_file_name;
            stats << " B " << cmd.str() << endl;
        }
        else {
            cmd << "/home/nq/RALSTP/VAL_latest/build/linux64/release/bin/./ValStep -i " << plan_file_name << ".valstep " << domain_identifier << " " << plan_file_name << ".pddl";
            stats << " A " << cmd.str() << endl;
        }
        runCommand(cmd.str());



        /*
        * parse plan and create priority based agent goals problem
        */
        //populate strategic template with remaining agent goals
        ifstream macro_file;
        macro_file.open(macro_file_name.str() + ".macro");
        ofstream orig_problem;
        orig_problem.open(macro_file_name.str()+".pddl");
        //landmarks_problem.open(macro_file_name.str()+"_lm.pddl");
        string lm_agent;
        bool empty_goal_state = true;
        string line;
        vector<VAL::proposition*> unsolved_parent_agent_goals;
        for (int i = 0; i < active_parent_agent_goals.size(); i++){
            bool solved_goal = false;
            VAL::proposition* goal = active_parent_agent_goals[i];
            stringstream prop_ss;
            prop_ss << "(" << goal->head->getName();
            for (VAL::parameter_symbol_list::iterator pit = goal->args->begin();
                 pit != goal->args->end(); pit++) {
                VAL::parameter_symbol *parameter_symbol_instance = *pit;
                prop_ss << " " << parameter_symbol_instance->getName();
            }
            prop_ss << ")";
            macro_file.close();
            macro_file.open(macro_file_name.str() + ".macro");
            while (getline(macro_file, line)) {
                if(line.find(prop_ss.str(), 0) != std::string::npos){
                    solved_goal = true;
                    break;
                }
            }
            if (!solved_goal){
                unsolved_parent_agent_goals.push_back(goal);
                empty_goal_state = false;
            }
        }



        macro_file.close();
        macro_file.open(macro_file_name.str() + ".macro");
        bool init_found = false;
        while (getline(macro_file, line)) {
            if (line.find("(:init", 0) != std::string::npos){
                init_found = true;
                orig_problem << "(define (problem " << original_problem_file_name.substr(0,original_problem_file_name.size()-5) << ")" << endl;
                orig_problem << "    (:domain " << problem->domain_name << ")" << endl;
                orig_problem << "    (:objects" << endl;


                //landmarks_problem << line << endl;
                //TODO fix multiple layers bug
                bool lm_agent_found = false;

                for (VAL::const_symbol_list::iterator sit = problem->objects->begin(); sit != problem->objects->end(); sit++){
                    VAL::const_symbol* const_symbol_instance = *sit;
                    if (isDeadEndAgent(getObjectType(const_symbol_instance->getName())->getName())) {continue;}
                    orig_problem << "    " << const_symbol_instance->getName() << " - " << getObjectType(const_symbol_instance->getName())->getName() << endl;
                    //landmarks_problem << "    " << const_symbol_instance->getName() << " - " << getObjectType(const_symbol_instance->getName())->getName() << endl;
//                    bool allowed = true;
//                    for(int j = 0; j < agents_structure.size(); j++) {
//                        //compare priority of agent with allowed proirities
//                        if (getObjectType(const_symbol_instance->getName())->getName().compare(agents_structure[j].type) == 0) {
//                            if (agents_structure[j].first == agents_structure[agents_structure.size() - 1].first) {
//                                if (!lm_agent_found) {
//                                    //landmarks_problem << "    " << const_symbol_instance->getName() << " - "
//                                                      << getObjectType(const_symbol_instance->getName())->getName()
//                                                      << endl;
//                                    lm_agent_found = true;
//                                    lm_agent = const_symbol_instance->getName();
//                                }
//                                allowed = false;
//                            }
//                        }
//                    }
//                    //if(allowed)landmarks_problem << "    " << const_symbol_instance->getName() << " - " << getObjectType(const_symbol_instance->getName())->getName() << endl;
                }
                orig_problem << "    )" << endl;
                orig_problem << line << endl;
                continue;
            }
            if (!init_found) {continue;}
            if ((line.find("mission", 0) != std::string::npos) || (line.find("stp_", 0) != std::string::npos)){
                continue;
            }
            if (macro_file.peek() != EOF){
                // skip line if it contains finalisaed goal agent
//                bool containsFinalised = false;
//                for (int j = 0; j < agent_groups.size(); ++j) {
//                    if(!agent_groups[j].has_parent_parent_dynamic_type()) continue;
//                    if(containsFinalised)break;
//                    for (int k = 0; k < agent_groups[j].getAgents().size(); k++) {
//                        if (((line.find(agent_groups[j].getAgents()[k]->getName()+" ", 0) != string::npos) || (line.find(agent_groups[j].getAgents()[k]->getName()+")", 0) != string::npos)) &&
//                            (agent_groups[j].getAgents()[k]->hasGoalState()) &&
//                            (!agent_groups[j].getAgents()[k]->isPriority())){
//                            containsFinalised = true;
//                            break;
//                        }
//                    }
//                }
//                if(containsFinalised) {continue;}
                bool has_dead_end_agent = false;
                for (VAL::const_symbol_list::iterator sit = problem->objects->begin(); sit != problem->objects->end(); sit++) {
                    VAL::const_symbol *const_symbol_instance = *sit;
                    if (isDeadEndAgent(getObjectType(const_symbol_instance->getName())->getName())) {
                        if((line.find(" "+const_symbol_instance->getName(), 0) != std::string::npos) || (line.find("("+const_symbol_instance->getName(), 0) != std::string::npos)) {
                            has_dead_end_agent = true;
                            break;
                        }
                    }
                }
                if(has_dead_end_agent) {continue;}

                orig_problem << line << endl;
                //landmarks_problem << line << endl;
//                if ((line.find(getObjectType(lm_agent)->getName(), 0) != std::string::npos) && (line.find(lm_agent, 0) == std::string::npos)){
//                    continue;
//                }
//                //landmarks_problem << line << endl;
            }
            else{
                orig_problem << endl << ")" << endl;
                //landmarks_problem << endl << ")" << endl;
                // add goals
                orig_problem << "  (:goal" << endl << "    (and" << endl;
                //landmarks_problem << "  (:goal" << endl << "    (and" << endl;
                for (int i = 0; i < active_parent_agent_goals.size(); i++){
                    VAL::proposition* goal = active_parent_agent_goals[i];
                    stringstream goal_ss;
                    goal_ss << "     (" << goal->head->getName();
                    for (VAL::parameter_symbol_list::iterator pit = goal->args->begin();
                         pit != goal->args->end(); pit++) {
                        VAL::parameter_symbol *parameter_symbol_instance1 = *pit;
                        goal_ss << " " << parameter_symbol_instance1->getName();
                    }
                    goal_ss << ")" << endl;
                    orig_problem << goal_ss.str();
                    //landmarks_problem << goal_ss.str();
                }
//                for (int j = 0; j < agent_groups.size(); ++j) {
//                    if(!agent_groups[j].has_parent_parent_dynamic_type()) continue;
//                    for (int k = 0; k < agent_groups[j].getAgents().size(); k++) {
//                        for(int p = 0; p < ac.size(); p++){
//                            bool generated = false;
////                            for(int q = 0; q < solved_goals.size(); q++){
////                                if(p == solved_goals[q]){
////                                    generated = true;
////                                    break;
////                                }
////                            }
//                            if(!generated){
//                                stringstream goal_ss;
//                                goal_ss << "     (" << goals[p]->head->getName();
//                                for (VAL::parameter_symbol_list::iterator pit = goals[p]->args->begin();
//                                     pit != goals[p]->args->end(); pit++) {
//                                    VAL::parameter_symbol *parameter_symbol_instance1 = *pit;
//                                    goal_ss << " " << parameter_symbol_instance1->getName();
//                                }
//                                goal_ss << ")" << endl;
//                                stats << goal_ss.str();
////                                if (((goal_ss.str().find((agent_groups[j].getAgents()[k]->getName()+" "), 0) != string::npos) || (goal_ss.str().find((agent_groups[j].getAgents()[k]->getName()+")"), 0) != string::npos)) &&
////                                if (((goal_ss.str().find((agent_groups[j].getAgents()[k]->getName()+" "), 0) != string::npos) || (goal_ss.str().find((agent_groups[j].getAgents()[k]->getName()+")"), 0) != string::npos)) &&
////                                    (agent_groups[j].getAgents()[k]->hasGoalState()) &&
////                                    (agent_groups[j].getAgents()[k]->isPriority())){
//
//                                if (((goal_ss.str().find((agent_groups[j].getAgents()[k]->getName()+" "), 0) != string::npos) || (goal_ss.str().find((agent_groups[j].getAgents()[k]->getName()+")"), 0) != string::npos)) &&
//                                    (agent_groups[j].getAgents()[k]->hasGoalState())){
//                                    orig_problem << goal_ss.str();
//                                    if (agent_groups[j].getAgents()[k]->isPriority()) {empty_goal_state = false;}
//                                    if(agent_groups[j].getPriority() == agent_groups[agent_groups.size()-1].getPriority()) {//landmarks_problem << goal_ss.str();}
//                                    agent_groups[j].getAgents()[k]->setPriority(false);
//
//                                    // add agent initial state facts that are different from goal (preserve state)
////                                        for(int q = 0; q < facts.size(); q++){
////                                            stringstream fact_ss;
////                                            fact_ss << "     (" << facts[q]->head->getName();
////                                            for (VAL::parameter_symbol_list::iterator fit = facts[q]->args->begin();
////                                                 fit != facts[q]->args->end(); fit++) {
////                                                VAL::parameter_symbol *parameter_symbol_instance2 = *fit;
////                                                fact_ss << " " << parameter_symbol_instance2->getName();
////                                            }
////                                            fact_ss << ")" << endl;
////                                            if((facts[q]->head->getName().compare(goals[p]->head->getName()) != 0) && ((fact_ss.str().find((agent_groups[j].getAgents()[k]->getName()+" "), 0) != string::npos) || (fact_ss.str().find((agent_groups[j].getAgents()[k]->getName()+")"), 0) != string::npos))){
////                                                orig_problem << fact_ss.str();
////                                                //landmarks_problem << fact_ss.str();
////                                            }
////                                        }
//                                }
//                            }
//                        }
//                    }
//                }
                orig_problem << "    )" << endl << "  )" << endl;
                //landmarks_problem << "    )" << endl << "  )" << endl;
            }
        }
        orig_problem << "  (:metric minimize (total-time))\n)" << endl;
        //landmarks_problem << "  (:metric minimize (total-time))\n)" << endl;


        //if (total_priorities==2){

        ofstream empty_plan_file;
        empty_plan_file.open("empty.plan");

        parent_agents_result.plan_file_name = "empty.plan";
        parent_agents_result.makespan = 0;
        parent_agents_result.plan_time = 0;
        parent_agents_result.valid = false;


        if (!empty_goal_state) {

            bool backup_planner = only_backup_planner;
            int threshold = 120;
            stats << "Attempting to solve all parent agent goals problem... " << macro_file_name.str()
                                                                             << ".pddl" << endl << endl;
            std::time_t parent_start_time = std::time(0);
            int time_since_start = parent_start_time - procedure_start_time;
            if ((max_time - time_since_start) < threshold) {
                stats << "Abandoned Parent Agent problem. Total-Time OVER: " << max_time << endl;
                stats << threshold << endl;
                stats << time_since_start << endl;
                return parent_agents_result;
            }
            else{
                stats << "   Threshold for solving Parent Agent goals: " << threshold << endl << endl;
                if (backup_planner){
                    cmd.str("");
                    cmd << "ulimit -t " << threshold << ";/home/nq/RALSTP/./optic-cplex -N " << original_domain_file_name << " " << macro_file_name.str() << ".pddl" << " > " << macro_file_name.str() << ".plan";
                    runCommand(cmd.str());
                }
                else{
                    char buffer[PATH_MAX]; //create string buffer to hold path
                    getcwd(buffer, sizeof(buffer));
                    string current_working_dir(buffer);
                    runCommand("rm -rf /home/nq/RALSTP/TPSHE/bin/stp_temp.*");
                    runCommand("rm -rf /home/nq/RALSTP/TPSHE/bin/tmp_stp_temp.*");

                    //create and move tpshe files to tpshe path
                    cmd << "/home/nq/RALSTP/TPSHE/bin/./plan.py she --time 0 --memory 4000000000000 --plan-file /home/nq/RALSTP/TPSHE/bin/stp_temp " << current_working_dir << "/" << original_domain_file_name << " " << current_working_dir << "/" << macro_file_name.str() << ".pddl";
                    runCommand(cmd.str());
                    runCommand("mv dom.pddl /home/nq/RALSTP/TPSHE/bin/");
                    runCommand("mv tdom.pddl /home/nq/RALSTP/TPSHE/bin/");
                    runCommand("mv tins.pddl /home/nq/RALSTP/TPSHE/bin/");


                    cmd.str("");
                    if (only_backup_planner){
                        cmd << "/home/nq/RALSTP/TPSHE/bin/./plan.py she --time " << threshold << " --memory 4000000000000 --plan-file /home/nq/RALSTP/TPSHE/bin/stp_temp " << current_working_dir << "/" << original_domain_file_name << " " << current_working_dir << "/" << macro_file_name.str() << ".pddl";

                    }
                    else{
                        cmd << "/home/nq/RALSTP/TPSHE/bin/./plan.py she --time " << threshold << " --memory 4000000000000 --plan-file /home/nq/RALSTP/TPSHE/bin/stp_temp " << current_working_dir << "/" << original_domain_file_name << " " << current_working_dir << "/" << macro_file_name.str() << ".pddl";
                    }
                    stats << "     Planner: Tpshe" << endl;

                    //runCommand(cmd.str());
                    std::promise<bool> p;
                    auto future = p.get_future();


                    std::thread t1(&ProblemGenerator::runCommandDetectOutput, this, cmd.str(), "No solution to be converted into temporal has been found", std::ref(p));
                    auto t1_handle = t1.native_handle();
                    t1.detach();
                    bool incompatible = false;

                    std::time_t planner_start_time = std::time(0);
                    bool plan_found = false;
                    while(!plan_found && !incompatible){
                        usleep(5000000);
                        std::time_t procedure_current_time = std::time(0);
                        ifstream plan;
                        plan.open("/home/nq/RALSTP/TPSHE/bin/stp_temp.1");

                        if(plan || ((procedure_current_time - planner_start_time) > threshold)){
                            pthread_cancel(t1_handle);
                            runCommand("killall -9 python downward");
                            plan_found = true;
                        }else{
                            auto status = future.wait_for(0ms);
                            if (status == std::future_status::ready) {
                                incompatible = true;
                            }
                        }
                    }
                    if (incompatible){
                        cout << "parent agent goals problem unsolvable with Tpshe" << endl;
                        backup_planner = true;
                        cmd.str("");
                        cmd << "ulimit -t " << (max_time - time_since_start) << ";/home/nq/RALSTP/./optic-cplex -N " << original_domain_file_name << " " << macro_file_name.str() << ".pddl" << " > " << macro_file_name.str() << ".plan";
                        runCommand(cmd.str());
//                        i--;
//                        continue
                    }else{
                        bool parsed_all_plans = false;
                        int plan_no = 1;
                        while(!parsed_all_plans){
                            ifstream plan;
                            plan.open("/home/nq/RALSTP/TPSHE/bin/stp_temp."  + to_string(plan_no));
                            if(plan){
                                cmd.str("");
                                cmd << "/home/nq/RALSTP/TPSHE/bin/./planSchedule /home/nq/RALSTP/TPSHE/bin/tdom.pddl /home/nq/RALSTP/TPSHE/bin/dom.pddl " << current_working_dir << "/" << macro_file_name.str() << ".pddl /home/nq/RALSTP/TPSHE/bin/stp_temp." << plan_no << " >> /home/nq/RALSTP/TPSHE/bin/tmp_stp_temp." << plan_no;
                                runCommand(cmd.str());
                                cmd.str("");

                                bool temporal_plan_ready = false;
                                while (!temporal_plan_ready){
                                    ifstream temporal_plan;
                                    temporal_plan.open("/home/nq/RALSTP/TPSHE/bin/tmp_stp_temp." + to_string(plan_no));
                                    if(temporal_plan) {temporal_plan_ready = true;}
                                    else {usleep(2000000);}
                                }

                                plan_no++;
                            }
                            else { parsed_all_plans = true;}
                        }
                    }
                }
            }

            //parse and output current priority problem
            parent_agents_result.plan_file_name = macro_file_name.str() + ".plan.clean";
            ifstream plan_file;
            if (backup_planner) { plan_file.open(macro_file_name.str() + ".plan");}
            else {
                cmd.str("");
                cmd << "touch " << macro_file_name.str() << ".plan";
                runCommand(cmd.str());
                cmd.str("");
                cmd << "cat /home/nq/RALSTP/TPSHE/bin/tmp_stp_temp.* >> " << macro_file_name.str() << ".plan";
                runCommand(cmd.str());
                runCommand("rm -rf /home/nq/RALSTP/TPSHE/bin/stp_temp.*");
                runCommand("rm -rf /home/nq/RALSTP/TPSHE/bin/tmp_stp_temp.*");

                plan_file.open((macro_file_name.str() + ".plan").c_str());
                vector<pair<double,string>> actions;
                double max_end_time = 0;
                while (getline(plan_file, line)) {
                    if ((line.find(";;;; init: (action) [duration]", 0) != std::string::npos)){
                        max_end_time = 0;
                        actions.clear();
                        continue;
                    }
                    if (line.find(": (", 0) != std::string::npos){
                        if (line.find(".0") != std::string::npos){
                            std::size_t pos = line.find(".0");
                            //line.replace(pos,2,".");
                        }
                        std::transform(line.begin(), line.end(), line.begin(),
                                       [](unsigned char c){ return std::tolower(c); });
                        pair<double,string> action;
                        action.first = stod(line.substr(0,line.find(":")));
                        action.second = line;
                        actions.push_back(action);
                        double duration = stod(line.substr(line.find("[")+1,line.find("]")));
                        if (max_end_time < (action.first + duration)) { max_end_time = action.first + duration ;}
                    }
                }
                plan_file.close();
                sort(actions.begin(), actions.end());
                stringstream best_plan;
                for (int l = 0; l < actions.size(); l++){
                    best_plan << actions[l].second << endl;
                }
                if (actions.size() > 0 ) {best_plan << "; TPSHE-cost " << std::setprecision(4) << std::fixed << max_end_time << endl;}
//                actions.insert(actions.begin(), best_plan.str());
                ofstream best_plan_file;
                best_plan_file.open(macro_file_name.str() + ".plan");
                best_plan_file << best_plan.str();
                best_plan_file.close();
                plan_file.open((macro_file_name.str() + ".plan").c_str());
            }

            ofstream clean_plan;
            bool found = false;
            clean_plan.open(parent_agents_result.plan_file_name);
            while (getline(plan_file, line)) {
                if (line.find("; Cost", 0) != std::string::npos) {
                    parent_agents_result.makespan = stod(line.substr(8, line.length() - 1));
                    found = true;
                }
                if (line.find("; Time", 0) != std::string::npos) {
                    parent_agents_result.plan_time = stod(line.substr(7, line.length() - 1));
                }
                if (line.find(": (", 0) != std::string::npos) {
                    clean_plan << line << endl;
                }
                if (line.find("; TPSHE-cost ", 0) != std::string::npos){
                    parent_agents_result.makespan = stod(line.substr(13, line.length()-1));
                    std::time_t parent_end_time = std::time(0);
                    parent_agents_result.plan_time = parent_end_time - parent_start_time;
                    found = true;
                }
            }
            if (!found) {
                stats << "Parent Agents Problem not sloved." << endl << endl;
                parent_agents_result.valid = false;
                backup_planner = true;
                //TODO add new decomposition node to get paret agent goals plan
//                stats << "Parent Agents Problem not sloved. Starting New Context Layer." << endl << endl;
//                cmd.str("");
//                node = node+noder;
//                cmd << "/home/nq/stp/temporal-landmarks/release/optic/./optic-clp -u -5 " << original_domain_file_name << " " << macro_file_name.str() << ".pddl" << " >> " << macro_file_name.str() << ".pddl" << ".tmplm.log";
//                cout << cmd.str() << endl;
//                runCommand(cmd.str());




            }
            else{
                stats << endl << "Parent Agent Goals solved. Makespan: " << parent_agents_result.makespan << " Plan time: " << parent_agents_result.plan_time << endl << endl;
            }

        }
        else{ stats << "All Parent Agent Goals are also achieved in the state where all Dead-end Agent Goals have been achieved." << endl << endl; }

        /////

        return parent_agents_result;
    }


    bool ProblemGenerator::generateAllProblems(std::chrono::_V2::system_clock::time_point begin, double procedure_start_time, bool all_dead_end_agent_goals_problem, bool active_agents_problem, bool allow_stp, string node) {

        bool plan_found = false;
        //save objects (types not loaded bug)
        for (VAL::const_symbol_list::iterator oit = problem->objects->begin();
             oit != problem->objects->end(); oit++) {
            VAL::const_symbol* const_symbol_object = *oit;
            //cout << "object " << const_symbol_object->getName() << endl;
            objects.push_back(*const_symbol_object);
        }

        // add constants as objects and fix type of constant not found
        if(domain->constants != NULL){
            for(VAL::const_symbol_list::iterator it = domain->constants->begin(); it != domain->constants->end(); it++) {
                VAL::const_symbol* const_symbol_object = *it;
                cout << "ERROR, POTENTIAL LEGACY BUG: the type of PDDL constant \"" << const_symbol_object->getName() << "\" is not parsed by the PDDL parser. Can be manually fixed by having the type of constant \"" << const_symbol_object->getName() << "\" part of the name of the constant when defined in the \"" << original_domain_file_name << "\" PDDL domain. Good example: plane1 - plane. Bad example: plane1 - aircraft." << endl;
                const_symbol_object->type = getObjectType(const_symbol_object->getName());
                objects.push_back(*const_symbol_object);
            }
        }

        //save types (types not loaded bug)
        for(VAL::pddl_type_list::iterator it = domain->types->begin(); it != domain->types->end(); it++) {
            VAL::pddl_type *temp_type = *it;
            types.push_back(*temp_type);
            VAL::pddl_type *temp_supertype;
            if(temp_type->type != NULL){
                temp_supertype = temp_type->type;
            }
            else {
                temp_supertype = new VAL::pddl_type("");
            }
            supertypes.push_back(*temp_supertype);
        }

        //save initial facts
        if (!all_dead_end_agent_goals_problem) {
            problem->initial_state->visit(this);
        }
        //cout << "import stage 2 save 1" << facts.size() << endl;
        for (int j = 0; j < func_name.size(); j++) {
            initial_func_name.push_back(func_name[j]);
            initial_values.push_back(values[j]);
            initial_func_args.push_back(func_args[j]);
            initial_operator_.push_back(operator_[j]);
        }


        //populate environment
        if(is_random)randomizeSets();
        //if(is_random)create_increasing_agent_pair_goals();
        if (!all_dead_end_agent_goals_problem){
            //cout << "import stage 2 start " << facts.size() << endl;
            //importProblem(node+"active_agents_problem_"+original_problem_file_name);
            importProblem(node+"active_agents_problem_"+original_problem_file_name);
            //cout << "import stage 2 end " << facts.size() << endl;
        }
        for (int j = 0; j < facts.size(); j++) {
            initial_facts.push_back(facts[j]);
        }
        //save initial goals
        for (int j = 0; j < goals.size(); j++) {
            initial_goals.push_back(goals[j]);
        }

        //cout << "import stage 2 save 2" << facts.size() << endl;

        //create active agent problem and all dead end agent goals problem
        //problem->the_goal->visit(this);
//






//        if (active_agents_problem) {
//            string active_agents_problem_name = generateActiveAgentsProblem(node,true,false);
//            stringstream cmd;
//            cmd.str("");
//            cmd << "./extract_landmarks " << original_domain_file_name << " " << active_agents_problem_name << " -1";
//            cout << cmd.str() << endl;
//            runCommand(cmd.str());
//
//            //run again
//            exit(0);
//        }
;
//        importProblem(node+"active_agents_problem_"+original_problem_file_name);

        //string active_agents_problem_name = generateActiveAgentsProblem(node,true,false);




        // save active agent goals
        for (int i = 0; i < goals.size(); i++){
            if(isParentAgentProposition(goals[i])) { active_parent_agent_goals.push_back(goals[i]); }
            else { active_dead_end_agent_goals.push_back(goals[i]); }
            active_agent_goals.push_back(goals[i]);
        }





        //solve all dead end agent goals problem


        //cout << "import stage 2 save 3" << facts.size() << endl;



        double makespan = 0;
        double plan_time = 0;


        //TODO are problemGoals and problemInitialState still in use?
        problemGoals = false;
        problemInitialState = false;


        /*
        * set agents
        */
        //clear agents
        for(int i = 0; i < agent_groups.size(); ++i){
            agent_groups[i].clear();
        }
        //see if object is agent and create agent
        for (VAL::const_symbol_list::iterator sit = problem->objects->begin(); sit != problem->objects->end(); sit++){
            VAL::const_symbol* const_symbol_object = *sit;
            const_symbol_object->type = getObjectType(const_symbol_object->getName());
            //cout << "checking "<< const_symbol_object->getName() << endl;
            if(isParentAgent(const_symbol_object->type->getName()) || isDeadEndAgent(const_symbol_object->type->getName())) {
                SimpleAgent* simpleAgent = new SimpleAgent(const_symbol_object,initial_goals,facts,values,func_name,func_args,operator_);

                // add agent to its group
                for(int j = 0; j < agent_groups.size(); ++j){
                    if(const_symbol_object->type->getName().compare(agent_groups[j].getType()) == 0){
                        agent_groups[j].addSimpleAgent(simpleAgent);
                        //cout << "added "<< const_symbol_object->getName() << " to " << agent_groups[j].getType() << endl;
                    }
                }
            }
        }


        //add final state goal set
//        map<string,vector<int>> goals_with_common_static_parameter;
//        for (int i = 0; i < goals.size(); i++){
//            VAL::proposition* goal = goals[i];
//            if(isParentAgentProposition(goal)) continue;
//            stringstream ss;
//            ss << goal->head->getName();
//            bool has_dead_end_agent = false;
//            for (VAL::parameter_symbol_list::iterator pit = goal->args->begin(); pit != goal->args->end(); pit++) {
//                string param = (*pit)->getName();
//                if(isDeadEndAgent(param)){
//                    has_dead_end_agent = true;
//                    continue;
//                }
//                ss << " " << param;
//            }
//            if(has_dead_end_agent) {goals_with_common_static_parameter[ss.str()].push_back(i);}
//        }
//        map<int,vector<int>> common_in_goal_sate_context;
//        int goal_set_index1 = 0;
//        for (map<string,vector<int>>::iterator mit = goals_with_common_static_parameter.begin(); mit != goals_with_common_static_parameter.end(); mit++){
//            common_in_goal_sate_context[goal_set_index1] = (*mit).second;
//            goal_set_index1++;
//            cout << "final goal set " << goal_set_index1 << endl;
//            for (int i = 0; i < (*mit).second.size(); i++){
//                int index = (*mit).second[i];
//                cout << propositionToString(goals[index]) << endl;
//            }
//            cout << endl;
//        }
//        SubgoalsSet* subgoals_set1 = new SubgoalsSet(common_in_goal_sate_context);
//        if (!this->addSubgoalsSet(*subgoals_set1)){
//            cout << "DUPLICATE DECOMPOSITION" << endl;
//        }

        //add initial state goal set
//        map<string,vector<int>> initial_facts_with_common_static_parameter;
//        for (int i = 0; i < facts.size(); i++){
//            VAL::proposition* fact = facts[i];
//            if(isParentAgentProposition(fact)) continue;
//            stringstream ss;
//            ss << fact->head->getName();
//            bool has_dead_end_agent = false;
//            for (VAL::parameter_symbol_list::iterator pit = fact->args->begin(); pit != fact->args->end(); pit++) {
//                string param = (*pit)->getName();
//                if(isDeadEndAgent(param)){
//                    has_dead_end_agent = true;
//                    continue;
//                }
//                ss << " " << param;
//            }
//            bool prop_is_goal = false;
//            if(has_dead_end_agent) {
//                for (int j = 0; j < goals.size(); j++) {
//                    VAL::proposition *goal = goals[j];
//                    if (isParentAgentProposition(goal)) continue;
//                    if (fact->head->getName().compare(goal->head->getName()) == 0) {
//                        prop_is_goal = true;
//                        break;
//                    }
//                }
//                if(prop_is_goal) {initial_facts_with_common_static_parameter[ss.str()].push_back(i);}
//            }
//        }
//        map<int,vector<int>> common_in_inital_sate_context;
//        int goal_set_index2 = 0;
//        for (map<string,vector<int>>::iterator mit = initial_facts_with_common_static_parameter.begin(); mit != initial_facts_with_common_static_parameter.end(); mit++){
//            common_in_inital_sate_context[goal_set_index2] = (*mit).second;
//            goal_set_index2++;
//            cout << "initial goal set " << goal_set_index2 << endl;
//            for (int i = 0; i < (*mit).second.size(); i++){
//                int index = (*mit).second[i];
//                cout << propositionToString(facts[index]) << endl;
//            }
//            cout << endl;
//        }
//        SubgoalsSet* subgoals_set2 = new SubgoalsSet(common_in_inital_sate_context);
//        if (!this->addSubgoalsSet(*subgoals_set2)){
//            cout << "DUPLICATE DECOMPOSITION" << endl;
//        }

        //cout << "import stage 2 save 4" << facts.size() << endl;

        ofstream stats;
        stringstream ss;
        ss << random << problem->name << ".stats";
        stats.open(ss.str(), std::ios_base::app);
        const auto time = std::chrono::system_clock::now();
        const std::time_t time_formated = std::chrono::system_clock::to_time_t(time);
        stats << endl;
        stats << std::ctime(&time_formated);
        stats << "============================ " << ss.str() << " ==== Contexts: "<< sigma.size()+1 << " ===================" << endl;
        stats << "Random: "; if(is_random)stats << "True"; else stats << "False";stats << endl;
        stats << "Connected Map: "; if(connected_map)stats << "True"; else stats << "False";stats << endl << endl;

        vector<result> results;
        //cout << "import stage 2 save 5" << facts.size() << endl;
        //all_dead_end_agent_goals_problem = false;
        if (all_dead_end_agent_goals_problem) {

            std::time_t adeag_start = std::time(0);
            stats << "Attempting All Dead End Agent Goals problem..." << endl << endl;
            //cout << "import stage 2 save 6" << facts.size() << endl;
            string all_dead_end_agent_goals_problem_name = generateActiveAgentsProblem(node,true,true,original_problem_file_name);

            //load all dead end agent goals problem into system
            //if(all_dead_end_agent_goals_problem) { importProblem("all_dead_end_agent_goals_"+original_problem_file_name); }

            result new_result;
            new_result.makespan = 99999999999999;
            new_result.plan_time = 99999999999999;
            new_result.max_planning_time = 0;
            new_result.valid = false;
            new_result.stp = false;

            string problem_name = original_problem_file_name.substr(0,original_problem_file_name.size()-5);
            int threshold_ad = 120;
            stringstream cmd;
            cmd.str("");

            bool backup_planner = only_backup_planner;
            while (true) {
                if (backup_planner) {
                    cmd.str("");
                    cmd << "ulimit -t " << threshold_ad << ";/home/nq/RALSTP/./optic-cplex -N " << original_domain_file_name << " "
                        << node << "all_dead_end_agent_goals_" << original_problem_file_name << " > " << node
                        << "all_dead_end_agent_goals_" << problem_name << ".plan";
                    runCommand(cmd.str());
                    break;
                } else {
                    char buffer[PATH_MAX]; //create string buffer to hold path
                    getcwd(buffer, sizeof(buffer));
                    string current_working_dir(buffer);
                    runCommand("rm -rf /home/nq/RALSTP/TPSHE/bin/stp_temp.*");
                    runCommand("rm -rf /home/nq/RALSTP/TPSHE/bin/tmp_stp_temp.*");

                    //create and move tpshe files to tpshe path
                    cmd.str("");
                    cmd
                            << "/home/nq/RALSTP/TPSHE/bin/./plan.py she --time 0 --memory 4000000000000 --plan-file /home/nq/RALSTP/TPSHE/bin/stp_temp "
                            << current_working_dir << "/" << original_domain_file_name << " " << current_working_dir
                            << "/" << node << "all_dead_end_agent_goals_" << original_problem_file_name;
                    runCommand(cmd.str());
                    runCommand("mv dom.pddl /home/nq/RALSTP/TPSHE/bin/");
                    runCommand("mv tdom.pddl /home/nq/RALSTP/TPSHE/bin/");
                    runCommand("mv tins.pddl /home/nq/RALSTP/TPSHE/bin/");

                    //run plan
                    cmd.str("");
                    cmd << "/home/nq/RALSTP/TPSHE/bin/./plan.py she --time "
                        << threshold_ad
                        << " --memory 4000000000000 --plan-file /home/nq/RALSTP/TPSHE/bin/stp_temp "
                        << current_working_dir << "/" << original_domain_file_name << " " << current_working_dir << "/"
                        << node << "all_dead_end_agent_goals_" << original_problem_file_name;
                    stats << "     Planner: Tpshe" << endl;

                    //runCommand(cmd.str());
                    std::promise<bool> p;
                    auto future = p.get_future();

                    std::thread t1(&ProblemGenerator::runCommandDetectOutput, this, cmd.str(),
                                   "No solution to be converted into temporal has been found", std::ref(p));
                    auto t1_handle = t1.native_handle();
                    t1.detach();
                    bool incompatible = false;

                    std::time_t planner_start_time = std::time(0);
                    bool plan_found = false;
                    while (!plan_found && !incompatible) {
                        usleep(5000000);
                        std::time_t procedure_current_time = std::time(0);
                        ifstream plan;
                        plan.open("/home/nq/RALSTP/TPSHE/bin/stp_temp.1");

                        if (plan || ((procedure_current_time - planner_start_time) > ((threshold_ad / 3) + 2))) {
                            pthread_cancel(t1_handle);
                            runCommand("killall -9 python downward");
                            plan_found = true;
                        } else {
                            auto status = future.wait_for(0ms);
                            if (status == std::future_status::ready) {
                                incompatible = true;
                            }
                        }
                    }
                    if (incompatible) {
                        stats << "all dead-end agent goals problem unsolvable with Tpshe" << endl;
                        backup_planner = true;
                        continue;
                    }
                    else{
                        bool parsed_all_plans = false;
                        int plan_no = 1;
                        while(!parsed_all_plans){
                            ifstream plan;
                            plan.open("/home/nq/RALSTP/TPSHE/bin/stp_temp."  + to_string(plan_no));
                            if(plan){
                                cmd.str("");
                                cmd << "/home/nq/RALSTP/TPSHE/bin/./planSchedule /home/nq/RALSTP/TPSHE/bin/tdom.pddl /home/nq/RALSTP/TPSHE/bin/dom.pddl " << current_working_dir << "/" << node << "all_dead_end_agent_goals_" << original_problem_file_name << " /home/nq/RALSTP/TPSHE/bin/stp_temp." << plan_no << " >> /home/nq/RALSTP/TPSHE/bin/tmp_stp_temp." << plan_no;
                                runCommand(cmd.str());
                                cmd.str("");

                                bool temporal_plan_ready = false;
                                while (!temporal_plan_ready){
                                    ifstream temporal_plan;
                                    temporal_plan.open("/home/nq/RALSTP/TPSHE/bin/tmp_stp_temp." + to_string(plan_no));
                                    if(temporal_plan) {temporal_plan_ready = true;}
                                    else {usleep(2000000);}
                                }

                                plan_no++;
                            }
                            else { parsed_all_plans = true;}
                        }
                        break;
                    }
                }
            }

            ifstream plan_file;
            if (backup_planner){ plan_file.open((node+"all_dead_end_agent_goals_"+problem_name+".plan").c_str()); }
            else{
                cmd.str("");
                cmd << "touch " << node << "all_dead_end_agent_goals_" << problem_name << ".plan";
                runCommand(cmd.str());
                cmd.str("");
                cmd << "cat /home/nq/RALSTP/TPSHE/bin/tmp_stp_temp.* >> " << node << "all_dead_end_agent_goals_" << problem_name << ".plan";
                runCommand(cmd.str());
                runCommand("rm -rf /home/nq/RALSTP/TPSHE/bin/stp_temp.*");
                runCommand("rm -rf /home/nq/RALSTP/TPSHE/bin/tmp_stp_temp.*");
                plan_file.open((node+"all_dead_end_agent_goals_"+problem_name+".plan").c_str());
                vector<pair<double,string>> actions;
                double max_end_time = 0;
                string line;
                while (getline(plan_file, line)) {
                    if ((line.find(";;;; init: (action) [duration]", 0) != std::string::npos)){
                        max_end_time = 0;
                        actions.clear();
                        continue;
                    }
                    if (line.find(": (", 0) != std::string::npos){
                        if (line.find(".0") != std::string::npos){
                            std::size_t pos = line.find(".0");
                            //line.replace(pos,2,".");
                        }
                        std::transform(line.begin(), line.end(), line.begin(),
                                       [](unsigned char c){ return std::tolower(c); });
                        pair<double,string> action;
                        action.first = stod(line.substr(0,line.find(":")));
                        action.second = line;
                        actions.push_back(action);
                        double duration = stod(line.substr(line.find("[")+1,line.find("]")));
                        if (max_end_time < (action.first + duration)) { max_end_time = action.first + duration ;}
                    }
                }
                plan_file.close();
                sort(actions.begin(), actions.end());
                stringstream best_plan;
                for (int l = 0; l < actions.size(); l++){
                    best_plan << actions[l].second << endl;
                }
                if (actions.size() > 0 ) {best_plan << "; TPSHE-cost " << std::setprecision(4) << std::fixed << max_end_time << endl;}
//                actions.insert(actions.begin(), best_plan.str());
                ofstream best_plan_file;
                best_plan_file.open(node+"all_dead_end_agent_goals_"+problem_name+".plan");
                best_plan_file << best_plan.str();
                best_plan_file.close();
                plan_file.open((node+"all_dead_end_agent_goals_"+problem_name+".plan").c_str());
            }


            double makespan = 0;
            double plan_time = threshold_ad + 0.0001;
            ofstream clean_plan;
            clean_plan.open((node+"all_dead_end_agent_goals_"+problem_name+".plan.clean").c_str());
            bool found = false;
            bool empty_plan = false;

            string line;
            while (getline(plan_file, line)) {
                //optic -N & popf
                if (line.find("; Cost", 0) != std::string::npos){
                    makespan = stod(line.substr(8, line.length()-1));
                    found = true;
                }
                //yahsp3
                if (line.find("; Makespan", 0) != std::string::npos){
                    makespan = stod(line.substr(11, line.length()-1));
                    found = true;
                }
                //optic & popf & yahsp3
                if (line.find("; Time", 0) != std::string::npos){
                    plan_time = stod(line.substr(7, line.length()-1));
                }

                if (line.find(": (", 0) != std::string::npos){
                    clean_plan << line << endl;
                }

                if (line.find("; TPSHE-cost ", 0) != std::string::npos){
                    makespan = stod(line.substr(13, line.length()-1));
                    std::time_t adeag_end = std::time(0);
                    plan_time = adeag_end - adeag_start;
                    found = true;
                }

                //optic & popf & yahsp3
                if ((line.find("; Cost: 0.000", 0) != std::string::npos) || (line.find("; Makespan: 0\n", 0) != std::string::npos)){
                    empty_plan = true;
                }

                // case RPG of problem not found
                if ((!found) && ((line.find(";; Problem unsolvable!", 0) != std::string::npos) || (line.find("unsolvable.", 0) != std::string::npos))){
                    stats << "All dead end agent goals problem unsolvable!";
                }
            }
            plan_file.close();
            clean_plan.close();
            if (!found){
                stats << "All dead end agent goals problem ran out of time all_dead> " << threshold_ad << endl << endl;
            }
            else{
                stats << endl << "All dead end agent goals solved. Makespan: " << makespan << " Plan time: "
                      << plan_time << endl << endl;
            }



            //create all dead end agent goals plan from strategic plan and validate its
            string validation_result = create_all_dead_end_agent_goals_plan_from_strategic_plan(node+"all_dead_end_agent_goals_"+problem_name+".plan.clean", node);
            if(validation_result.compare("Plan valid") != 0) {
                stats << node << " All Dead End Agent Goals Plan NOT valid 1" << endl << endl;
                stats << validation_result << endl << endl << endl << endl;
                return false;
            }
            else { stats << "All Dead End Agent Goals Plan is Valid" << endl << endl; }




            ProblemGenerator::parent_agents_result parent_agents_result = generateParentAgentGoalsProblem((node+"all_dead_end_agent_goals_"+problem_name).c_str(), original_domain_file_name, stats, -1, node);


            new_result.makespan = makespan + parent_agents_result.makespan;
            new_result.plan_time = plan_time + parent_agents_result.plan_time;
            //new_result.plan_file_name = node + "all_dead_end_agent_goals_"+problem_name+".plan.clean"+".purley_tactical";
            validation_result = create_initial_problem_plan(node+"all_dead_end_agent_goals_"+problem_name+".plan.clean"+".purley_tactical",parent_agents_result.plan_file_name,makespan, node);
            //create initial planning problem plan from all dead end agent goals plan and all parent agent goals plan and check if it is valid
            if(validation_result.compare("Plan valid") != 0) {
                stats << node << " Initial Plannig Problem Plan NOT valid - ADEAG" << endl;
                stats << validation_result << endl << endl << endl << endl;
                new_result.valid = false;
            }
            else {
                stats << "Initial Plannig Problem Plan Valid" << endl << endl;
                stats << "Final Concurrent Makespan: "  << new_result.makespan << " Total Planning Time: " << new_result.plan_time << " Plan File Name: " << new_result.plan_file_name << endl << endl << endl << endl;
                plan_found = true;
                new_result.plan_file_name = node + "all_dead_end_agent_goals_"+problem_name+".plan.clean"+".purley_tactical.initial";
                new_result.valid = true;
                best_makespan = new_result.makespan;
                runCommand("rm -rf "+original_problem_file_name.substr(0,original_problem_file_name.length()-5)+".best_plan:*");
                runCommand("cp " + new_result.plan_file_name + " " + original_problem_file_name.substr(0,original_problem_file_name.length()-5)+".best_plan:"+to_string(best_makespan));
            }




            subgoals_plan_time.push_back(new_result.plan_time);
            subgoals_makespan.push_back(new_result.makespan);

            subgoals_max_size.push_back(goals.size());
            subgoals_no_of_sets.push_back(0);
            subgoals_lm_sets.push_back(0);
            subgoals_goals_in_lm_subgoals.push_back(0);
            subgoals_goals_in_all_subgoals.push_back(active_dead_end_agent_goals.size());
            subgoals_decomposition.push_back(0);

            results.push_back(new_result);
            //merge plans
            //validate

            // add to result

        }

        //bool allow_stp = true;
        if(allow_stp){
            stats << "STP Start" << endl;
            bool has_all_landmarks_goals = false;
            // process no_lm goals and add to subgoals set if present
            for (int i = 0; i < sigma.size(); i++) {
                sigma[i].lm_subgoals = sigma[i].getSet().size();
                sigma[i].goals_in_lm_subgoals = sigma[i].getGoalsNoInSubgoals();
                //sigma[i] = createNO_LMsubgoals(sigma[i], getAgentPairs(), true);
                if (sigma[i].goals_in_lm_subgoals - this->get_dead_end_agent_goals() == 0) {
                    has_all_landmarks_goals = true;
                }
            }

//            stats << "STP 1" << endl;

            if ((sigma.size() == 0) || (!has_all_landmarks_goals)) {
//                stats << "STP 2" << endl;
//                stats << "Max Goal-Sets LM problem" << endl;
//                std::sort(sigma.begin(), sigma.end(),
//                          [](SubgoalsSet &l, SubgoalsSet &r) {
//                              if (l.getSet().size() != r.getSet().size())return l.getSet().size() < r.getSet().size();
//                              return l.getMaxSubgoalSize() > r.getMaxSubgoalSize();
//                          });
//                result max_spread = generateProblem(sigma[sigma.size()-1], stats, makespan, plan_time, true,
//                                                 sigma[sigma.size()-1].lm_subgoals, node);
//                double previous_makespan = best_makespan;
//                vector<string> previous_plan = runCommandReturnOutput("ls | grep "+original_problem_file_name.substr(0,original_problem_file_name.length()-5)+".best_plan:");
//                if (previous_plan.size() > 0){
//                    auto ppos = previous_plan[0].find(":");
//                    if (ppos != std::string::npos){
//                        previous_makespan = stod(previous_plan[0].substr(ppos + 1));;
//                    }
//                }
//                if ((max_spread.makespan < previous_makespan) && max_spread.valid){
//                    best_makespan = max_spread.makespan;
//                    runCommand("rm -rf "+original_problem_file_name.substr(0,original_problem_file_name.length()-5)+".best_plan:*");
//                    runCommand("cp " + max_spread.plan_file_name + " " + original_problem_file_name.substr(0,original_problem_file_name.length()-5)+".best_plan:"+to_string(best_makespan));
//                }
//                results.push_back(max_spread);


                map<int, vector<int>> subgoals_set;
                SubgoalsSet *subgoalsSet = new SubgoalsSet(subgoals_set);
                sigma.push_back(*subgoalsSet);
                sigma[0] = createNO_LMsubgoals(sigma[0], getAgentPairs(), false);
                sigma[0].goals_in_lm_subgoals = 0;
                stats << "Single NO LM problem" << endl;
                result result_no_lm = generateProblem(sigma[0], stats, makespan, plan_time, false, 99999, node, false);
                double previous_makespan = best_makespan;
                vector<string> previous_plan = runCommandReturnOutput("ls | grep "+original_problem_file_name.substr(0,original_problem_file_name.length()-5)+".best_plan:");
                if (previous_plan.size() > 0){
                    auto ppos = previous_plan[0].find(":");
                    if (ppos != std::string::npos){
                        previous_makespan = stod(previous_plan[0].substr(ppos + 1));;
                    }
                }
                if ((result_no_lm.makespan < previous_makespan) && result_no_lm.valid){
                    best_makespan = result_no_lm.makespan;
                    runCommand("rm -rf "+original_problem_file_name.substr(0,original_problem_file_name.length()-5)+".best_plan:*");
                    runCommand("cp " + result_no_lm.plan_file_name + " " + original_problem_file_name.substr(0,original_problem_file_name.length()-5)+".best_plan:"+to_string(best_makespan));
                }
                const auto after_solve = std::chrono::system_clock::now();
                std::chrono::duration<double> process_duration = after_solve - begin;
                std::time_t procedure_current_time = std::time(0);
                if (procedure_current_time - procedure_start_time > max_time) {
                    stats << "Abandoned Single NO LM problem. Total-Time OVER: " << max_time << endl;
                }
                results.push_back(result_no_lm);
            } else {

                // sort subgoals by smallest no of decompositions and set max time
                std::sort(sigma.begin(), sigma.end(),
                          [](SubgoalsSet &l, SubgoalsSet &r) {
                              if (l.getSet().size() != r.getSet().size())return l.getSet().size() < r.getSet().size();
                              return l.getMaxSubgoalSize() > r.getMaxSubgoalSize();
                          });

                //remove invalid sets
//                stats << "STP 3a " << sigma.size() << endl;
                vector<SubgoalsSet> temp_sigma;
//                temp_sigma.push_back(sigma[sigma.size()-1]);
                for (int i = 0; i < sigma.size(); i++) {
                    if ((sigma[i].goals_in_lm_subgoals - this->get_dead_end_agent_goals() != 0) && (sigma[i].lm_subgoals > 0)) {
                        continue;
                    }
                    else{
                        temp_sigma.push_back(sigma[i]);
                    }
                }
                sigma = temp_sigma;
//                stats << "STP 3b " << sigma.size() << endl;
                // remove duplicate and invalid goal sets
                temp_sigma.clear();
                for (int i = 0; i < sigma.size(); i++) {
                    if ((i > 0) && (sigma[i].getSet().size() == sigma[i - 1].getSet().size())){
                        bool has_same_goals = true;
                        for (int j = 0; j < sigma[i].getSet().size(); j++){
                            if (sigma[i].getSet()[j] != sigma[i-1].getSet()[j]) {has_same_goals = false;}
                        }
                        if (!has_same_goals){
                            temp_sigma.push_back(sigma[i]);
                        }
                    }
                    else{
                        temp_sigma.push_back(sigma[i]);
                    }
                }
                sigma = temp_sigma;
//                stats << "STP 3c " << sigma.size() << endl;

                // create no_lm subgoal
                map<int, vector<int>> subgoals_set;
                SubgoalsSet *subgoalsSet = new SubgoalsSet(subgoals_set);
                SubgoalsSet no_lm_set = createNO_LMsubgoals(*subgoalsSet, getAgentPairs(), false);
                no_lm_set.lm_subgoals = 0;
                no_lm_set.goals_in_lm_subgoals = 0;
                // add to front
                // sigma.insert(sigma.begin(), no_lm_set);
                // add tp end
                sigma.push_back(no_lm_set);
//                stats << "STP 3d " << sigma.size() << endl;
                // execute lm subgoals
                int decomposition_count = 0;
                bool lm_only = true;
                for (int i = 0; i < sigma.size(); i++) {
                    stats << "Time Passed: " << (std::time(0) - procedure_start_time) << endl;
                    if (std::time(0) - procedure_start_time > max_time) {
                        stats << "Abandoned: Not starting Context " << current_decomposition << " Total-Time OVER: " << max_time;
                        break;
                    }
                    //if(sigma[i].getSet().size() != 8)continue;
//                    if ((sigma[i].goals_in_lm_subgoals - this->get_dead_end_agent_goals() != 0) &&
//                        (sigma[i].lm_subgoals > 0)) {
//                        if (((sigma[i].goals_in_lm_subgoals - this->get_dead_end_agent_goals() != 0) && lm_only) || (sigma[i].generated_problem)){
//                        if ((i == sigma.size()-1) && lm_only){
//                            i = -1;
//                            stats << endl << "a lm only subgoals finished, continuing with no lm agent based decompositions" << endl;
//                            lm_only = false;
//                        }
//                        continue;
//                    }
                    // remove set with all dead-end agent goals
                    if (sigma[i].getSet()[0].size() == active_dead_end_agent_goals.size()){stats << "context " << i << " skipped" << endl; continue;};
                    sigma[i].generated_problem = true;
                    current_decomposition = decomposition_count;
                    //if (sigma.size() < 2)break;
                    result current_result;
                    if (sigma[i].lm_subgoals > 0) {
                        // execute lm subgoals
                        cout << "Generating problem " << current_decomposition << endl;
                        stats << endl << endl << "Context " << current_decomposition << endl << endl;
                        agent_constraints = true;
                        agent_constraints_index = 0;
                        set_all_constraints_in_vector(true);

                        current_result = generateProblem(sigma[i], stats, makespan, plan_time, true,
                                                         sigma[i].lm_subgoals, node, false);
                        double previous_makespan = best_makespan;
                        vector<string> previous_plan = runCommandReturnOutput("ls | grep "+original_problem_file_name.substr(0,original_problem_file_name.length()-5)+".best_plan:");
                        if (previous_plan.size() > 0){
                            auto ppos = previous_plan[0].find(":");
                            if (ppos != std::string::npos){
                                previous_makespan = stod(previous_plan[0].substr(ppos + 1));;
                            }
                        }
                        if ((current_result.makespan < previous_makespan) && current_result.valid){
                            best_makespan = current_result.makespan;
                            runCommand("rm -rf "+original_problem_file_name.substr(0,original_problem_file_name.length()-5)+".best_plan:*");
                            runCommand("cp " + current_result.plan_file_name + " " + original_problem_file_name.substr(0,original_problem_file_name.length()-5)+".best_plan:"+to_string(best_makespan));
                        }
                    } else {
                        //execute no lm subgoal
                        current_decomposition = 99999;
                        cout << "Generating NO LM problem. Subgoals set size: " << sigma.size() << endl;
                        stats << endl << endl << "Context " << current_decomposition << " NO LM " << endl << endl;
                        agent_constraints = true;
                        agent_constraints_index = 0;
                        set_all_constraints_in_vector(true);
                        current_result = generateProblem(sigma[i], stats, makespan, plan_time, false, 99999, node, false);
                        double previous_makespan = best_makespan;
                        vector<string> previous_plan = runCommandReturnOutput("ls | grep "+original_problem_file_name.substr(0,original_problem_file_name.length()-5)+".best_plan:");
                        if (previous_plan.size() > 0){
                            auto ppos = previous_plan[0].find(":");
                            if (ppos != std::string::npos){
                                previous_makespan = stod(previous_plan[0].substr(ppos + 1));;
                            }
                        }
                        if ((current_result.makespan < previous_makespan) && current_result.valid){
                            best_makespan = current_result.makespan;
                            runCommand("rm -rf "+original_problem_file_name.substr(0,original_problem_file_name.length()-5)+".best_plan:*");
                            runCommand("cp " + current_result.plan_file_name + " " + original_problem_file_name.substr(0,original_problem_file_name.length()-5)+".best_plan:"+to_string(best_makespan));
                        }
                    }

                    const auto after_solve = std::chrono::system_clock::now();
                    std::chrono::duration<double> process_duration = after_solve - begin;
                    std::time_t procedure_current_time = std::time(0);
                    if (std::time(0) - procedure_start_time > max_time) {
                        stats << "Abandoned: Not adding result of Context " << current_decomposition << " Total-Time OVER: " << max_time
                              << endl;
                        break;
                    }
                    results.push_back(current_result);
//                    if ((i == sigma.size()-1) && lm_only){
//                        i = -1;
//                        stats << endl << "b lm only subgoals finished, continuing with no lm agent based decompositions" << endl;
//                        lm_only = false;
//                    }
                    decomposition_count++;
                }

            }
        }


        if (results.size() > 0){
            int best_decomposition = 0;
            double best_makespan = results[0].makespan;
            double total_planning_time = results[0].plan_time;
            double max_planning_time = results[0].max_planning_time;
            string valid = "";
            if(results[0].valid){valid = "Valid Plan";} else{valid= "INVALID PLAN";}
            if(results[0].stp){valid = "STP "+valid;} else{valid= "ADEAG "+valid;}
            for (int i = 1; i < results.size(); i++){
                if(results[i].makespan < best_makespan){
                    best_makespan = results[i].makespan;
                    best_decomposition = i;
                    if(results[i].valid){valid = "Valid Plan";} else{valid= "INVALID PLAN";}
                    if(results[i].stp){valid = "STP "+valid;} else{valid= "ADEAG "+valid;}
                }
                if(results[i].max_planning_time > max_planning_time){
                    max_planning_time = results[i].max_planning_time;
                }
                total_planning_time = total_planning_time + results[i].plan_time;
            }

            std::time_t procedure_current_time = std::time(0);

            stats << "Total Contexts: " << results.size() << endl;
            stats << "Best Context: " << best_decomposition << " | Agent + Non-Agent Concurrent Makespan: " << best_makespan << " | Total Planning Time: " << total_planning_time << endl << endl;
            ofstream all_results;
            all_results.open("all_results.txt", std::ios_base::app);
            all_results << valid << " " << original_problem_file_name << " best makespan: " << best_makespan << " total process time: " << (procedure_current_time - procedure_start_time) << " best context: " << best_decomposition << endl;
            all_results.close();
            ofstream individual_results;
            individual_results.open("individual_results.txt", std::ios_base::app);
            individual_results << valid << " " << original_problem_file_name << " best makespan: " << best_makespan << " total process time: " << (procedure_current_time - procedure_start_time) << " best context: " << best_decomposition << endl;
            individual_results.close();

            for(int i = 0; i < subgoals_makespan.size(); i++){
                stats << "Context " << subgoals_decomposition[i]
                      << " - All Subproblems: " << subgoals_no_of_sets[i]
                      << " - LM Subproblems: " << subgoals_lm_sets[i]
                      << " - NO LM Subproblems " << subgoals_no_of_sets[i] - subgoals_lm_sets[i]
                      << " - Not in LM goals: " << active_dead_end_agent_goals.size() - subgoals_goals_in_lm_subgoals[i]
                      << " - Not in ALL goals: " << active_dead_end_agent_goals.size() - subgoals_goals_in_all_subgoals[i]
                      << " - Max subgoal size: " << subgoals_max_size[i];
                if (subgoals_makespan[i] > 99999){
                    stats    << " - Plan time: FAILED - Makespan: FAILED" << endl;
                }
                else{
                    stats << " - Plan time: " << subgoals_plan_time[i]
                          << " - Makespan: " << subgoals_makespan[i] << endl;
                }

            }
            stats << endl;


        }
        else{
            stats << " No results" << endl;
        }
//        stats << std::ctime(&time_formated) << endl;
        //stats.close();

        return plan_found;
    }


    /*
     * VisitController
     */

    VAL::proposition* ProblemGenerator::stringToProposition(string prop_string){
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
                    //VAL::pddl_type* type = new VAL::pddl_type(getInstanceType(instance));
                    VAL::parameter_symbol* param = new VAL::parameter_symbol(instance);
                    param->type = NULL;
                    parameter_symbol_list->push_back(param);
                    prop_string = prop_string.substr(pos,prop_string.size());;

                }
                else{
                    //VAL::pddl_type* type = new VAL::pddl_type(getInstanceType(prop_string));
                    VAL::parameter_symbol* param = new VAL::parameter_symbol(prop_string);
                    param->type = NULL;
                    parameter_symbol_list->push_back(param);
                }
            }
        }
        else{
            head = new pred_symbol(prop_string.substr(1,prop_string.length()-2));
        }
        VAL::proposition* prop = new VAL::proposition(head,parameter_symbol_list);
        return prop;
    }

    string ProblemGenerator::propositionToString(VAL::proposition* prop){
        stringstream ss;
        ss << "(" << prop->head->getName();
        if(prop->args->begin() != prop->args->end()){
            for (VAL::parameter_symbol_list::iterator pit = prop->args->begin(); pit != prop->args->end(); pit++){
                VAL::parameter_symbol* param = *pit;
                bool is_constant = false;
                if (domain->constants != NULL){
                    for(VAL::const_symbol_list::iterator it = domain->constants->begin(); it != domain->constants->end(); it++) {
                        VAL::const_symbol* const_symbol_object = *it;
                        if (param->getName().compare(const_symbol_object->getName()) == 0) {
                            ss << " " << param->getName();
                            is_constant = true;
                            break;
                        }
                    }
                }
                if(!is_constant) {ss << " ?" << param->getName();}

            }
        }
        ss << ")";
        return ss.str();
    }



    //general
    void ProblemGenerator::visit_proposition(VAL::proposition *p) {
        cout << "visit_proposition" << endl;
    }
    void ProblemGenerator::visit_time_spec(VAL::time_spec ts){
        cout << "visit_time_spec" << endl;
        switch(ts){
            case E_AT_START  : last_operator_time_specs.push_back("at start");    break;
            case E_AT_END    : last_operator_time_specs.push_back("at end");      break;
            case E_OVER_ALL  : last_operator_time_specs.push_back("over all");    break;
            case E_CONTINUOUS: last_operator_time_specs.push_back("continuous");  break;
            case E_AT        : last_operator_time_specs.push_back("at");          break;
        }
        last_comparisons.push_back(last_comparison);
        last_values.push_back(last_value);
        last_comparison = "no comp";
        durative_action = false;
    }

    void ProblemGenerator::visit_const_symbol(const VAL::const_symbol *s){
        cout << "visit_const_symbol" << endl;
        cout << s->getName() << endl;
        cout << s->type->getName() << endl << endl;
    }

    //operators
    void ProblemGenerator::visit_operator_(VAL::operator_ *p) {
        cout << "visit_operator" << endl;
        p->visit(this);
    }
    void ProblemGenerator::visit_durative_action(VAL::durative_action * da) {
        durative_action = true;
        cout << "visit_durative_action" << endl;
        da->dur_constraint->visit(this);


    }

    //metric
    void ProblemGenerator::visit_metric_spec(VAL::metric_spec *s) {
        cout << "visit_metric_spec" << endl;
        s->expr->visit(this);
        switch (s->opt) {
            case VAL::E_MINIMIZE: optimization = "minimize"; break;
            case VAL::E_MAXIMIZE: optimization = "maximize"; break;
        }
    }

    //goals
    void ProblemGenerator::visit_simple_goal(VAL::simple_goal *g) {
        cout << "visit_simple_goal " << last_comparison << endl;
        if(problemGoals)goals.push_back(const_cast<VAL::proposition*>(g->getProp()));
        else {
            last_operator_propositions.push_back(const_cast<VAL::proposition*>(g->getProp()));
            last_expression << propositionToString(const_cast<VAL::proposition*>(g->getProp()));
            last_values.push_back(last_value);
        }
        switch (g->getPolarity()) {
            case VAL::E_NEG: last_polarity = true; break;
            case VAL::E_POS: last_polarity = false; break;
        }
    }

    void ProblemGenerator::visit_qfied_goal(VAL::qfied_goal *g) {
        cout << "visit_qfied_goal" << endl;
        g->getVars()->visit(this);
        g->getGoal()->visit(this);
    }
    void ProblemGenerator::visit_conj_goal(VAL::conj_goal *g) {
        cout << "visit_conj_goal" << endl;
        last_comparison = "no comp";
        g->getGoals()->visit(this);
    }
    void ProblemGenerator::visit_timed_goal(VAL::timed_goal *g) {
        cout << "visit_timed_goal" << endl;
        last_expression.str("");
        last_comparison = "no comp";
        g->getGoal()->visit(this);
        visit_time_spec(g->getTime());
        last_operator_string_propositions.push_back(last_expression.str());
        last_negative_effects.push_back(last_polarity);
    }
    void ProblemGenerator::visit_disj_goal(VAL::disj_goal *g) {
        cout << "visit_disj_goal" << endl;
        g->getGoals()->visit(this);
    }
    void ProblemGenerator::visit_imply_goal(VAL::imply_goal *g) {
        cout << "visit_imply_goal" << endl;
        g->getAntecedent()->visit(this);
        g->getConsequent()->visit(this);
    }
    void ProblemGenerator::visit_neg_goal(VAL::neg_goal *g) {
        cout << "visit_neg_goal" << endl;
        g->getGoal()->visit(this);
    }
    void ProblemGenerator::visit_con_goal(VAL::con_goal *g) {
        cout << "visit_con_goal" << endl;
//        g->getGoal()->visit(this);
    }
    void ProblemGenerator::visit_constraint_goal(VAL::constraint_goal *g) {
        cout << "visit_constraint_goal" << endl;
//        g->getGoal()->visit(this);
    }
    void ProblemGenerator::visit_goal(VAL::goal *g){
        cout << "visit_goal" << endl;
    }
    void ProblemGenerator::visit_comparison(VAL::comparison *c) {
        switch( c->getOp() )
        {
            case VAL::E_GREATER: last_comparison = ">"; break;
            case VAL::E_GREATEQ: last_comparison = ">="; break;
            case VAL::E_LESS: last_comparison = "<"; break;
            case VAL::E_LESSEQ: last_comparison = "<="; break;
            case VAL::E_EQUALS: last_comparison = "="; break;
        }
        cout << "visit_comparison " << last_comparison << endl;

        last_expression << "(" << last_comparison << " ";
        c->getLHS()->visit(this);
        last_expression << " ";
        c->getRHS()->visit(this);
        last_expression << ")";



//        ineq.LHS.tokens = last_expr.tokens;
//
//        last_expr.tokens.clear();

//
//        cout << "    visit right" << endl;
//        c->getRHS()->visit(this);
//        cout << "    visit right" << endl;
//        cout << last_comparison << " " << last_expression.str() << " " << last_value << endl;
//        if (!durative_action){
//            VAL::proposition* prop = stringToProposition(last_expression.str());
//            last_operator_propositions.push_back(prop);
//            cout << "asd " << last_operator_propositions.size() << endl;
//        }

//        ineq.RHS.tokens = last_expr.tokens;

//
//        item.ineq = ineq;
//        goals.push_back(item);
    }

//int base_type;
//int special_type;
//int comparison;
//static string operatpor_;
//static string function_name;
//static parameter_symbol_list func_param_list;
//static NumScalar value;

    //effects
    void ProblemGenerator::visit_assignment(VAL::assignment *e) {
        cout << "visit_assignment" << endl;
        string last_assignment = "";
        switch( e->getOp() )
        {
            case VAL::E_ASSIGN: last_assignment = "assign"; break;
            case VAL::E_INCREASE: last_assignment = "increase"; break;
            case VAL::E_DECREASE: last_assignment = "decrease"; break;
            case VAL::E_SCALE_UP: last_assignment = "??scale up??"; break;
            case VAL::E_SCALE_DOWN: last_assignment = "??scale down??"; break;
            case VAL::E_ASSIGN_CTS: last_assignment = "??assign cts??"; break;
        }


        last_expression << "(" << last_assignment << " ";


        if (last_assignment == "assign" && !durative_action){
            operator_.push_back("=");
            func_name.push_back(e->getFTerm()->getFunction()->getName()); //const func_symbol
            func_args.push_back(const_cast<VAL::parameter_symbol_list*>(e->getFTerm()->getArgs()));
        }


        last_expression << "(" << e->getFTerm()->getFunction()->getName();
        for (VAL::parameter_symbol_list::iterator fit = const_cast<VAL::parameter_symbol_list*>(e->getFTerm()->getArgs())->begin();
             fit != const_cast<VAL::parameter_symbol_list*>(e->getFTerm()->getArgs())->end(); fit++) {
            VAL::parameter_symbol *parameter_symbol_instance = *fit;
            last_expression << " ?" << parameter_symbol_instance->getName();
        }
        last_expression << ") ";

        e->getExpr()->visit(this); //const param_list
        last_expression << ")";
        last_negative_effects.push_back(last_polarity);
    }
    void ProblemGenerator::visit_effect_lists(VAL::effect_lists *e) {
        cout << "visit_effect_lists" << endl;
        last_polarity = false;
        e->add_effects.pc_list<VAL::simple_effect*>::visit(this);

        last_polarity = true;
        e->del_effects.pc_list<VAL::simple_effect*>::visit(this);
        last_polarity = false;

        e->forall_effects.pc_list<VAL::forall_effect*>::visit(this);
        e->cond_effects.pc_list<VAL::cond_effect*>::visit(this);
        e->cond_assign_effects.pc_list<VAL::cond_effect*>::visit(this);
        e->assign_effects.pc_list<VAL::assignment*>::visit(this);
        e->timed_effects.pc_list<VAL::timed_effect*>::visit(this);
    }
    void ProblemGenerator::visit_simple_effect(VAL::simple_effect *e) {
        cout << "visit_simple_effect" << " " << last_polarity << " " << e->prop->head->getName() << endl;

        if(problemInitialState)facts.push_back(const_cast<VAL::proposition*>(e->prop));
        else {
            last_operator_propositions.push_back(const_cast<VAL::proposition*>(e->prop));
            last_expression << propositionToString(const_cast<VAL::proposition*>(e->prop));
            last_negative_effects.push_back(last_polarity);
        }
    }
    void ProblemGenerator::visit_cond_effect(VAL::cond_effect *e) {
        cout << "visit_cond_effect" << endl;
        e->getCondition()->visit(this);
        e->getEffects()->visit(this);
    };
    void ProblemGenerator::visit_timed_effect(VAL::timed_effect *e){
        durative_action = true;
        last_expression.str("");
        cout << "visit_timed_effect" << endl;
        e->effs->visit(this);
        visit_time_spec(e->ts);
        cout << "    " << last_expression.str() << endl;
        last_operator_string_propositions.push_back(last_expression.str());
    }
    void ProblemGenerator::visit_timed_initial_literal(VAL::timed_initial_literal *s) {
        cout << "visit_timed_initial_literal" << endl;
    }
    //expressions

    void ProblemGenerator::visit_binary_expression(VAL::binary_expression *s) {
        cout << "visit_binary_expression" << endl;


    }

    void ProblemGenerator::visit_plus_expression(VAL::plus_expression *s) {
        cout << "visit_plus_expression" << endl;
        last_expression << "(+ ";
        s->getLHS()->visit(this);
        last_expression << " ";
        s->getRHS()->visit(this);
        last_expression << ")";
    }

    void ProblemGenerator::visit_minus_expression(VAL::minus_expression *s) {
        cout << "visit_minus_expression" << endl;
        last_expression << "(- ";
        s->getLHS()->visit(this);
        last_expression << " ";
        s->getRHS()->visit(this);
        last_expression << ")";
    }

    void ProblemGenerator::visit_mul_expression(VAL::mul_expression *s) {
        cout << "visit_mul_expression" << endl;
        last_expression << "(* ";
        s->getLHS()->visit(this);
        last_expression << " ";
        s->getRHS()->visit(this);
        last_expression << ")";
    }

    void ProblemGenerator::visit_div_expression(VAL::div_expression *s) {
        cout << "visit_div_expression" << endl;
        last_expression << "(/ ";
        s->getLHS()->visit(this);
        last_expression << " ";
        s->getRHS()->visit(this);
        last_expression << ")";


    }

    void ProblemGenerator::visit_uminus_expression(VAL::uminus_expression *s) {
        cout << "visit_uminus_expression" << endl;
        last_expression << "(- ";
        s->getExpr()->visit(this);
        last_expression << ")";

    }

    void ProblemGenerator::visit_num_expression(VAL::num_expression *s) {
        cout << "visit_num_expression" << endl;
    }

    void ProblemGenerator::visit_int_expression(VAL::int_expression *s) {
        cout << "visit_int_expression" << endl;
        last_expression << s->double_value();
        if(problemInitialState)values.push_back(s->double_value());
    }

    void ProblemGenerator::visit_float_expression(VAL::float_expression *s) {
        cout << "visit_float_expression" << endl;
        last_expression << s->double_value();
        if(problemInitialState)values.push_back(s->double_value());
    }

    void ProblemGenerator::visit_func_term(VAL::func_term *s) {
        cout << "visit_func_term" << endl;
        last_expression << "(" << s->getFunction()->getName();


        //TODO
        // serializable subgoals / val
        // rpg bruteforce / val

        for (VAL::parameter_symbol_list::const_iterator pit = s->getArgs()->begin(); pit != s->getArgs()->end(); pit++) {
            last_expression << " ?" << (*pit)->getName();
        }
        last_expression << ")";

    }

    void ProblemGenerator::visit_special_val_expr(VAL::special_val_expr *s) {
        cout << "visit_special_val_expr" << endl;

        switch(s->getKind()) {
            case VAL::E_HASHT:          metric = "??hasht??";  last_expression << " ?????hasht???? "; break;
            case VAL::E_DURATION_VAR:   metric = "total-duration";  last_expression << "?duration"; break;
            case VAL::E_TOTAL_TIME:     metric = "total-time"; last_expression << " ?????time???? "; break;
        }
    }



}