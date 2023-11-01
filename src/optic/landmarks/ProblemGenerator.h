//
// Created by Dorian Buksz on 01/05/2020.
//

#include <iostream>
#include "SimpleAgent.h"
#include <FlexLexer.h>
#include <iomanip>
#include <chrono>
#include <thread>
#include <future>

#ifndef TEMPORAL_LANDMARKS_PROBLEMGENERATOR_H
#define TEMPORAL_LANDMARKS_PROBLEMGENERATOR_H

using namespace Planner;
using namespace std;
//using namespace TIM;


namespace strategic_tactical {

    class SubgoalsSet{
    private:
        map<int,vector<int>> subgoals_set;
        vector<double> subgoals_plan_time;
        vector<double> subgoals_makespan;
        double total_plan_time = 0;
        double total_makespan = 0;
    public:
        SubgoalsSet(map<int,vector<int>> subgoals_set_) : subgoals_set(subgoals_set_){

        }
        virtual ~SubgoalsSet(){}

        //getters
        map<int,vector<int>> getSet(){return subgoals_set;};
        vector<double> getSubgoalsTime(){return subgoals_plan_time;};
        double getTotalTime(){return total_plan_time;};
        double getTotalMakespan(){return total_makespan;};
        int getGoalsNoInSubgoals();
        int getMaxSubgoalSize();

        //setters
        void addSubgoalTime(double plan_time_){subgoals_plan_time.push_back(plan_time_);};
        void addToTotalTime(double plan_time_){total_plan_time = total_plan_time + plan_time_;}
        void addSubgoalMakespan(double makespan_){subgoals_makespan.push_back(makespan_);};
        void addToTotalMakespan(double makespan_){total_makespan = total_makespan + makespan_;}
        void addSubgoal(vector<int>subgoal_){subgoals_set[subgoals_set.size()]=subgoal_;}
        void clear(){subgoals_set.clear();}

        int lm_subgoals = 0;
        int goals_in_lm_subgoals;
        bool generated_problem = false;

    };

//static class CoreExpression{
//private:
//
//public:
//    int base_type;
//    int special_type;
//    int comparison;
//    static string operator_;
//    static string function_name;
//    static parameter_symbol_list func_param_list;
//    static NumScalar value;
//};
//
//class CompleteExpression{
//private:
//    vector<CoreExpression> expression;
//public:
//    CompleteExpression(){}
//    virtual ~CompleteExpression(){}
//};
    class ProblemGenerator : public VAL::VisitController{
    private:
        //instances + goals
        VAL::domain* domain = VAL::current_analysis->the_domain;
        vector<VAL::pddl_type> types;
        vector<VAL::pddl_type> supertypes;

        VAL::problem* problem = VAL::current_analysis->the_problem;
        vector<VAL::proposition*> initial_goals;

        vector<int> goals_in_initial_state;
        vector<VAL::proposition*> active_agent_goals;
        vector<VAL::proposition*> active_dead_end_agent_goals;
        vector<VAL::proposition*> active_parent_agent_goals;
        vector<VAL::const_symbol> objects;

        vector<SubgoalsSet> sigma;
        vector<int>solved_goals;
        vector<int>solved_goals_old;
        vector<int> current_dead_end_agent_goal_set;
        string random = "";
        string original_domain_file_name;
        string original_problem_file_name;
        double procedure_start_time;
        bool is_random = false;
        vector<RPGBuilder::agent> agents_structure;
        vector<string> dynamic_types_vector;
        vector<AgentGroup> agent_groups;
        int total_priorities = 1;
        bool agent_constraints = true;
        vector<bool> agent_contstraints_vector;
        vector<bool> agent_contstraints_vector_backup;
        int agent_constraints_index = 0;
        vector<string> last_operator_time_specs;
        vector<VAL::proposition*> last_operator_propositions;
        vector<string> last_operator_string_propositions;
        vector<bool> last_negative_effects;
        vector<string>last_comparisons;
        string last_comparison = "no comp";
        stringstream last_comp;
        double last_value = 9999999;
        vector<double> last_values;
        bool durative_action = false;
        stringstream last_expression;
        bool problemGoals = true;
        bool problemInitialState = true;
        bool last_polarity = false;
        vector<string> tactical_problem_names;
        bool connected_map = false;
        int dead_end_agent_goals;
        bool only_backup_planner = false;
        std::chrono::duration<double> tactical_plan_solve_time;
        std::chrono::duration<double> strategic_plan_solve_time;
        bool unsolvable = false;
        int max_time = 1800;
        double best_makespan = 99999999;


