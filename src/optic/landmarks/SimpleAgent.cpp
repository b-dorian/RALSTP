//
// Created by Dorian Buksz on 31/05/2020.
//

#include "SimpleAgent.h"


namespace strategic_tactical{

    void SimpleAgent::processConstraints(){


//        setNonGoalInitialStateFacts();
        setAgentInitialStateFacts();
        setAgentGoalFacts();
        setAgentFinalizedFacts();

        //setOutputFactsAttributes();
    }

    VAL::proposition* SimpleAgent::assign_types_to_proposition(VAL::proposition* proposition){
        VAL::parameter_symbol_list* new_args = new VAL::parameter_symbol_list;
        for (VAL::pred_decl_list::const_iterator pit = VAL::current_analysis->the_domain->predicates->begin(); pit != VAL::current_analysis->the_domain->predicates->end(); pit++) {
            VAL::pred_decl* predicate = *pit;
            if(predicate->getPred()->symbol::getName().compare(proposition->head->getName()) == 0){
                int position = 0;
                for (VAL::var_symbol_list::const_iterator sit = predicate->getArgs()->begin();
                     sit != predicate->getArgs()->end(); sit++) {
                    const VAL::var_symbol *var_symbol_instance = *sit;
                    VAL::parameter_symbol_list::iterator lit = proposition->args->begin();
                    advance(lit,position);
                    VAL::parameter_symbol* new_symbol = *lit;
                    new_symbol->type = var_symbol_instance->type;
                    new_args->push_back(new_symbol);
                    position++;
                }
            }
        }
        proposition->args = new_args;
        return proposition;
    }

//    void SimpleAgent::setNonGoalInitialStateFacts(){
//        for (int i = 0; i < initial_state_facts.size(); i++){
//            for (VAL::parameter_symbol_list::iterator pit = initial_state_facts[i]->args->begin();
//                 pit != initial_state_facts[i]->args->end(); pit++) {
//                VAL::parameter_symbol* parameter_symbol_instance = *pit;
//                if(name.compare(parameter_symbol_instance->getName()) == 0){
//                    if(initial_state_facts[i]->args->size() == 1){
//                        single_attribute_agent_facts.push_back(initial_state_facts[i]->head->getName());
//                    }
//
//                }
//            }
//        }
//        sort( single_attribute_agent_facts.begin(), single_attribute_agent_facts.end() );
//        single_attribute_agent_facts.erase( unique( single_attribute_agent_facts.begin(), single_attribute_agent_facts.end() ), single_attribute_agent_facts.end() );
//    }



    bool SimpleAgent::setAgentGoalFacts(){
        for (int i = 0; i < all_goals.size(); i++){
            bool in_proposition = false;
            for (VAL::parameter_symbol_list::iterator pit = all_goals[i]->args->begin();
                 pit != all_goals[i]->args->end(); pit++) {
                VAL::parameter_symbol* parameter_symbol_instance = *pit;
                if(name.compare(parameter_symbol_instance->getName()) == 0){
                    in_proposition = true;
                    break;
                }
            }
            if(in_proposition && !isPropositiontStatic(all_goals[i]))agent_output_facts.push_back(all_goals[i]);
        }
        if(agent_output_facts.size() == 0){
            if(!initialized){
                priority = false;
                goalState = false;
                initialized = true;
            }

            return false;
        }
        if(!initialized){
            priority = true;
            goalState = true;
            initialized = true;
        }
        return true;
    }

    void SimpleAgent::setAgentInitialStateFacts(){
        for (int i = 0; i < initial_state_facts.size(); i++){
            bool in_proposition = false;
            for (VAL::parameter_symbol_list::iterator pit = initial_state_facts[i]->args->begin();
                 pit != initial_state_facts[i]->args->end(); pit++) {
                VAL::parameter_symbol* parameter_symbol_instance = *pit;
                if(name.compare(parameter_symbol_instance->getName()) == 0){
                    in_proposition = true;
                    break;
                }
            }
            if(in_proposition && !isPropositiontStatic(initial_state_facts[i]))agent_initial_facts.push_back(initial_state_facts[i]);

        }
    }

