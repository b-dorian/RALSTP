/*
 * TemporalLandmarksAndConstraints.cpp
 *
 *  Created on: Apr 10, 2014
 *      Author: karpase
 */

#include "TemporalLandmarksAndConstraints.h"
#include "RPGBuilder.h"
#include "TemporalActionLandmark.h"
#include "TemporalCompleteActionLandmark.h"
#include "TemporalStateLandmark.h"
#include "SimpleTemporalConstraint.h"
#include "DerivationRuleAchieverOfFormula.h"
#include "DerivationRuleFirstAchieverOfFormula.h"
#include "DerivationRuleTEventCondition.h"
#include "DerivationRuleTEventEffect.h"
#include "DerivationRuleActionStartEnd.h"
#include "solver.h"
#include <thread>


#include <cstdio>
#include <iostream>
#include <algorithm>
#include <map>

using namespace Planner;

using namespace std;

namespace temporal_landmarks {

    TemporalLandmarksAndConstraints::TemporalLandmarksAndConstraints():
            start_time_point(tpv_factory.getNewTimepointVariable()),
            end_time_point(tpv_factory.getNewTimepointVariable())
    {
        mark_as_used(start_time_point);
        mark_as_used(end_time_point);

        derivation_rules.push_back(new DerivationRuleActionStartEnd(*this));
        derivation_rules.push_back(new DerivationRuleTEventCondition(*this));
        derivation_rules.push_back(new DerivationRuleTEventEffect(*this));
        derivation_rules.push_back(new DerivationRuleAchieverOfFormula(*this));
        derivation_rules.push_back(new DerivationRuleFirstAchieverOfFormula(*this));

        unique_landmarks_only = Globals::unique_landmarks_only;

        solver = getNewSolver();
        vector<pair<int,double> > emptyEntries;
        solver->addCol(emptyEntries, 0, 0, MILPSolver::C_REAL);
        solver->addCol(emptyEntries, 0, solver->getInfinity(), MILPSolver::C_REAL);
        solver->hush();

//	const map<Literal*, set<pair<int, Planner::time_spec> > > &fa =  RPGBuilder::getHeuristic()->get_fact_achievers();
//	for (map<Literal*, set<pair<int, Planner::time_spec> > >::const_iterator it = fa.begin();
//			it != fa.end(); ++it) {
//		const Literal* lit = it->first;
////		lit->write(std::cout);
////		std::cout << std::endl;
//		for (set<pair<int, Planner::time_spec> >::const_iterator sit = it->second.begin(); sit != it->second.end(); ++sit) {
//			TEvent event(sit->first, sit->second);
////			std::cout << "\t";
////			event.write(std::cout);
////			std::cout << std::endl;
//		}
//	}
    }

    TemporalLandmarksAndConstraints::~TemporalLandmarksAndConstraints() {
    }

    void TemporalLandmarksAndConstraints::add_temporal_constraint(SimpleTemporalConstraint constraint, TemporalLandmark *lm) {
        vector<pair<int,double> > emptyEntries;
        if (is_used(constraint.t1) && is_used(constraint.t2)) {
            constraints.insert(constraint);
            lm->constraints.insert(constraint);
//        constraint.t1.lb = 0;

            for (int i = solver->getNumCols(); i < tpv_factory.get_last_id(); i++) {
                solver->addCol(emptyEntries, 0, solver->getInfinity(), MILPSolver::C_REAL);
            }

            vector<pair<int,double> > entries;
            entries.push_back(make_pair(constraint.t1.getID(), 1.0));
            entries.push_back(make_pair(constraint.t2.getID(), -1.0));
            double lb = -solver->getInfinity();
            double ub = solver->getInfinity();
            if ((constraint.lb != EpsilonResolutionTimestamp::undefined()) && (constraint.lb > -EpsilonResolutionTimestamp::infinite())) {
                lb = constraint.lb.toDouble();
            }
            if ((constraint.ub != EpsilonResolutionTimestamp::undefined()) && (constraint.ub < EpsilonResolutionTimestamp::infinite())) {
                ub = constraint.ub.toDouble();
            }
            solver->addRow(entries, lb, ub);
            //if(lb>=0)std::cout << "Constraint " << constraints.size() << ": " << constraint.t1.getID() << " & " << constraint.t2.getID() << ": " << lb << " - " << ub << "\n";
        }
    }

    void TemporalLandmarksAndConstraints::output_timpepoints() {

        // partial order 0-infinitiy timepoints
        // find usefullness of lb <= t1 - t2
        // find usefullness of lb <= t1 - t2 <= ub
        // assign landmakrs to timepoints

        int currentId = 0;
        for (auto time_point : used_time_points) {
            if (!get_lower_bound(time_point).toDouble()>=0) {

                std::cout << "t" << time_point.getID() << " lb: " << get_lower_bound(time_point) << " ub: " << get_upper_bound(time_point)<< "\n";

            }
        }
    }

    void TemporalLandmarksAndConstraints::generate_happenings(std::ostream & o) {
        for (auto time_point : used_time_points) {
            if (!get_lower_bound(time_point).toDouble()>=0) {
                happenings.push_back(get_lower_bound(time_point).toDouble());
            }
        }
        sort( happenings.begin(), happenings.end() );
        happenings.erase( unique( happenings.begin(), happenings.end() ), happenings.end() );

        //order by lower bound
        std::vector<TemporalLandmark *> lb_ordered_landmarks;
        for (int i = 0; i < happenings.size(); i++) {
            //std::cout << happenings[i] << " - ";
//            o << "Happening: " << happenings[i] << "\n\n";
//            for (size_t j = 0; j < landmarks.size(); j++) {
//                if (landmarks[j]->get_num_timepoint_variables() == 1) {
////                    if (happenings[i] == get_lower_bound(landmarks[j + 1]->get_timepoint_variables(0)).toDouble()) {
////                        TemporalCompleteActionLandmark *complete_action_landmark = new TemporalCompleteActionLandmark(
////                                landmarks[j + 1]->get_events_non_const(), landmarks[j]->get_events_non_const(),
////                                landmarks[j + 1]->get_timepoint_variables(0), landmarks[j]->get_timepoint_variables(0));
////                        complete_action_landmark->id = landmarks[j]->id;
////                        complete_action_landmark->type = "ACTION";
////                        lb_ordered_landmarks.push_back(complete_action_landmark);
////
////                    }
////                    j++;
//                } else {
//                    if (happenings[i] == get_lower_bound(landmarks[j]->get_timepoint_variables(0)).toDouble()) {
//                        lb_ordered_landmarks.push_back(landmarks[j]);
//                        std::pair<double,double> bounds;
//                        bounds.first = get_lower_bound(landmarks[j]->get_timepoint_variables(0)).toDouble();
//                        bounds.second = get_lower_bound(landmarks[j]->get_timepoint_variables(1)).toDouble();
//                        landmarks[j]->write(o, bounds);
//                        o << "\n\n";
//                    }
//                }
//            }
        }
//        std::cout << "\n\nLower bound order:\n\n";
//        std::pair<double,double> bounds;
//        for (int i = 0; i < lb_ordered_landmarks.size(); i++) {
//            if(lb_ordered_landmarks[i]->type.length() == 4){
//                bounds.first = get_lower_bound(lb_ordered_landmarks[i]->get_timepoint_variables(0)).toDouble();
//                bounds.second = get_lower_bound(lb_ordered_landmarks[i]->get_timepoint_variables(1)).toDouble();
//                lb_ordered_landmarks[i]->write(o, bounds);
//                o << "\n\n";
//            }
//        }
//        for (int i = 0; i < lb_ordered_landmarks.size(); i++) {
//            if(lb_ordered_landmarks[i]->type.length() > 4)std::cout << "LM-" << lb_ordered_landmarks[i]->id << " " << lb_ordered_landmarks[i]->type << " " << get_lower_bound(lb_ordered_landmarks[i]->get_timepoint_variables(0)) << " " << get_lower_bound(lb_ordered_landmarks[i]->get_timepoint_variables(1)) << "\n";
//        }
        //order from lower to lower + upper bound
//        std::vector<TemporalLandmark *> helper_landmarks;
//        for (int j = 0; j < lb_ordered_landmarks.size(); j++) {
//            if(j<lb_ordered_landmarks.size()-1) {
//                if(get_lower_bound(lb_ordered_landmarks[j]->get_timepoint_variables(0))==get_lower_bound(lb_ordered_landmarks[j+1]->get_timepoint_variables(0))){
//                    helper_landmarks.push_back(lb_ordered_landmarks[j]);
//                    helper_landmarks.push_back(lb_ordered_landmarks[j+1]);
//                    ++j;
//                }
//                else{
//                    helper_landmarks.push_back(lb_ordered_landmarks[j]);
//
//                    //bubble sort (to modify to merge sort)
//                    bool sorted = false;
//                    TemporalLandmark *temp;
//                    int count = 0;
//                    while(!sorted){
//                        sorted = true;
//                        for(int k = 0; k < helper_landmarks.size()-1; k++){
//                            if(get_lower_bound(helper_landmarks[k]->get_timepoint_variables(1)).toDouble() < get_lower_bound(helper_landmarks[k+1]->get_timepoint_variables(1)).toDouble()){
//                                temp = helper_landmarks[k];
//                                helper_landmarks[k] = helper_landmarks[k+1];
//                                helper_landmarks[k+1] = temp;
//                                sorted = false;
//                                std::cout << "for" << get_lower_bound(helper_landmarks[k]->get_timepoint_variables(0)).toDouble() << "\n";
//                            }
//                        }
//                        std::cout << "while" << count << "\n";
//                        count++;
//                    }
//                    for(int k = 0; k < helper_landmarks.size(); k++){
//                        ordered_landmarks.push_back(helper_landmarks[k]);
//                        std::cout << "out ub\n";
//                    }
//                }
//            }
//      }

//        std::cout << "Lower + upper bounds order:\n";
//        for (int i = 0; i < ordered_landmarks.size(); i++) {
//            std::cout << "LM-" << ordered_landmarks[i]->id << " " << ordered_landmarks[i]->type << " " << get_lower_bound(ordered_landmarks[i]->get_timepoint_variables(0)) << " " << get_lower_bound(ordered_landmarks[i]->get_timepoint_variables(1)) << "\n";
//        }
//        o << "Nr of FACT landmarks: " << lb_ordered_landmarks.size() << endl;
    }