        struct result {
            double makespan;
            double plan_time;
            double max_planning_time;
            double agent_plan_time;
            string plan_file_name;
            bool valid = false;
            bool stp = true;
        };






        //functions
        vector<VAL::NumScalar> values;
        vector<string> func_name;
        vector<VAL::parameter_symbol_list*> func_args;
        vector<string> operator_;

        //metric
        string optimization;
        string metric;


        //single agent parameters
        vector<VAL::proposition*> initial_facts;
        vector<VAL::NumScalar> initial_values;
        vector<string> initial_func_name;
        vector<VAL::parameter_symbol_list*> initial_func_args;
        vector<string> initial_operator_;

        struct parent_agents_result {
            double makespan;
            double plan_time;
            string plan_file_name;
            bool valid;
        };



        parent_agents_result generateParentAgentGoalsProblem(string all_dead_end_agent_goals_plan, string domain_identifier, ostream & stats, int current_decomposition, string node);
        result generateProblem(SubgoalsSet subgoals_set_, ostream & stats, double pre_makespan, double pre_plan_time, bool lm_tactical, int lm_subgoals, string node, bool all_dead_end_agent_goals);

        int get_dead_end_agent_goals(){return dead_end_agent_goals;};
        SubgoalsSet randomizer(SubgoalsSet current_set);
        bool is_duplicate_subgoal(SubgoalsSet new_set);
        int getAgentPairs();
        bool isSameProposition(VAL::proposition* origin, VAL::proposition* target);
        SubgoalsSet createNO_LMsubgoals(SubgoalsSet subgoals_set_, int agent_pairs, bool lm_tactical);
        void randomizeSets();
        void create_increasing_agent_pair_goals();
        bool isRejectedFact(VAL::proposition* fact);
        bool isParentAgent(string type);
        bool isParentAgentProposition(VAL::proposition* proposition);
        bool isDeadEndAgent(string type);
        bool isDeadEndAgentProposition(VAL::proposition* proposition);
        VAL::pddl_type* getObjectType(string name);
//        bool isInstanceInInitialFacts(string name);
        VAL::proposition* updateIfAgentGoalState(VAL::proposition* fact);

        std::string create_all_dead_end_agent_goals_plan_from_strategic_plan(string all_dead_end_agent_goals_strategic_plan, string node);
        std::string create_initial_problem_plan(string all_dead_end_agent_goals_plan, string all_parent_agent_goals_plan, double all_dead_end_agent_goals_plan_makespan, string node);
        bool agent_constraints_in_vector(vector<bool> agent_contstraints_vector){
            bool constraints = false;
            for (int i = 0; i < agent_contstraints_vector.size(); i++){
                if(agent_contstraints_vector[i] == true){
                    constraints = true;
                }
            }
            return constraints;
        };
        void set_all_constraints_in_vector(bool value){
            for (int i = 0; i < agent_contstraints_vector.size(); i++){
                agent_contstraints_vector[i] = value;
            }
        }


        //output
        vector<double> subgoals_max_size;
        vector<double> subgoals_plan_time;
        vector<double> subgoals_makespan;
        vector<double> subgoals_no_of_sets;
        vector<double> subgoals_lm_sets;
        vector<double> subgoals_goals_in_lm_subgoals;
        vector<double> subgoals_goals_in_all_subgoals;
        vector<double> subgoals_decomposition;

