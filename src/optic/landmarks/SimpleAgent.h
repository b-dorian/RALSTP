//
// Created by Dorian Buksz on 31/05/2020.
//

#include "TemporalLandmark.h"

#ifndef STP_SIMPLEAGENT_H
#define STP_SIMPLEAGENT_H

using namespace Planner;
using namespace std;


namespace strategic_tactical{
    class SimpleAgent : public VAL::VisitController{


    private:
        string subtype, supertype;
        VAL::const_symbol* agent_instance;
        string name = agent_instance->getName();
        VAL::pddl_type* type = agent_instance->type;
        //vector<int> subproblem_goals;
        vector<VAL::NumScalar> values;
        vector<string> func_name;
        vector<VAL::parameter_symbol_list*> func_args;
        vector<string> operator_;
        vector<VAL::proposition*> all_goals;
        vector<VAL::proposition*> initial_state_facts;
        vector<VAL::proposition*> agent_output_facts;
        vector<VAL::proposition*> agent_initial_facts;
        vector<VAL::proposition*> agent_finalized_facts;
        vector<string> single_attribute_agent_facts;
        vector<VAL::parameter_symbol*> output_facts_attributes;
        vector<VAL::proposition*> last_operator_propositions;
        bool initialized = false;
        bool available = true;
        bool reused = false;
        bool priority;
        bool goalState;

        bool setAgentGoalFacts();
        //void setNonGoalInitialStateFacts();
        void setAgentInitialStateFacts();
        // void setOutputFactsAttributes();
        void setAgentFinalizedFacts();
        VAL::proposition* assign_types_to_proposition(VAL::proposition* proposition);
        //effects
        virtual void visit_effect_lists(VAL::effect_lists *) ;
        virtual void visit_timed_effect(VAL::timed_effect *) ;
        virtual void visit_simple_effect(VAL::simple_effect *) ;

        //TODO add for function goals (now only for fact goals)

        //TODO MAYBE
//    vector<string> supertypes;
//    vector<VAL::proposition> predicates;
//    vector<VAL::proposition> non_goal_predicates;
//    void setAllPredicates();
//    void setNonGoalPredicates();

    public:
        SimpleAgent(VAL::const_symbol* agent_instance_, vector<VAL::proposition*> all_goals_, vector<VAL::proposition*> initial_state_facts_, vector<VAL::NumScalar> values_, vector<string> func_name_, vector<VAL::parameter_symbol_list*> func_args_, vector<string> operator__) : agent_instance(agent_instance_), all_goals(all_goals_), initial_state_facts(initial_state_facts_), values(values_), func_name(func_name_), func_args(func_args_), operator_(operator__), priority(false), goalState(false){
            all_agent_names.insert(agent_instance->getName());
        }
        virtual ~SimpleAgent() {
        }

        static set<std::string> all_agent_names;

        vector<int> subproblem_goals;

        //getters
        string getName(){return name;};
        string getType(){return type->getName();};
        string getSuperType(){return type->type->getName();};
//        bool matchesType(string outer_type_string);
        vector<VAL::proposition*> getOutputFacts(){return agent_output_facts;};
        vector<VAL::parameter_symbol*> getOutputFactsAttributes(){return output_facts_attributes;};
        vector<string> getNonGoalInitialStateFacts(){return single_attribute_agent_facts;};
        bool isAvailable(){return available;};
        bool isPriority(){return priority;};
        bool isReused(){return reused;};
        bool hasGoalState(){return goalState;};

        //setters
        void setAvailable(bool available_){available = available_;};
        void setPriority(bool priority_){priority = priority_;};
        void setReused(bool reused_){reused = reused_;};
        void setSubproblemGoals(vector<int> subproblem_goals_){subproblem_goals = subproblem_goals_;};
        void processConstraints();
        void clear(){agent_output_facts.clear(); output_facts_attributes.clear();};

        //main
        void agentCaseAction(std::ostream & o);
        //void agentFinalisedGoals(std::ostream & o);
        bool isPropositiontStatic(VAL::proposition* fact);

    };



    class AgentGroup{
    private:
        vector<SimpleAgent*> agents;
        VAL::pddl_type* type;
        vector<VAL::pddl_type*> supertypes;
        const int priority;
        const bool is_parent;

    public:
        AgentGroup (VAL::pddl_type* type_, vector<VAL::pddl_type*> supertypes_, int priority_, bool is_parent_) : type(type_), supertypes(supertypes_), priority(priority_), is_parent(is_parent_){}
        virtual ~AgentGroup() {
        }

        //getters
        const int getPriority(){return priority;};
        int size(){return agents.size();};
        string getType(){return type->getName();};
        bool has_parent_parent_dynamic_type(){return is_parent;};

        //TODO eliminate getSuperType() string method and refactor all code to work with getSuperTypes() vector method
        vector<VAL::pddl_type*> getSuperTypes(){return supertypes;};
        string getSuperType(int index_){if (index_ < supertypes.size())return supertypes[index_]->getName();else return "";};

        //        bool matchesType(string outer_type_string);
        vector<SimpleAgent*> getAgents(){return agents;};
        int availableAgents(){
            int size = 0;
            for (int i = 0; i < agents.size(); i++){
                if(agents[i]->isAvailable())size++;
            }return size;};

        //setters
        void clear(){agents.clear();};

        //main
        void addSimpleAgent(SimpleAgent* agent_){agents.push_back(agent_);};
        int getRemainingPriorityAgents(){
            int priority_agents = 0;
            for (int i = 0; i < agents.size(); i++){
                if(agents[i]->isPriority())priority_agents++;
            }
            return priority_agents;
        };
        vector<string> getRemainingAvailableAgents(){
            vector<string> available_agents;
            for (int i = 0; i < agents.size(); i++){
                if(agents[i]->isAvailable())available_agents.push_back(agents[i]->getName());
            }
            return available_agents;
        };
        vector<string> getRemainingNotReUsedAgents(){
            vector<string> not_reused_agents;
            for (int i = 0; i < agents.size(); i++){
                if(!agents[i]->isReused())not_reused_agents.push_back(agents[i]->getName());
            }
            return not_reused_agents;
        };
        void makeAllAgentsAvailable(){
            for (int i = 0; i < agents.size(); i++){
                agents[i]->setAvailable(true);
            };
        };
        void makeAllAgentsNotReUsed(){
            for (int i = 0; i < agents.size(); i++){
                agents[i]->setReused(false);
            };
        };
    };
}




#endif //STP_SIMPLEAGENT_H