    bool TemporalLandmarksAndConstraints::is_not_subsumed(TemporalLandmark *lm) {
        for (size_t i = 0; i < landmarks.size(); i++) {
            if ((lm->subsumed_by(landmarks[i])) || (landmarks[i]->subsumed_by(lm))) {
            //if ((lm->subsumed_by(landmarks[i])) ) {
//			lm->write(cout);
//			cout << " is not unique, subsumed by ";
//			landmarks[i]->write(cout);
//			cout << endl;
                return false;
            }
        }
        return true;
    }

    void TemporalLandmarksAndConstraints::add_temporal_landmark(TemporalLandmark *lm) {
        lm->id = lm_id++;
        landmarks.push_back(lm);
        for (size_t i = 0; i < lm->get_num_timepoint_variables(); i++) {
            mark_as_used(lm->get_timepoint_variables(i));
            assign_to_timepoint(lm->get_timepoint_variables(i).getID(),lm->id);

            if (lm->get_timepoint_variables(i) != get_end_timepoint_variable()) {
                add_temporal_constraint(
                        SimpleTemporalConstraint(get_end_timepoint_variable(), lm->get_timepoint_variables(i),
                                                 EpsilonResolutionTimestamp::zero(),
                                                 EpsilonResolutionTimestamp::infinite()), lm);
                add_temporal_constraint(
                        SimpleTemporalConstraint(lm->get_timepoint_variables(i), get_start_timepoint_variable(),
                                                 (Globals::use_trpg_bounds ?
                                                  lm->get_earliest() :
                                                  EpsilonResolutionTimestamp::zero()),
                                                 EpsilonResolutionTimestamp::infinite()), lm);
            }
        }
    }

    void TemporalLandmarksAndConstraints::backchain_from(TemporalLandmark *lm, vector<TemporalLandmark *> chain) {
        //for (size_t kkk = 0; kkk < chain.size(); kkk++) {std::cout << " ";}
        std::pair<double,double> bounds;
        bounds.first = 0;
        bounds.second = 0;
        //std::cout << "Backchain from: ";
        //lm->write(std::cout, bounds);
        //std::cout << std::endl;
        for (size_t i = 0; i < derivation_rules.size(); i++) {
            const std::chrono::time_point<std::chrono::system_clock> generate_lm_current_time = std::chrono::system_clock::now();
            const std::chrono::duration<double> generate_lm_duration = generate_lm_current_time - generate_lm_start;
            int max_search_time = 450;
            if (generate_lm_duration.count() > max_search_time ) {
                cout << endl << "Backchaining operation out of time. Allowed search time:  " << max_search_time << endl << endl;
                return;}
            //else {cout << endl << "Landmarks extraction duration: " << generate_lm_duration.count() << endl << endl;}

            //for (size_t kkk = 0; kkk < chain.size(); kkk++) {std::cout << " ";}
            //std::cout << "Rule " << i;

            std::vector<TemporalLandmark *> new_lms;
            set<SimpleTemporalConstraint> new_constraints;
            vector<TemporalLandmark *> really_new_lms;
            int pre_size = new_lms.size();
            bool something_new = derivation_rules[i]->apply_rule(lm, new_lms, new_constraints);

            //std::cout << "  " << new_lms.size() << " / " << new_constraints.size() << "  -  " << something_new << std::endl;

            if (new_lms.size() > pre_size) {
                for (size_t j = 0; j < new_lms.size(); j++) {
                    bool is_new = true;
                    for (size_t k = 0; k < chain.size(); k++) {
                        if (new_lms[j]->subsumed_by(chain[k]) || chain[k]->subsumed_by(new_lms[j]) ) {
                        //if (new_lms[j]->subsumed_by(chain[k])  ) {
                            //for (size_t kkk = 0; kkk < chain.size(); kkk++) std::cout << " ";
                            //new_lms[j]->write(std::cout);
                            //std::cout << "  is not new" << std::endl;
                            is_new = false;
                            break;
                        }
                        if (unique_landmarks_only && !is_not_subsumed(new_lms[j])) {
                            is_new = false;
                            break;
                        }
                    }
                    if (is_new) {
                        really_new_lms.push_back(new_lms[j]);
                    }
                }

                for (size_t j = 0; j < really_new_lms.size(); j++) {
                    really_new_lms[j]->parentLandmark = lm;
                    really_new_lms[j]->rootLandmark = lm->rootLandmark;
                    really_new_lms[j]->rootGoal = lm->rootGoal;
                    switch (i) {
                        case 0:{ really_new_lms[j]->rule = "0 - DerivationRuleActionStartEnd"; break;}
                        case 1:{ really_new_lms[j]->rule = "1 - DerivationRuleTEventCondition"; break;}
                        case 2:{ really_new_lms[j]->rule = "2 - DerivationRuleTEventEffect"; break;}
                        case 3:{ really_new_lms[j]->rule = "3 - DerivationRuleAchieverOfFormula"; break;}
                        case 4:{ really_new_lms[j]->rule = "4 - DerivationRuleFirstAchieverOfFormula"; break;}
                    }
                    add_temporal_landmark(really_new_lms[j]);
                }
                for (set<SimpleTemporalConstraint>::const_iterator it = new_constraints.begin();
                     it != new_constraints.end(); ++it) {
                    add_temporal_constraint(*it,lm);
                }
                for (size_t j = 0; j < really_new_lms.size(); j++) {
#ifndef NDEBUG
                    size_t old_size = chain.size();
#endif
                    chain.push_back(really_new_lms[j]);
                    backchain_from(really_new_lms[j], chain);
                    chain.pop_back();
#ifndef NDEBUG
                    assert(old_size == chain.size());
#endif
                }

            }
        }
    }