        int current_lm_sets;
        int current_goals_in_lm_subgoals;
        int current_goals_in_all_subgoals;
        int current_decomposition = 99999;

    public:
        ProblemGenerator(bool is_random_, string original_problem_file_name_, string original_domain_file_name_, double procedure_start_time_, vector<RPGBuilder::agent> agents_structure_, vector<string> dynamic_types_vector_, bool connected_map_, int dead_end_agent_goals_) : is_random(is_random_), original_problem_file_name(original_problem_file_name_), original_domain_file_name(original_domain_file_name_), procedure_start_time(procedure_start_time_), agents_structure(agents_structure_), dynamic_types_vector(dynamic_types_vector_), connected_map(connected_map_), dead_end_agent_goals(dead_end_agent_goals_){
            //sort(agents_structure.begin(), agents_structure.end());
            for (int i = 0; i < agents_structure.size(); i++){
                for(VAL::pddl_type_list::iterator it = domain->types->begin(); it != domain->types->end(); it++){
                    VAL::pddl_type* temp_type = *it;
                    VAL::pddl_type* temp_super_type = *it;
                    vector<VAL::pddl_type*> temp_supertypes;
                    cout << "debug supertype " << temp_type->getName() << endl;
                    //loop through all supertypes
                    while (temp_super_type->type != NULL){
                        cout << "    debug supertype " << temp_super_type->type->getName() << endl;
                        temp_supertypes.push_back(temp_super_type->type);
                        temp_super_type = temp_super_type->type;
                    }


                    if(temp_type->getName().compare(agents_structure[i].type) == 0){
                        auto agentGroup = new AgentGroup(temp_type,temp_supertypes,agents_structure[i].priority,agents_structure[i].is_parent);
                        agent_groups.push_back(*agentGroup);
                        if((*agentGroup).has_parent_parent_dynamic_type()) {agent_contstraints_vector.push_back(true);}
                    }
                }
                if(i>0){
                    if(agents_structure[i].priority != agents_structure[i-1].priority)total_priorities++;
                }

            }
        }

        vector<VAL::proposition*> goals;
        vector<VAL::proposition*> facts;

        ProblemGenerator(VAL::domain* domain_, VAL::problem* problem_) {
            this->domain = domain_;
            this->problem = problem_;
            problemGoals = false;
            problemInitialState = false;
        }