    void SimpleAgent::setAgentFinalizedFacts(){
        for (int i = 0; i < agent_initial_facts.size(); i++){
            bool isOutput = false;
            for (int j = 0; j < agent_output_facts.size(); j++){
                if (agent_initial_facts[i]->head->getName().compare(agent_output_facts[j]->head->getName()) == 0){
                    agent_finalized_facts.push_back(agent_output_facts[j]);
                    isOutput = true;
                    break;
                }
            }
            if(!isOutput){
                agent_finalized_facts.push_back(agent_initial_facts[i]);
            }
        }
    }

    //TODO check if attribute already in
//    void SimpleAgent::setOutputFactsAttributes(){
//        for (int i = 0; i < agent_output_facts.size(); i++){
//            int agent_position = 0;
//            VAL::proposition* typed_proposition = assign_types_to_proposition(agent_output_facts[i]);
//
//
//
//
//            //check if facts contains agent
//            for (VAL::parameter_symbol_list::iterator pit1 = typed_proposition->args->begin();
//                 pit1 != typed_proposition->args->end(); pit1++) {
//                agent_position++;
//                VAL::parameter_symbol* parameter_symbol_instance1 = *pit1;
//                if(name.compare(parameter_symbol_instance1->getName()) == 0){
//
//                    //ignore fact if static
//                    //if(isPropositiontStatic(typed_proposition))break;
//
//                    //push back all single attribute agent predicates
//                    if(typed_proposition->args->size() == 1){
//                        stringstream ss;
//                        ss << typed_proposition->head->getName() << " " << parameter_symbol_instance1->getName();
//                        single_attribute_agent_facts.push_back(ss.str());
//                    }
//                    else{
//                        //push back all non-agent attributes if fact not static
//                        for (VAL::parameter_symbol_list::iterator pit2 = typed_proposition->args->begin();
//                             pit2 != typed_proposition->args->end(); pit2++) {
//                            VAL::parameter_symbol* parameter_symbol_instance2 = *pit2;
//                            if(name.compare(parameter_symbol_instance2->getName()) != 0){
//                                agent_output_facts.push_back(parameter_symbol_instance2);
//                            }
//                        }
//                    }
//                }
//            }
//        }
//    }

    //check if fact is not static
    bool SimpleAgent::isPropositiontStatic(VAL::proposition* prop){
        bool static_ = true;

        //iterate through all domain operators
        for (VAL::operator_list::iterator oit = VAL::current_analysis->the_domain->ops->begin(); oit != VAL::current_analysis->the_domain->ops->end(); oit++) {
            if(!static_)break;
            VAL::operator_ *operator_ = *oit;
            last_operator_propositions.clear();
            operator_->effects->visit(this);

            //iterate through all operator effects
            for(int i = 0; i < last_operator_propositions.size(); i++){
                if (last_operator_propositions[i]->head->getName().compare(prop->head->getName()) == 0){
                    static_ = false;
                    break;
                }
            }
        }
        return static_;
    }

    void SimpleAgent::visit_effect_lists(VAL::effect_lists *e) {
        cout << "agent visit_effect_lists" << endl;
        //last_effect = false;
        e->add_effects.pc_list<VAL::simple_effect*>::visit(this);

        //last_effect = true;
        e->del_effects.pc_list<VAL::simple_effect*>::visit(this);
        //last_effect = false;

        e->forall_effects.pc_list<VAL::forall_effect*>::visit(this);
        e->cond_effects.pc_list<VAL::cond_effect*>::visit(this);
        e->cond_assign_effects.pc_list<VAL::cond_effect*>::visit(this);
        e->assign_effects.pc_list<VAL::assignment*>::visit(this);
        e->timed_effects.pc_list<VAL::timed_effect*>::visit(this);
    }