    bool TemporalLandmarksAndConstraints::isParentAgentGoal(std::string goal, vector<RPGBuilder::agent> agents_structure) {
        cout << "checking: " << goal << endl;
        stringstream ss;
        for (int i = 0; i < agents_structure.size(); i++){
            ss << agents_structure[i].type << " ";
            cout << agents_structure[i].type << " " << agents_structure[i].is_parent << endl;
            if((goal.find(" "+agents_structure[i].type, 0) != std::string::npos) && (agents_structure[i].is_parent == true)){
                cout << " is agent of type: " << agents_structure[i].type << endl;
                return true;
            }
        }
        cout << " is NOT agent of types: " << ss.str() << endl;
        return false;
    }

    void TemporalLandmarksAndConstraints::generate_landmarks(string node) {
        generate_lm_start = std::chrono::system_clock::now();
        cout << "Generating Temporal Landmarks" << endl;

        list<Literal*>::const_iterator gsItr = RPGBuilder::getLiteralGoals().begin();
        const list<Literal*>::iterator gsEnd = RPGBuilder::getLiteralGoals().end();

        list<double>::const_iterator dlItr = RPGBuilder::getLiteralGoalDeadlines().begin();
        const list<double>::const_iterator dlEnd = RPGBuilder::getLiteralGoalDeadlines().end();


        int goal_lm_count = 0;
        int old_fact_lm_size = 0;
        int old_landmarks_size = 0;
        std::vector<TemporalLandmark *> all_landmarks;

        //extracting parent agents
//        vector<pair<int,string>> parent_agents_structure;
//        vector<pair<int,string>> dead_end_agents_structure;
//        int target_priority = agents_structure[agents_structure.size()-1].first;
//        for (int i = 0; i < agents_structure.size(); i++){
//            if(agents_structure[i].first == target_priority) {
//                cout << agents_structure[i].second << " popped" << endl;
//                dead_end_agents_structure.push_back(agents_structure[i]);
//                continue;
//            }
//            parent_agents_structure.push_back(agents_structure[i]);
//        }


        while (gsItr != gsEnd) {
            assert(dlItr != dlEnd);
            TimePointVariable goal_literal_ach_time(tpv_factory.getNewTimepointVariable());

            TemporalStateLandmark *goal_lm = new TemporalStateLandmark(new SingleLiteralFormula(*gsItr),
                                                                       goal_literal_ach_time, get_end_timepoint_variable());

            stringstream ss;
            goal_lm->formula->output_to_stringstream(ss);
            if(isParentAgentGoal(ss.str(), agents_structure)){
                cout << "Landmark is parent agent goal: " << ss.str() << endl;
                ++gsItr;
                ++dlItr;
                continue;
            }
            cout << "   Processing: " << ss.str() << endl;


            goal_lm->type=1;
            goal_lm->rootGoal=goal_lm_count;
            goal_lm->parentLandmark=goal_lm;
            goal_lm->rootLandmark=goal_lm;
            goal_lm->rule=10;
            add_temporal_landmark(goal_lm);

            double deadline = *dlItr;
            if (deadline < std::numeric_limits<double>::max()) {
                add_temporal_constraint(SimpleTemporalConstraint::is_less_than(goal_literal_ach_time, deadline), goal_lm);
            }

            vector<TemporalLandmark *>chain;
            backchain_from(goal_lm, chain);
            ++gsItr;
            ++dlItr;


            int fact_lm_size = 0;
            for (int i = 0; i < landmarks.size(); i++){
                if(landmarks[i]->get_num_timepoint_variables() == 2){
                    ++fact_lm_size;
                }
                landmarks_map[goal_lm_count].push_back(landmarks[i]);
                all_landmarks.push_back(landmarks[i]);
            }

//            with push initial state fact
//            for (int i = 1; i < landmarks.size(); i++){
//                if(landmarks[i]->get_num_timepoint_variables() == 2){
//                    if ((landmarks[0]->subsumed_by(landmarks[i])) && (landmarks[i]->subsumed_by(landmarks[0]))) { continue;}
//                    ++fact_lm_size;
//                }
//                    landmarks_map[goal_lm_count].push_back(landmarks[i]);
//                    all_landmarks.push_back(landmarks[i]);
//            }


            //add initial state landmarks
            LiteralSet state;
            vector<double> fluents;
            RPGBuilder::getInitialState(state, fluents);
            const SingleLiteralFormula* current_goal_formula = dynamic_cast<SingleLiteralFormula*>(goal_lm->formula);


            for (LiteralSet::const_iterator it = state.begin(); it != state.end(); ++it) {
                //cout << "initial state search start" << endl;
                SingleLiteralFormula initial_state_fact = *it;
                //current_goal_formula->write(cout);
                const Literal* const_literal_goal = current_goal_formula->get_literal();
                Literal* literal_goal = const_cast<Literal*>(const_literal_goal);
                const Literal* const_literal_fact = initial_state_fact.get_literal();
                Literal* literal_fact = const_cast<Literal*>(const_literal_fact);
                //cout << literal_goal->getHead()->getName() << " if1 " << literal_fact->getHead()->getName() << endl;
                if (literal_goal->getHead()->getName().compare(literal_fact->getHead()->getName()) == 0){
                    //cout << literal_goal->getHead()->getName() << " yes1 " << literal_fact->getHead()->getName() << endl;

                    VAL::LiteralParameterIterator<VAL::parameter_symbol_list::iterator> pit = literal_goal->begin();
                    while (pit != literal_goal->end()){
                        VAL::parameter_symbol* goal_arg = *pit;
                        for (int q = 0; q < agents_structure.size(); q++){

                            //cout << goal_arg->getName() << " if2 " << agents_structure[q].type << endl;
                            if((goal_arg->getName().find(agents_structure[q].type, 0) != std::string::npos) && (agents_structure[q].is_parent == false)){
                                string target_agent = goal_arg->getName();
                                //cout << target_agent << " yes2 " << agents_structure[q].type << endl;

                                VAL::LiteralParameterIterator<VAL::parameter_symbol_list::iterator> pit2 = literal_fact->begin();
                                while (pit2 != literal_fact->end()){
                                    VAL::parameter_symbol* fact_arg = *pit2;
                                    //cout << target_agent << " if3 " << fact_arg->getName() << endl;
                                    if (target_agent.compare(fact_arg->getName()) == 0) {
                                        //cout << target_agent << " yes3 " << fact_arg->getName() << endl;
                                        //create new fact landmark from initial state agent fact
                                        TimePointVariable ts((*this).get_tpv_factory().getNewTimepointVariable());
                                        TimePointVariable te((*this).get_tpv_factory().getNewTimepointVariable());
                                        ts.id = 0;
                                        te.id = 0;
                                        LiteralFormula *initial_state_formula = new SingleLiteralFormula(
                                                initial_state_fact.get_literal());
                                        TemporalStateLandmark *new_lm = new TemporalStateLandmark(initial_state_formula,
                                                                                                  ts, te);

                                        new_lm->type=2;
                                        new_lm->rule=9;
                                        new_lm->rootGoal=goal_lm_count;
                                        new_lm->parentLandmark=goal_lm;
                                        new_lm->rootLandmark=goal_lm;


                                        //push initial state fact (with landmarks[1] modification)
//                                        add_temporal_landmark(new_lm);
//                                        landmarks_map[goal_lm_count].push_back(landmarks[landmarks.size()-1]);
//                                        all_landmarks.push_back(landmarks[landmarks.size()-1]);

                                        std::pair<double,double> bounds;
                                        bounds.first = 0;
                                        bounds.second = 0;
                                        //new_lm->write(cout,bounds);
                                        cout << " added to " << endl;
                                        //goal_lm->formula->write(cout);
                                    }
                                    ++pit2;
                                }
                            }
                        }
                        ++pit;
                    }
                }
            }




            ++goal_lm_count;


            old_fact_lm_size = old_fact_lm_size + fact_lm_size;
           //old_landmarks_size = landmarks.size();

            old_landmarks_size = old_landmarks_size + landmarks.size();
            landmarks.clear();


        }
        landmarks = all_landmarks;
        //write(cout);


        cout << endl << endl;
        cout << "Number of temporal landmarks: " << landmarks.size() << endl<< endl;
        cout << "Number of temporal constraints: " << constraints.size() << endl << endl;
        //output_timpepoints();
        cout << "Temporal Landmarks Makespan Estimate: (t" << get_end_timepoint_variable().getID() << " lb): " << get_lower_bound(get_end_timepoint_variable()) << endl << endl;

        //solver->writeLp("mylp");
    }