        ProblemGenerator(bool is_random_, VAL::domain* domain_, VAL::problem* problem_, string original_problem_file_name_, string original_domain_file_name_, double procedure_start_time_, vector<RPGBuilder::agent> agents_structure_, vector<string> dynamic_types_vector_) : is_random(is_random_), original_problem_file_name(original_problem_file_name_), original_domain_file_name(original_domain_file_name_), procedure_start_time(procedure_start_time_), agents_structure(agents_structure_), dynamic_types_vector(dynamic_types_vector_){
            this->domain = domain_;
            this->problem = problem_;
            for (int i = 0; i < agents_structure.size(); i++){
                for(VAL::pddl_type_list::iterator it = domain->types->begin(); it != domain->types->end(); it++){
                    VAL::pddl_type* temp_type = *it;
                    VAL::pddl_type* temp_super_type = *it;
                    vector<VAL::pddl_type*> temp_supertypes;

                    //loop through all supertypes
                    while (temp_super_type->type != NULL){
                        temp_supertypes.push_back(temp_super_type->type);
                        temp_super_type = temp_super_type->type;
                    }

                    if(temp_type->getName().compare(agents_structure[i].type) == 0){
                        auto agentGroup = new AgentGroup(temp_type,temp_supertypes,agents_structure[i].priority,agents_structure[i].is_parent);
                        agent_groups.push_back(*agentGroup);
                    }
                }
                if(i>0){
                    if(agents_structure[i].priority != agents_structure[i-1].priority)total_priorities++;
                }
                agent_contstraints_vector.push_back(true);
            }
            //save objects (types not loaded bug)
            for (VAL::const_symbol_list::iterator oit = problem->objects->begin();
                 oit != problem->objects->end(); oit++) {
                VAL::const_symbol* const_symbol_object = *oit;
                objects.push_back(*const_symbol_object);
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
        }
        virtual ~ProblemGenerator() {}

        //main
        bool addSubgoalsSet(SubgoalsSet subgoals_set_);
        bool generateAllProblems(std::chrono::_V2::system_clock::time_point begin, double procedure_start_time, bool all_dead_end_agent_goals_problem, bool active_agents_problem, bool allow_stp, string node);
        std::vector<vector<VAL::proposition*>> getOperatorsParameters();
        std::vector<vector<VAL::proposition*>> getOperatorsPreconditions();
        std::vector<vector<VAL::proposition*>> getOperatorsEffects();
        string generateActiveAgentsProblem(string node, bool execute_problem, bool all_dead_end_agent_goals_problem, string original_problem_file_name);
        void importProblem(string temp_state);
        void importProblemAndDomain(string problem_name, string domain_name);

        void runCommand(string cmd);
        void runCommandDetectOutput(string cmd, string target, std::promise<bool>& p);
        vector<string> runCommandReturnOutput(string cmd);

//        void runCommandThread (string cmd){
//            std::thread t1(&ProblemGenerator::runCommand, this, cmd);
//            t1.join();
//        }

        /*
         * VisitController
         */
        VAL::proposition* stringToProposition(string prop_string);
        string propositionToString(VAL::proposition* prop);

        //general
        virtual void visit_proposition(VAL::proposition *);
        virtual void visit_operator_(VAL::operator_ *) ;
        virtual void visit_durative_action(VAL::durative_action * da);
        virtual void visit_time_spec(VAL::time_spec ts);
        virtual void visit_const_symbol(const VAL::const_symbol *s);

        //goals
        virtual void visit_simple_goal(VAL::simple_goal *);
        virtual void visit_qfied_goal(VAL::qfied_goal *);
        virtual void visit_conj_goal(VAL::conj_goal *);
        virtual void visit_timed_goal(VAL::timed_goal *);
        virtual void visit_disj_goal(VAL::disj_goal *);
        virtual void visit_imply_goal(VAL::imply_goal *);
        virtual void visit_neg_goal(VAL::neg_goal *);
        virtual void visit_con_goal(VAL::con_goal *);
        virtual void visit_constraint_goal(VAL::constraint_goal *);
        virtual void visit_goal(VAL::goal *);


        //effects
        virtual void visit_effect_lists(VAL::effect_lists *) ;
        virtual void visit_simple_effect(VAL::simple_effect *) ;
        virtual void visit_cond_effect(VAL::cond_effect *) ;
        virtual void visit_timed_effect(VAL::timed_effect *) ;
        virtual void visit_timed_initial_literal(VAL::timed_initial_literal *) ;
        virtual void visit_assignment(VAL::assignment *) ;
        virtual void visit_comparison(VAL::comparison * c);

        //metric
        virtual void visit_metric_spec(VAL::metric_spec *);

        //expressions
        virtual void visit_binary_expression(VAL::binary_expression *);
        virtual void visit_plus_expression(VAL::plus_expression *);
        virtual void visit_minus_expression(VAL::minus_expression *);
        virtual void visit_mul_expression(VAL::mul_expression *);
        virtual void visit_div_expression(VAL::div_expression *);
        virtual void visit_uminus_expression(VAL::uminus_expression *);
        virtual void visit_num_expression(VAL::num_expression *);
        virtual void visit_int_expression(VAL::int_expression *);
        virtual void visit_float_expression(VAL::float_expression *);
        virtual void visit_special_val_expr(VAL::special_val_expr *);
        virtual void visit_func_term(VAL::func_term *);
    };


}


#endif //TEMPORAL_LANDMARKS_PROBLEMGENERATOR_H