    void SimpleAgent::visit_timed_effect(VAL::timed_effect *e){
        cout << "agent visit_timed_effect" << endl;
        e->effs->visit(this);
    }

    void SimpleAgent::visit_simple_effect(VAL::simple_effect *e) {
        //cout << "visit_simple_effect" << " " << last_effect << " " << e->prop->head->getName() << endl;
        cout << "agent visit_simple_effect" << endl;
        last_operator_propositions.push_back(const_cast<VAL::proposition*>(e->prop));
        //    last_negative_effects.push_back(last_effect);
    }


//    void SimpleAgent::agentFinalisedGoals(std::ostream & o){
//        sort( agent_finalized_facts.begin(), agent_finalized_facts.end() );
//        agent_finalized_facts.erase( unique( agent_finalized_facts.begin(), agent_finalized_facts.end() ), agent_finalized_facts.end() );
//        // stp_agent facts from final state
//        for (int i = 0; i < agent_finalized_facts.size(); i++){
//            stringstream prop_ss;
//            for (VAL::parameter_symbol_list::iterator pit = agent_finalized_facts[i]->args->begin();
//                 pit != agent_finalized_facts[i]->args->end(); pit++) {
//                VAL::parameter_symbol *parameter_symbol_instance = *pit;
//                prop_ss << " " << parameter_symbol_instance->getName();
//            }
//            o << "      (" << agent_finalized_facts[i]->head->getName() << prop_ss.str() << ")" << endl;
//        }
//    }