    // mark disjunctive landmarks
    void TemporalLandmarksAndConstraints::markDisjunctiveLandmarks(){
        std::map < int, std::vector < TemporalLandmark * > > ::iterator it;
        for (it = landmarks_map.begin(); it != landmarks_map.end(); it++) {
            for (size_t i = 0; i < it->second.size(); i++) {
                //fact landmarks
                if(it->second[i]->get_num_timepoint_variables()==2){
                    TemporalStateLandmark *current_lm = dynamic_cast<TemporalStateLandmark*>(it->second[i]);
                    if(current_lm->formula->get_num_disjuncts() > 1){
                        it->second[i]->disjunctive = true;
                    }
                }
                //action landmarks
                if(it->second[i]->get_num_timepoint_variables()==1){
                    TemporalActionLandmark *current_lm = dynamic_cast<TemporalActionLandmark*>(it->second[i]);
                    if(current_lm->get_events().size() > 1){
                        it->second[i]->disjunctive = true;
                    }
                }
            }
        }
        std::cout << "Disjunctive Landmarks removed." << endl << endl;
    }

//    // mark disjunctive landmarks
//    void TemporalLandmarksAndConstraints::compressAbstractDisjunctiveLandmarks(){
//        std::map < int, std::vector < TemporalLandmark * > > ::iterator it;
//        for (it = landmarks_map.begin(); it != landmarks_map.end(); it++) {
//            for (size_t i = 0; i < it->second.size(); i++) {
//                //fact landmarks
//                if(it->second[i]->get_num_timepoint_variables()==2){
//                    TemporalStateLandmark *current_lm = dynamic_cast<TemporalStateLandmark*>(it->second[i]);
//                    if(current_lm->formula->get_num_disjuncts() > 1){
//
//                    }
//                }
//                //action landmarks
//                if(it->second[i]->get_num_timepoint_variables()==1){
//                    TemporalActionLandmark *current_lm = dynamic_cast<TemporalActionLandmark*>(it->second[i]);
//                    if(current_lm->get_events().size() > 1){
//
//                    }
//                }
//            }
//        }
//        std::cout << "Disjunctive Landmarks removed." << endl << endl;
//    }



    /*
     * update_if_abstract functions
     */

    //port details except timepoints
    TemporalLandmark* TemporalLandmarksAndConstraints::port_static_details(TemporalLandmark* origin, TemporalLandmark* target){
        target->disjunctive = origin->disjunctive;
        target->abstract = origin->abstract;
        target->id = origin->id;
        target->parentLandmark = origin->parentLandmark;
        target->rootLandmark = origin->rootLandmark;
        target->rootGoal = origin->rootGoal;
        target->rule = origin->rule;
        target->type = origin->type;
        return target;
    }


    //check landmarks if abstract and update if yes
    void TemporalLandmarksAndConstraints::update_if_abstract() {
        std::map < int, std::vector < TemporalLandmark * > > ::iterator it;
        for (it = landmarks_map.begin(); it != landmarks_map.end(); it++) {
            for (size_t i = 0; i < it->second.size(); i++) {
                //fact landmarks
                if(it->second[i]->get_num_timepoint_variables()==2){
                    TemporalStateLandmark *current_lm = dynamic_cast<TemporalStateLandmark*>(landmarks_map[it->first][i]);
                    LiteralSet updated_disjuncts = current_lm->formula->update_if_abstract_formula(agents_structure);
                    //if(current_lm->formula->get_num_disjuncts() == 1){
                        //non-disjunctive
                        if(updated_disjuncts.size()==1){
                            LiteralSet::iterator lit = updated_disjuncts.begin();
                            Literal* updated_literal = *lit;
                            if(updated_literal->abstract){
                                SingleLiteralFormula* updated_formula = new SingleLiteralFormula(dynamic_cast<SingleLiteralFormula*>(current_lm->formula)->get_literal());
                                updated_formula->abstract = true;
                                updated_formula->abstract_literal = updated_literal;
                                TemporalLandmark* abstract_landmark = new TemporalStateLandmark(updated_formula, current_lm->ts, current_lm->te);
                                landmarks_map[it->first][i]->abstract = true;
                                landmarks_map[it->first][i] = port_static_details(landmarks_map[it->first][i],abstract_landmark);
                            }
                        }
                        //disjunctive
                        if(updated_disjuncts.size()>1){
                            bool isAbstract = false;
                            for(LiteralSet::iterator lit = updated_disjuncts.begin(); lit != updated_disjuncts.end(); ++lit){
                                if((*lit)->abstract){
                                    isAbstract = true; break;
                                }
                            }
                            if(isAbstract) {
                                LiteralSet old_disjuncts = dynamic_cast<DisjunctiveLiteralFormula*>(current_lm->formula)->get_disjuncts();
                                DisjunctiveLiteralFormula* updated_formula = new DisjunctiveLiteralFormula(old_disjuncts);
                                updated_formula->abstract = true;
                                updated_formula->abstract_disjuncts = updated_disjuncts;
                                updated_formula->make_abstract();
                                TemporalLandmark* abstract_landmark = new TemporalStateLandmark(updated_formula, current_lm->ts, current_lm->te);
                                landmarks_map[it->first][i]->abstract = true;
                                landmarks_map[it->first][i] = port_static_details(landmarks_map[it->first][i],abstract_landmark);
                            }
                        }
                    //}
                }
                //action landmarks
                if(it->second[i]->get_num_timepoint_variables()==1){
                    TEventSet updated_events;
                    TemporalActionLandmark *current_lm = dynamic_cast<TemporalActionLandmark*>(it->second[i]);
                    updated_events = current_lm->update_if_abstract_event(agents_structure);
                    for(TEventSet::iterator eit = updated_events.begin(); eit != updated_events.end(); ++eit){
                        if((*eit).abstract){
                            TemporalLandmark* abstract_landmark = new TemporalActionLandmark(updated_events,current_lm->t);
                            landmarks_map[it->first][i]->abstract = true;
                            landmarks_map[it->first][i] = port_static_details(landmarks_map[it->first][i],abstract_landmark);
                            break;
                        }
                    }
                }
            }
        }
        std::cout << "Abstract Landmarks Updated." << endl << endl;
    }

    /*
     * landmark_based_decomposition_into_subgoals functions
     */

    // TODO fix non-dijunctive infinite loop bug
    //
    std::map < int, std::vector < TemporalLandmark *>> TemporalLandmarksAndConstraints::make_identical_if_duplicate(std::map < int, std::vector < TemporalLandmark *>> map) {
        std::map < int, std::vector < TemporalLandmark * > > ::iterator
        it1, it2;
        for (it1 = map.begin(); it1 != map.end(); it1++) {
            for (size_t i = 0; i < it1->second.size(); i++) {
                for (it2 = map.begin(); it2 != map.end(); it2++) {
                    for (size_t j = 0; j < it2->second.size(); j++) {
                        // do not process landmarks if disjunctive
                        if((it1->second[i]->disjunctive) || (it2->second[j]->disjunctive)){
                            continue;
                        }
                        if ((it1->second[i]->subsumed_by(it2->second[j])) && (it2->second[j]->subsumed_by(it1->second[i]))) {
                            if(it1->second[i]->get_num_timepoint_variables()==2) {
//                                if (current_lm->formula->get_num_disjuncts() == 1) {
                                //std::cout << "FACT LM-" << map[it1->first][i]->id << " is subsumed by FACT LM-" << map[it2->first][j]->id << endl;
                                //TODO decide on timepoints
                                //get timepoints from found
                                //map[it2->first][j] = it1->first][i];
                                //get timepoints from origin
                                map[it2->first][j] = port_static_details(map[it1->first][i],map[it2->first][j]);
//                                }
                            }
                            if(it1->second[i]->get_num_timepoint_variables()==1) {
                                map[it2->first][j] = port_static_details(map[it1->first][i],map[it2->first][j]);
                                //std::cout << "ACTION LM-" << map[it1->first][i]->id << " is subsumed by ACTION LM-" << map[it2->first][j]->id << endl;
                            }
                        }
                    }
                }
            }
        }
        return map;
    }


    //check and how many common landmars and export to ostream (for console output, not a component of GPAL)
    void TemporalLandmarksAndConstraints::check_common_landmarks(std::ostream & o){
        o << "Total Common Landmarks (" << common_landmarks.size() << "):" << endl << endl;
        std::map<int,TemporalLandmark *>::iterator it;
        for(it = common_landmarks.begin(); it != common_landmarks.end(); it++){
            int landmark_goals = 0;
            std::stringstream ss;
            vector<int> goals_in_set;
            for(int i = 0; i < it->second->goals_containing_landmark.size(); i++){
                if(it->second->goals_containing_landmark[i] != 0){
                    landmark_goals++;
                    ss << "G" << i << "-";
                    goals_in_set.push_back(i);
                }
            }
            std::map<string,GoalSet *>::iterator mit = goal_sets_map_fast.find(ss.str());
//            if(landmark_goals < subgoal_max_size){
            if(mit != goal_sets_map_fast.end()){
                goal_sets_map_fast[ss.str()]->push_back(it->second);
            }
            else{
                GoalSet* parent = new GoalSet(ss.str(),goals_in_set);
                parent->push_back(it->second);
                goal_sets_map_fast[ss.str()] = parent;
            }
//            }
            ss << "\n\n";
            it->second->no_of_goals = landmark_goals;
            it->second->output_to_stringstream(ss,get_landmark_bounds(it->second));
            if(it->second->abstract) o << "A-";
            o << "LM-" << it->second->id << " in " << it->second->no_of_goals <<  " goals: " << ss.str() << endl << endl;
        }
    }

    // sort goal sets by number of common landmarks
    void TemporalLandmarksAndConstraints::convert_to_vector(std::map<string,GoalSet *> goal_sets_map){
        std::transform(goal_sets_map.begin(), goal_sets_map.end(),
                       std::back_inserter(goal_sets_vector),
                       [](const pair<string,GoalSet *> &p) {
                           return p;
                       });
    }


    //assign weights to landmarks/goals sets and export to ostream
    void TemporalLandmarksAndConstraints::assign_weights(bool fast, std::ostream & o){
        //assign weights to landmarks
        for (int i = goal_sets_vector.size()-1; i >=0 ; i--){
            int normal_fact_lm_weight = 0;
            int normal_action_lm_weight = 0;
            int abstract_fact_lm_weight = 0;
            int abstract_action_lm_weight = 0;
            std::stringstream ss;
            vector<TemporalLandmark *> landmarks_w = goal_sets_vector[i].second->getLandmarks();
            int adjust = 1;
            if(fast)adjust = goal_sets_vector[i].second->getGoalsInSet().size();
            for(int j = 0; j < landmarks_w.size(); j++){
                bool abstract_lm = landmarks_w[j]->abstract;
                //fact landmarks weigth
                if(landmarks_w[j]->get_num_timepoint_variables()==2){
                    if(abstract_lm){
                        ss << "A-Fact-";
                        abstract_fact_lm_weight = abstract_fact_lm_weight + landmarks_w[j]->get_num_timepoint_variables()*adjust;
                    }
                    else{
                        ss << "Fact-";
                        normal_fact_lm_weight = normal_fact_lm_weight + landmarks_w[j]->get_num_timepoint_variables()*adjust;
                    }
                }
                //action landmarks weigyh
                if(landmarks_w[j]->get_num_timepoint_variables()==1){
                    if(abstract_lm){
                        ss << "A-";
                        abstract_action_lm_weight = abstract_action_lm_weight + landmarks_w[j]->get_num_timepoint_variables()*adjust;
                    }
                    else normal_action_lm_weight = normal_action_lm_weight + landmarks_w[j]->get_num_timepoint_variables()*adjust;
                    TemporalActionLandmark *current_lm = dynamic_cast<TemporalActionLandmark*>(landmarks_w[j]);
                    if (current_lm->events.begin()->get_event_type() == E_AT_START)
                        ss << "START-";
                    else if (current_lm->events.begin()->get_event_type() == E_AT_END)
                        ss << "END-";
                }
                ss << "LM-" << landmarks_w[j]->id <<"  ";
            }
            //duplicate goal achiever end action landmark bug error fix
            if(normal_action_lm_weight%2!=0)normal_action_lm_weight--;
            if(abstract_action_lm_weight%2!=0)abstract_action_lm_weight--;

            //TODO check if this needed anymore and remove if not
            goal_sets_vector[i].second->setNormalFactScore(normal_fact_lm_weight/2);
            goal_sets_vector[i].second->setNormalActionScore(normal_action_lm_weight/2);
            goal_sets_vector[i].second->setAbstractFactScore(abstract_fact_lm_weight/2);
            goal_sets_vector[i].second->setAbstractActionScore(abstract_action_lm_weight/2);
//            if (landmarks_w.size() > 0)o << goal_sets_vector[i].first << endl << ss.str() << "\n" << "Algo score: " << goal_sets_vector[i].second->getNormalandAbstractScore() << " Total-Normal-Score: " << (normal_fact_lm_weight+normal_action_lm_weight)/2 << " Total-Normal+Abstract-Score: " << (normal_fact_lm_weight+abstract_fact_lm_weight+normal_action_lm_weight+abstract_action_lm_weight)/2   << "   Fact-LM-Score: " << normal_fact_lm_weight/2 << "   A-Fact-LM-Score: " << abstract_fact_lm_weight/2 << "   Action-LM-Score: " << normal_action_lm_weight/2 << "   A-Action-LM-Score: " << abstract_action_lm_weight/2<< endl << endl;
        }

        //sort based on length
        std::sort(goal_sets_vector.begin(), goal_sets_vector.end(),
                  [](const pair<string,GoalSet *>& l, const pair<string,GoalSet *>& r){
                      if (l.first.length() != r.first.length())return l.first.length() < r.first.length();
                      return l.first < r.first;});
        //compute inheritance
    }