    void SimpleAgent::agentCaseAction(std::ostream & o){

        /*
         * Durative Action Version
        */

        o << "(:durative-action stp_case_" << name << endl;
        o << "  :parameters()" << endl;
        o << "  :duration(= ?duration 0)" << endl;
        o << "  :condition\n    (and" << endl;

        // add dead-end agent goals
        for (int i = 0; i < subproblem_goals.size(); ++i){
            stringstream prop_ss;
            for (VAL::parameter_symbol_list::iterator git = all_goals[subproblem_goals[i]]->args->begin();
                 git != all_goals[subproblem_goals[i]]->args->end(); git++) {
                VAL::parameter_symbol *parameter_symbol_instance = *git;
                prop_ss << " " << parameter_symbol_instance->getName();
            }
            o << "      (at start (" << all_goals[subproblem_goals[i]]->head->getName() << prop_ss.str() << "))" << endl;
        }

        // stp_selected_agentType agentName
        o << "      (at start (stp_selected_" << type->getName() << " " << name << "))" << endl;



//        sort( agent_finalized_facts.begin(), agent_finalized_facts.end() );
//        agent_finalized_facts.erase( unique( agent_finalized_facts.begin(), agent_finalized_facts.end() ), agent_finalized_facts.end() );
//        for (int i = 0; i < agent_finalized_facts.size(); i++){
//            stringstream prop_ss;
//            for (VAL::parameter_symbol_list::iterator pit = agent_finalized_facts[i]->args->begin();
//                 pit != agent_finalized_facts[i]->args->end(); pit++) {
//                VAL::parameter_symbol *parameter_symbol_instance = *pit;
//                prop_ss << " " << parameter_symbol_instance->getName();
//            }
//            o << "      (at start (" << agent_finalized_facts[i]->head->getName() << prop_ss.str() << "))" << endl;
//        }

        //





        if(!priority){
            for (int i = 0; i < agent_initial_facts.size(); i++){
                stringstream prop_ss;
                for (VAL::parameter_symbol_list::iterator pit = agent_initial_facts[i]->args->begin();
                     pit != agent_initial_facts[i]->args->end(); pit++) {
                    VAL::parameter_symbol *parameter_symbol_instance = *pit;
                    prop_ss << " " << parameter_symbol_instance->getName();
                }
                o << "      (at start (" << agent_initial_facts[i]->head->getName() << prop_ss.str() << "))" << endl;
            }

            //add initial state functions
            //TODO add finalise functions ??
            for (int j = 0; j < func_name.size(); j++) {
                stringstream func_ss;
                func_ss << "      (at start (" << operator_[j] << " (" << func_name[j];
                bool irelevant = false;
                for (VAL::parameter_symbol_list::iterator fit = func_args[j]->begin();
                     fit != func_args[j]->end(); fit++) {
                    VAL::parameter_symbol *parameter_symbol_instance = *fit;
                    func_ss << " " << parameter_symbol_instance->getName();
                }
                func_ss << ") " << values[j] << "))" << endl;
                if(func_ss.str().find(name) != std::string::npos){
                    o << func_ss.str();
                }
            }
        }
        else{
            for (int i = 0; i < agent_output_facts.size(); i++){
                stringstream prop_ss;
                for (VAL::parameter_symbol_list::iterator pit = agent_output_facts[i]->args->begin();
                     pit != agent_output_facts[i]->args->end(); pit++) {
                    VAL::parameter_symbol *parameter_symbol_instance = *pit;
                    prop_ss << " " << parameter_symbol_instance->getName();
                }
                o << "      (at start (" << agent_output_facts[i]->head->getName() << prop_ss.str() << "))" << endl;
            }
            // add function goals
        }



        //TODO functions from initial state or final state

//        for (set<string>::iterator sit = all_agent_names.begin(); sit != all_agent_names.end(); sit++) {
//            if((name).compare(*sit) == 0)continue;
//            for (int i = 0; i < initial_state_facts.size(); i++){
//                bool in_proposition = false;
//                for (VAL::parameter_symbol_list::iterator pit = initial_state_facts[i]->args->begin();
//                     pit != initial_state_facts[i]->args->end(); pit++) {
//                    VAL::parameter_symbol* parameter_symbol_instance = *pit;
//                    if((*sit).compare(parameter_symbol_instance->getName()) == 0){
//                        in_proposition = true;
//                        break;
//                    }
//                }
//                if(in_proposition && !isPropositiontStatic(initial_state_facts[i])){
//                    stringstream prop_ss;
//                    for (VAL::parameter_symbol_list::iterator pit = initial_state_facts[i]->args->begin();
//                         pit != initial_state_facts[i]->args->end(); pit++) {
//                        VAL::parameter_symbol *parameter_symbol_instance = *pit;
//                        prop_ss << " " << parameter_symbol_instance->getName();
//                    }
//                    o << "      (" << initial_state_facts[i]->head->getName() << prop_ss.str() << ")" << endl;
//                }
//            }
//        }


        o << "    )" << endl;


        o << "  :effect\n    (and" << endl;


        // disable agent
        o << "      (at start (not (stp_selected_" << type->getName() << " " << name << ")))" << endl;

        // stp_agent_complete
        o << "      (at start (stp_" << type->getName() << "_complete))" << endl;



        o << "    )" << endl;
        o << "  )" << endl << endl << endl;


        /*
         * Classical Action Version
        */

//        o << "(:action stp_case_" << name << endl;
//        o << "  :parameters()" << endl;
//        o << "  :precondition\n    (and" << endl;
//
//        // dead-end agent subproblem goals
//        for (int i = 0; i < subproblem_goals.size(); ++i){
//            stringstream prop_ss;
//            for (VAL::parameter_symbol_list::iterator git = all_goals[subproblem_goals[i]]->args->begin();
//                 git != all_goals[subproblem_goals[i]]->args->end(); git++) {
//                VAL::parameter_symbol *parameter_symbol_instance = *git;
//                prop_ss << " " << parameter_symbol_instance->getName();
//            }
//            o << "      (" << all_goals[subproblem_goals[i]]->head->getName() << prop_ss.str() << ")" << endl;
//        }
//
//        // stp_selected_agentType agentName
//        o << "      (stp_selected_" << type->getName() << " " << name << ")" << endl;
//
//        // stp_agent facts from initial state
////        for (int i = 0; i < agent_initial_facts.size(); i++){
////            stringstream prop_ss;
////            for (VAL::parameter_symbol_list::iterator pit = agent_initial_facts[i]->args->begin();
////                 pit != agent_initial_facts[i]->args->end(); pit++) {
////                VAL::parameter_symbol *parameter_symbol_instance = *pit;
////                prop_ss << " " << parameter_symbol_instance->getName();
////            }
////            o << "      (" << agent_initial_facts[i]->head->getName() << prop_ss.str() << ")" << endl;
////        }
//
//
//        sort( agent_finalized_facts.begin(), agent_finalized_facts.end() );
//        agent_finalized_facts.erase( unique( agent_finalized_facts.begin(), agent_finalized_facts.end() ), agent_finalized_facts.end() );
//        // selected agent parent agent goals
//        for (int i = 0; i < agent_finalized_facts.size(); i++){
//            stringstream prop_ss;
//            for (VAL::parameter_symbol_list::iterator pit = agent_finalized_facts[i]->args->begin();
//                 pit != agent_finalized_facts[i]->args->end(); pit++) {
//                VAL::parameter_symbol *parameter_symbol_instance = *pit;
//                prop_ss << " " << parameter_symbol_instance->getName();
//            }
//            o << "      (" << agent_finalized_facts[i]->head->getName() << prop_ss.str() << ")" << endl;
//        }
//
////        for (set<string>::iterator sit = all_agent_names.begin(); sit != all_agent_names.end(); sit++) {
////            if((name).compare(*sit) == 0)continue;
////            for (int i = 0; i < initial_state_facts.size(); i++){
////                bool in_proposition = false;
////                for (VAL::parameter_symbol_list::iterator pit = initial_state_facts[i]->args->begin();
////                     pit != initial_state_facts[i]->args->end(); pit++) {
////                    VAL::parameter_symbol* parameter_symbol_instance = *pit;
////                    if((*sit).compare(parameter_symbol_instance->getName()) == 0){
////                        in_proposition = true;
////                        break;
////                    }
////                }
////                if(in_proposition && !isPropositiontStatic(initial_state_facts[i])){
////                    stringstream prop_ss;
////                    for (VAL::parameter_symbol_list::iterator pit = initial_state_facts[i]->args->begin();
////                         pit != initial_state_facts[i]->args->end(); pit++) {
////                        VAL::parameter_symbol *parameter_symbol_instance = *pit;
////                        prop_ss << " " << parameter_symbol_instance->getName();
////                    }
////                    o << "      (" << initial_state_facts[i]->head->getName() << prop_ss.str() << ")" << endl;
////                }
////            }
////        }
//
//        o << "    )" << endl;
//
//
//        o << "  :effect\n    (and" << endl;
//
//
//        // disable agent
//        o << "      (not (stp_selected_" << type->getName() << " " << name << "))" << endl;
//
//        // stp_agent_complete
//        o << "      (stp_" << type->getName() << "_complete)" << endl;
//
//
//
//        o << "    )" << endl;
//        o << "  )" << endl << endl << endl;




    }

//    bool SimpleAgent::matchesType(string outer_type_string){
//        VAL::pddl_type* current_type = type;
//        do{
//            if (current_type->getName().compare(outer_type_string) == 0)return true;
//            current_type = current_type->type;
//        }
//        while (current_type->type != NULL);
//        return false;
//    }
//
//    //TODO change file name - fix matchesType
//    bool AgentGroup::matchesType(string outer_type_string){
//        VAL::pddl_type* current_type = type;
//        do{
//            if (current_type->getName().compare(outer_type_string) == 0)return true;
//            current_type = current_type->type;
//        }
//        while (current_type->type != NULL);
//        return false;
//    }



}