    void TemporalLandmarksAndConstraints::compute_inherited_score(bool fast) {
        int depth = 1;
        if(fast)depth = goal_sets_vector[goal_sets_vector.size()-1].second->getGoalsInSet()[goal_sets_vector[goal_sets_vector.size()-1].second->getGoalsInSet().size()-1];
        for(int i = 0; i < goal_sets_vector.size(); i++){
            for(int j = 0; j < goal_sets_vector.size(); j++){
                if((goal_sets_vector[j].second->getGoalsInSet()[goal_sets_vector[j].second->getGoalsInSet().size()-1] <= goal_sets_vector[i].second->getGoalsInSet()[goal_sets_vector[i].second->getGoalsInSet().size()-1]) && (goal_sets_vector[i].second->getGoalsInSet().size() - goal_sets_vector[j].second->getGoalsInSet().size() <= depth) && (goal_sets_vector[i].second->getGoalsInSet().size() > goal_sets_vector[j].second->getGoalsInSet().size())){
//                    std::cout << endl << goal_sets_vector[j].second->getGoalsInSet()[goal_sets_vector[j].second->getGoalsInSet().size()-1] << " " << goal_sets_vector[i].second->getGoalsInSet()[goal_sets_vector[i].second->getGoalsInSet().size()-1] << endl;
                    //if(bloom_filter_found(i,j)){
                    if(contains_goalset(i,j)){
                        //goal_sets_vector[i].second->inheritAllInheritedScores(goal_sets_vector[j].second);
                        //goal_sets_vector[i].second->inheritAllBaseScores(goal_sets_vector[j].second);
                        //goal_sets_vector[j].second->setIsContained(true);
                    }
                    //}
                }
            }
        }
    }

    //check if smaller set is in bigger set
    bool TemporalLandmarksAndConstraints::contains_goalset(int parent,int child){
        int count = 0;
        bool found = true;
        for(int i = 0; i < goal_sets_vector[child].second->getGoalsInSet().size(); i++){
            if((goal_sets_vector[child].second->getGoalsInSet()[i] >= goal_sets_vector[parent].second->getGoalsInSet()[count])&&(found)){
                found = false;
                for(int j = 0; j < goal_sets_vector[parent].second->getGoalsInSet().size(); j++){
                    if(goal_sets_vector[child].second->getGoalsInSet()[i] == goal_sets_vector[parent].second->getGoalsInSet()[j]){
                        ++count;
                        found = true;
                        break;
                    }
                }
            }
        }
        if(count == goal_sets_vector[child].second->getGoalsInSet().size()){
            return true;
        }
        return false;
    }

    // check if 2 sets have at least one
    bool TemporalLandmarksAndConstraints::have_common_goals(int parent,int child){
        for(int i = 0; i < goal_sets_vector[child].second->getGoalsInSet().size(); i++){
            for(int j = 0; j < goal_sets_vector[parent].second->getGoalsInSet().size(); j++){
                if(goal_sets_vector[child].second->getGoalsInSet()[i] == goal_sets_vector[parent].second->getGoalsInSet()[j]){
                    return true;
                }
            }
        }
        return false;
    }


    //sort based on score and uniquenes
    void TemporalLandmarksAndConstraints::eliminate_sets_with_common_goals_and_send_to_ProblemGenerator(std::ostream & o, int decomposition){
        std::stringstream ss;
        //o << endl << "Context " << decomposition << " - Possible subgoals (max subgoal size " << subgoal_max_size << "): " << endl << endl;
        o << endl << "Context " << decomposition << ": " << endl << endl;


        //option 1 (enable if option 2 disabled): disable upper sets for not populating all decompositions based on largest landmark
        for(int i = 0; i < goal_sets_vector.size(); i++) {
            if (i > last_head_subgoal) {
                goal_sets_vector[i].second->setIsContained(true);
            }
        }


        //option 2 (enable if option 1 disabled): eliminate goal-sets with goals in common with the starting set
//        for (int i = goal_sets_vector.size()-1; i >=0 ; i--){
//            if (i == last_head_subgoal)continue;
//            if(have_common_goals(i,last_head_subgoal)){
//                goal_sets_vector[i].second->setIsContained(true);
//            }
//        }


        //check if goal-sets cotain each other
        for (int i = goal_sets_vector.size()-1; i >=0 ; i--){
            int max_head = i-1;
            for (int j = max_head; j >=0 ; j--){
                if((!goal_sets_vector[i].second->isContained()) && (!goal_sets_vector[j].second->isContained()) && (have_common_goals(i,j))){
                    goal_sets_vector[j].second->setIsContained(true);
                }
            }
        }


        //output results
        map<int,vector<int>> subgoals_set;
        int set_index = 0;
        int no_of_goals = 0;
        o << "Head Set: " << goal_sets_vector[last_head_subgoal].first  << endl << "Norm: " << goal_sets_vector[last_head_subgoal].second->getNormalScore()+goal_sets_vector[last_head_subgoal].second->getInheritedNormalScore() << "   Abs: " << goal_sets_vector[last_head_subgoal].second->getAbstractScore()+goal_sets_vector[last_head_subgoal].second->getInheritedAbstractScore() << "   Norm+Abs: " << goal_sets_vector[last_head_subgoal].second->getNormalandAbstractScore()+goal_sets_vector[last_head_subgoal].second->getInheritedNormalandAbstractScore() << endl;
        no_of_goals = no_of_goals + goal_sets_vector[last_head_subgoal].second->getGoalsInSet().size();
        subgoals_set[set_index] = goal_sets_vector[last_head_subgoal].second->getGoalsInSet();
        set_index++;
        for (int i = goal_sets_vector.size()-1; i >=0 ; i--){
            if (i == last_head_subgoal)continue;
            if ((goal_sets_vector[i].second->getLandmarks().size() > 0) && (!goal_sets_vector[i].second->isContained())){
                o << goal_sets_vector[i].first  << endl << "Norm: " << goal_sets_vector[i].second->getNormalScore()+goal_sets_vector[i].second->getInheritedNormalScore() << "   Abs: " << goal_sets_vector[i].second->getAbstractScore()+goal_sets_vector[i].second->getInheritedAbstractScore() << "   Norm+Abs: " << goal_sets_vector[i].second->getNormalandAbstractScore()+goal_sets_vector[i].second->getInheritedNormalandAbstractScore() << endl;
                no_of_goals = no_of_goals + goal_sets_vector[i].second->getGoalsInSet().size();
                subgoals_set[set_index] = goal_sets_vector[i].second->getGoalsInSet();
                set_index++;
            }
        }
        o << "Subgoals (dead-end agent) with common LM: " << set_index << endl << "Dead-end agent Goals in Subgoals: " << no_of_goals << endl << "Leftover dead-end agent goals: " << landmarks_map.size() - no_of_goals << endl << "Total dead-end agent Goals: " << landmarks_map.size() << endl;
        SubgoalsSet* subgoalsSet = new SubgoalsSet(subgoals_set);

        if (!problemGenerator->addSubgoalsSet(subgoals_set)){
            o << "DUPLICATE CONTEXT" << endl;
        }
        //output remainder subgoal
    }


    void TemporalLandmarksAndConstraints::landmark_based_decomposition_into_subgoals(bool include_abstract, bool random, bool connected_map, std::ostream & o, std::chrono::_V2::system_clock::time_point begin, double procedure_start_time, bool all_dead_end_agent_goals_problem, string node) {
        //common landmark, <nr of goals that have the common landmark, <exact goals that have the common landmark>>
        //std::map < TemporalLandmark *, std::pair<int,std::vector<int>>> duplicat_landmarks_map;
        std::vector<int> landmark_goals_vector(landmarks_map.size(), 0);
        std::map < int, std::vector < TemporalLandmark * > > export_landmarks, compare_landmarks;
        export_landmarks = make_identical_if_duplicate(landmarks_map);
        compare_landmarks = export_landmarks;
        o << endl << endl << "Duplicate Landmarks:" << endl;
        std::map < int, std::vector < TemporalLandmark * > > ::iterator it1, it2;
        int goal_lm_count = 0;
        //if (all_dead_end_agent_goals_problem){
            for (it1 = export_landmarks.begin(); it1 != export_landmarks.end(); it1++) {
                compare_landmarks.erase(goal_lm_count);
                for (size_t i = 0; i < it1->second.size(); i++) {
                    for (it2 = compare_landmarks.begin(); it2 != compare_landmarks.end(); it2++) {
                        for (size_t j = 0; j < it2->second.size(); j++) {

                            // do not process landmarks if disjunctive
                            if((it1->second[i]->disjunctive) || (it2->second[j]->disjunctive)){
                                continue;
                            }
                            if (it1->second[i]->subsumed_by(it2->second[j]) && (it2->second[j]->subsumed_by(it1->second[i]))) {

                                //do not process abstract landmarks if abstract not specified true
                                if(it1->second[i]->abstract && !include_abstract){
                                    continue;
                                }
                                //add goals to common fact landmark
                                if(it1->second[i]->get_num_timepoint_variables()==2){
                                    TemporalStateLandmark *current_lm = dynamic_cast<TemporalStateLandmark*>(it1->second[i]);
                                    //check if map entry exists
                                    std::map<int,TemporalLandmark *>::iterator mit;
                                    mit = common_landmarks.find(it1->second[i]->id);
                                    if(mit == common_landmarks.end()){
                                        common_landmarks[it1->second[i]->id] = it1->second[i];
                                        common_landmarks[it1->second[i]->id]->goals_containing_landmark = landmark_goals_vector;
                                    }
                                    //insert goals to landmark goals vector
                                    common_landmarks[it1->second[i]->id]->goals_containing_landmark[it1->first] = 2;
                                    common_landmarks[it1->second[i]->id]->goals_containing_landmark[it2->first] = 2;
                                    if(current_lm->abstract)o << "\nA-";else o << "\n";
                                    if(current_lm->disjunctive)o << "Disjunctive ";
                                    o << "Fact Between GOAL-" << it1->first << " and GOAL-" << it2->first;
                                    if(current_lm->abstract)o << " | A-LM-";else o << " | LM-";
                                    o << it1->second[i]->id << endl;
                                }
                                //add goals to common action landmarks
                                if(it1->second[i]->get_num_timepoint_variables()==1){
                                    TemporalActionLandmark *current_lm = dynamic_cast<TemporalActionLandmark*>(it1->second[i]);
                                    //check if map entry exists
                                    std::map<int,TemporalLandmark *>::iterator mit;
                                    mit = common_landmarks.find(it1->second[i]->id);
                                    if(mit == common_landmarks.end()){
                                        common_landmarks[it1->second[i]->id] = it1->second[i];
                                        common_landmarks[it1->second[i]->id]->goals_containing_landmark = landmark_goals_vector;
                                    }
                                    //insert goals to landmark goals vector
                                    common_landmarks[it1->second[i]->id]->goals_containing_landmark[it1->first] = 1;
                                    common_landmarks[it1->second[i]->id]->goals_containing_landmark[it2->first] = 1;
                                    if(current_lm->abstract)o << "\nA-";else o << "\n";
                                    if(current_lm->disjunctive)o << "Disjunctive ";
                                    if (current_lm->events.begin()->get_event_type() == E_AT_START)
                                        o << "START Action";
                                    else if (current_lm->events.begin()->get_event_type() == E_AT_END)
                                        o << "END Action";
                                    o << " Between GOAL-" << it1->first << " and GOAL-" << it2->first;
                                    if(current_lm->abstract)o << " | A-LM-";else o << " | LM-";
                                    o << it1->second[i]->id << endl;
                                }
                            }
                        }
                    }
                }
                if (compare_landmarks.size() > 0)o << "\n\n   GOAL " << goal_lm_count << " END" << endl << endl;
                ++goal_lm_count;
            }

            cout << "Duplicate Landmarks extracted." << endl << endl;


            //remove dead-end agents
//        vector<pair<int,string>> temp_agents_structure;
//        int target_priority = agents_structure[agents_structure.size()-1].first;
//        for (int i = 0; i < agents_structure.size(); i++){
//            if(agents_structure[i].first == target_priority) {
//                cout << agents_structure[i].second << " popped" << endl;
//                continue;
//            }
//            temp_agents_structure.push_back(agents_structure[i]);
//        }
//        agents_structure = temp_agents_structure;
        //}


        problemGenerator = new ProblemGenerator(random,original_problem_file_name,original_domain_file_name,procedure_start_time,agents_structure,dynamic_types_vector,connected_map,landmarks_map.size());

        //if (all_dead_end_agent_goals_problem){
            subgoal_max_size = 0;


            check_common_landmarks(o); // for console, not a component of GPAL

            convert_to_vector(goal_sets_map_fast);
            assign_weights(true,o);





            cout << "debug no of subgoals: " << goal_sets_vector.size() << endl;
            for (int i = 0; i < goal_sets_vector.size(); i++){
                cout << "debug size: " << goal_sets_vector[i].second->getGoalsInSet().size() << endl;
                cout << "debug score: " << goal_sets_vector[i].second->getNormalandAbstractScore() << endl;
                cout << "debug goalset: " << goal_sets_vector[i].first << endl;


//            vector<int>::iterator iit = find(goal_sizes.begin(), goal_sizes.end(), goal_sets_vector[i].second->getGoalsInSet().size());
//            if(iit == goal_sizes.end()){
//                goal_sizes.push_back(goal_sets_vector[i].second->getGoalsInSet().size());
//            }
//            if(subgoal_max_size < goal_sets_vector[i].second->getGoalsInSet().size()){
//                subgoal_max_size = goal_sets_vector[i].second->getGoalsInSet().size();
//            }
            }
//        goal_sizes.erase(
//                std::unique(goal_sizes.begin(), goal_sizes.end()),
//                goal_sizes.end());
//        std::sort(goal_sizes.begin(), goal_sizes.end());

            //sort subgoals by size (1st) and score (2nd)
            std::sort(goal_sets_vector.begin(), goal_sets_vector.end(),
                      [](const pair<string,GoalSet *>& l, const pair<string,GoalSet *>& r){
                          if (l.second->getGoalsInSet().size() != r.second->getGoalsInSet().size())return l.second->getGoalsInSet().size() < r.second->getGoalsInSet().size();
                          return l.second->getNormalandAbstractScore() + l.second->getInheritedNormalandAbstractScore() < r.second->getNormalandAbstractScore() + r.second->getInheritedNormalandAbstractScore();});

            for (int i = goal_sets_vector.size()-1; i >=0 ; i--){
                for (int j = 0; j < goal_sets_vector.size(); j++){
                    goal_sets_vector[j].second->clearInheritedScore();
                    goal_sets_vector[j].second->setIsContained(false);
                }
                last_head_subgoal = i;

                compute_inherited_score(true);

                eliminate_sets_with_common_goals_and_send_to_ProblemGenerator(o,goal_sets_vector.size()-1-i);
            }

            //sort subgoals by score (1st) and size (2nd)
            std::sort(goal_sets_vector.begin(), goal_sets_vector.end(),
                      [](const pair<string,GoalSet *>& l, const pair<string,GoalSet *>& r){
                          if (l.second->getNormalandAbstractScore() + l.second->getInheritedNormalandAbstractScore() != r.second->getNormalandAbstractScore() + r.second->getInheritedNormalandAbstractScore())return l.second->getNormalandAbstractScore() + l.second->getInheritedNormalandAbstractScore() < r.second->getNormalandAbstractScore() + r.second->getInheritedNormalandAbstractScore();
                          return l.second->getGoalsInSet().size() < r.second->getGoalsInSet().size();});

            for (int i = goal_sets_vector.size()-1; i >=0 ; i--){
                for (int j = 0; j < goal_sets_vector.size(); j++){
                    goal_sets_vector[j].second->clearInheritedScore();
                    goal_sets_vector[j].second->setIsContained(false);
                }
                last_head_subgoal = i;

                compute_inherited_score(true);

                eliminate_sets_with_common_goals_and_send_to_ProblemGenerator(o,goal_sets_vector.size()-1-i);
            }
        //}

        //Start of Solving Layer
        problemGenerator->generateAllProblems(begin, procedure_start_time, false, true, true, node);
    }



//    //get the number of common landmakrs for each goal set
//    void TemporalLandmarksAndConstraints::compute_common_lm_goalsets(std::ostream & o){
//        std::map<int,TemporalLandmark *>::iterator it;
//        for(it = common_landmarks.begin(); it != common_landmarks.end(); it++){
//            if (it->second->no_of_goals < subgoal_max_size){
//                for(int i = 0; i <it->second->goals_containing_landmark.size(); i++){
//                    if((it->second->goals_containing_landmark[i] != 0) && (i < it->second->goals_containing_landmark.size()-1)){
//                        std::stringstream ss;
//                        ss << "G" << i << "-";
//                        vector<int> goals_in_set;
//                        goals_in_set.push_back(i);
//                        GoalSet* parent = new GoalSet(ss.str(),goals_in_set);
//                        complete_goal_sets(it,i,*parent,o);
//                    }
//                }
//            }
//        }
//    }
//
//    //recursive method for matching all goal sets with their common landmarks
//    void TemporalLandmarksAndConstraints::complete_goal_sets(std::map<int,TemporalLandmark *>::iterator it, int i, GoalSet parent_set, std::ostream & o)  {
//        //std::cout << endl << "LM-" << it->first->id << endl;
//        for(int j = i+1; j < it->second->goals_containing_landmark.size(); j++){
//            if(it->second->goals_containing_landmark[j] != 0) {
//                std::stringstream ss;
//                ss << parent_set.getName() << "G" << j <<"-";
//                std::map<string,GoalSet *>::iterator mit = goal_sets_map.find(ss.str());
//                if(mit != goal_sets_map.end())
//                {
//                    goal_sets_map[ss.str()]->push_back(it->second);
//                }
//                else{
//                    vector<int> goals_in_set;
//                    goals_in_set=parent_set.getGoalsInSet();
//                    goals_in_set.push_back(j);
//                    GoalSet* child = new GoalSet(&parent_set,ss.str(),goals_in_set);
//                    child->push_back(it->second);
//                    goal_sets_map[ss.str()] = child;
//                }
//                if(j < it->second->goals_containing_landmark.size()-1){
//                    complete_goal_sets(it,j,*goal_sets_map[ss.str()],o);
//                }
//            }
//        }
//    }









    std::pair<double,double> TemporalLandmarksAndConstraints::get_landmark_bounds(TemporalLandmark* lm){
        std::pair<double,double> bounds;
        if(lm->get_num_timepoint_variables()==1){
            bounds.first = get_lower_bound(lm->get_timepoint_variables(0)).toDouble();
        }
        else{
            bounds.first = get_lower_bound(lm->get_timepoint_variables(0)).toDouble();
            bounds.second = get_lower_bound(lm->get_timepoint_variables(1)).toDouble();
        }
        return bounds;
    }

    void TemporalLandmarksAndConstraints::write_lisp(std::ostream & o)  {
        for (size_t i = 0; i < landmarks.size(); i++) {
            landmarks[i]->write_lisp(o);
            o << endl;
        }
    }

    void TemporalLandmarksAndConstraints::write(std::ostream & o)  {
        o << "Landmarks: " << endl << endl;
        std::map<int, std::vector<TemporalLandmark *> >::iterator it;
        std::pair<double,double> bounds;
        bounds.first = 0;
        bounds.second = 0;
        for ( it = landmarks_map.begin(); it != landmarks_map.end(); it++ ){

            for (size_t i = 0; i < it->second.size(); i++) {
                it->second[i]->write(o, get_landmark_bounds(it->second[i]));
                o << endl << endl;
            }
        }
//        o << "Constraints: " << endl;
//        for (set<SimpleTemporalConstraint>::const_iterator it = constraints.begin(); it != constraints.end(); ++it)	{
//            if (is_used(it->t1) && is_used(it->t2)) {
//                it->write(o);
//                o << endl;
//            }
//        }
    }

    EpsilonResolutionTimestamp TemporalLandmarksAndConstraints::get_lower_bound(const TimePointVariable &v) {
//	cout << "LB: ";
//	v.write(cout);
//	cout << endl;

        solver->clearObjective();
        solver->setObjCoeff(v.getID(), 1.0);
        solver->setMaximiseObjective(false);


        if (solver->solve(false)) {
            return EpsilonResolutionTimestamp(solver->getObjValue(), true);
        }
        return -EpsilonResolutionTimestamp::zero();
    }

    EpsilonResolutionTimestamp TemporalLandmarksAndConstraints::get_upper_bound(const TimePointVariable &v) {
//	cout << "UB: ";
//	v.write(cout);
//	cout << endl;

        solver->clearObjective();
        solver->setObjCoeff(v.getID(), 1.0);
        solver->setMaximiseObjective(true);


        if (solver->solve(false)) {
            return EpsilonResolutionTimestamp(solver->getObjValue(), false);
        }
        return EpsilonResolutionTimestamp::infinite();
    }

    EpsilonResolutionTimestamp TemporalLandmarksAndConstraints::get_lower_bound(
            const TimePointVariable &t1, const TimePointVariable &t2) {
//	cout << "LB: ";
//	t1.write(cout);
//	cout << " - ";
//	t2.write(cout);
//	cout << endl;

        solver->clearObjective();
        solver->setObjCoeff(t1.getID(), 1.0);
        solver->setObjCoeff(t2.getID(), -1.0);
        solver->setMaximiseObjective(false);

        if (solver->solve(false)) {
            return EpsilonResolutionTimestamp(solver->getObjValue(), true);
        }
        return -EpsilonResolutionTimestamp::infinite();
    }

    EpsilonResolutionTimestamp TemporalLandmarksAndConstraints::get_upper_bound(
            const TimePointVariable &t1, const TimePointVariable &t2) {
//	cout << "UB: ";
//	t1.write(cout);
//	cout << " - ";
//	t2.write(cout);
//	cout << endl;

        solver->clearObjective();
        solver->setObjCoeff(t1.getID(), 1.0);
        solver->setObjCoeff(t2.getID(), -1.0);
        solver->setMaximiseObjective(true);

        if (solver->solve(false)) {
            return EpsilonResolutionTimestamp(solver->getObjValue(), false);
        }
        return EpsilonResolutionTimestamp::infinite();
    }
} /* namespace temporal_landmarks */
