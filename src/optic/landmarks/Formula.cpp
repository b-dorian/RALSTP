/*
 * Formula.cpp
 *
 *  Created on: Apr 11, 2014
 *      Author: karpase
 */

#include "Formula.h"
#include "RPGBuilder.h"
#include <cassert>
#include <algorithm>
#include "temporalanalysis.h"
#include "FFEvent.h"

using namespace Planner;

namespace temporal_landmarks {


    size_t get_literal_achievers(const Literal *literal, TEventSet &achievers, EpsilonResolutionTimestamp max_time) {
        //literal->write(std::cout);
        assert(literal->getStateID() < RPGBuilder::getEffectsToActions().size());
        const list <pair<int, Planner::time_spec>> &ach_list = RPGBuilder::getEffectsToActions()[literal->getStateID()];
        for (list < pair < int, Planner::time_spec > > ::const_iterator it = ach_list.begin(); it != ach_list.end();
        ++it) {
            TEvent event(it->first, it->second);
            if (event.get_earliest() <= max_time) {
                achievers.insert(event);
            }
        }
        return achievers.size();
    }

    EpsilonResolutionTimestamp get_earliest_for_literal(const Literal *literal) {
        LiteralSet state;
        vector<double> fluents;
        RPGBuilder::getInitialState(state, fluents);
        Literal *lit = const_cast<Literal *>(literal);
        if (state.find(lit) != state.end()) {
//		std::cout << "\t In initial state" << std::endl;
            return EpsilonResolutionTimestamp::zero();
        }

        EpsilonResolutionTimestamp ret = EpsilonResolutionTimestamp::infinite();

        TEventSet ach;
        get_literal_achievers(literal, ach, EpsilonResolutionTimestamp::infinite());
//	cout << "\t +++> " << ach.size() << std::endl;
        for (TEventSet::const_iterator fit = ach.begin(); fit != ach.end(); ++fit) {
            EpsilonResolutionTimestamp t = RPGHeuristic::getEarliestForStarts()[fit->get_opid()];
//		std::cout << "\t\t\t\t\t\t\t";
//		fit->write(std::cout);
//		std::cout << " - " << t << " / " << TemporalAnalysis::actionIsNeverApplicable(fit->get_opid()) << std::endl;
            if (t < ret) {
                ret = t;
            }
        }
        //std::cout << "\t\t\t\t\t ---> " << RPGHeuristic::getEarliestPropositionPOTimes()[literal->getStateID()] << std::endl;
        return ret;
    }

    size_t get_first_achievers_of_literals(const LiteralSet &literals, TEventSet &achievers) {
//	std::cout << "\tGetting first achievers of (" << literals.size() << "): ";
//	(*literals.begin())->write(std::cout);
//	std::cout << std::endl;

        TEventSet tev_achievers;
        for (LiteralSet::const_iterator lit = literals.begin(); lit != literals.end(); ++lit) {
            get_literal_achievers(*lit, tev_achievers, EpsilonResolutionTimestamp::infinite());
        }
//	std::cout << "\t\t" << tev_achievers << std::endl;

        set<int> all_achievers_ids;
        for (TEventSet::const_iterator it = tev_achievers.begin(); it != tev_achievers.end(); ++it) {
            const TEvent &event = *it;
//		std::cout << "excluding: ";
//		it->write(std::cout);
//		std::cout << std::endl;
//		if (TemporalAnalysis::actionIsNeverApplicable(event.get_opid())) {
//			std::cout << "action ";
//			event.write(std::cout);
//			std::cout << " is never applicable" << std::endl;
//			continue;
//		}
            all_achievers_ids.insert(event.get_opid());
        }

        //std::cout << "FIRST ACHIEVER START --------------------------------------------" << std::endl;
        RPGHeuristic *currRPG = RPGBuilder::generateRPGHeuristic(all_achievers_ids);
        TemporalAnalysis::findActionTimestampLowerBounds(currRPG);


        const map<Literal *, set < pair < int, Planner::time_spec> > > &fa = currRPG->get_fact_achievers();
        for (map < Literal * , set < pair < int, Planner::time_spec > > > ::const_iterator it = fa.begin();
                it != fa.end();
        ++it) {
//		const Literal* lit = it->first;
//		lit->write(std::cout);
//		std::cout << std::endl;
            for (set < pair < int, Planner::time_spec > > ::const_iterator sit = it->second.begin(); sit !=
                                                                                                     it->second.end();
            ++sit) {
                TEvent event(sit->first, sit->second);
//			std::cout << "\t";
//			event.write(std::cout);
//			std::cout << std::endl;
            }
        }


//	std::cout << "FIRST ACHIEVER END --------------------------------------------" << std::endl;

        /*
        LiteralSet initialState;
        vector<double> initialFluents;

        RPGBuilder::getNonStaticInitialState(initialState, initialFluents);

        MinimalState refState;
        refState.insertFacts(initialState.begin(), initialState.end(), StepAndBeforeOrAfter());

        refState.secondMin = initialFluents;
        refState.secondMax = initialFluents;

        list<FFEvent> dummyPlan;
        TemporalAnalysis::pullInAbstractTILs(refState, dummyPlan);


        currRPG->doFullExpansion(refState, dummyPlan);
    */
//	for (size_t i = 0; i < RPGHeuristic::getEarliestForStarts().size(); i++) {
//		std::cout << "\t";
//		RPGBuilder::getInstantiatedOp(i)->write(std::cout);
//		std::cout << " : " << RPGHeuristic::getEarliestForStarts()[i] << std::endl;
//	}

//	std::cout << "Going over achievers:" << std::endl;
        for (TEventSet::const_iterator it = tev_achievers.begin(); it != tev_achievers.end(); ++it) {
            const TEvent &event = *it;
            assert(event.get_earliest() == EpsilonResolutionTimestamp::infinite());

//		std::cout << "\t\t\t";
//		event.write(std::cout);
//		std::cout << std::endl;

//		std::cout << "\t";
//		event.write(std::cout);
//		std::cout << std::endl;

            EpsilonResolutionTimestamp t_act = EpsilonResolutionTimestamp::zero();

            list < Literal * > cond(event.get_conditions(false, false));
            if (event.get_event_type() == E_AT_END) {
                // if this is an END event, we want to remove overall/end conditions which are achieved by start effects

                assert(event.other_end().get_event_type() == E_AT_START);
                const list<Literal *> &start_eff = event.other_end().get_conditions(false, true);

                const list<Literal *> &cond_oe = event.other_end().get_conditions(false, false);
                for (list<Literal *>::const_iterator cit = cond_oe.begin(); cit != cond_oe.end(); ++cit) {
                    if (::find(start_eff.begin(), start_eff.end(), *cit) == start_eff.end()) {
                        cond.push_back(*cit);
                    }
                }
                const list<Literal *> &cond_inv = event.get_conditions(true, false);
                for (list<Literal *>::const_iterator cit = cond_inv.begin(); cit != cond_inv.end(); ++cit) {
                    if (::find(start_eff.begin(), start_eff.end(), *cit) == start_eff.end()) {
                        cond.push_back(*cit);
                    }
                }
            }

            for (list<Literal *>::const_iterator cit = cond.begin(); cit != cond.end(); ++cit) {
                const Literal *lit = *cit;
//			std::cout << "\t\t\t\t";
//			lit->write(std::cout);
                EpsilonResolutionTimestamp t_lit = get_earliest_for_literal(lit);
//			std::cout << "  -  " << t_lit << std::endl;
                if (t_lit > t_act) {
                    t_act = t_lit;
                }
//			std::cout << "\t\t";
//			lit->write(std::cout);
//			std::cout << " " << t_lit << std::endl;
            }

//		std::cout << "\t" << t_act << std::endl;
            if (t_act < EpsilonResolutionTimestamp::infinite()) {
                achievers.insert(event);
            }
//		else {
//			std::cout << "pruning ";
//			event.write(std::cout);
//			std::cout << " from first achievers " << std::endl;
//		}
        }

        // restore? RPGHeuristic
        //set<int> empty;
        //currRPG = RPGBuilder::generateRPGHeuristic(empty);
        //currRPG->doFullExpansion(refState, dummyPlan);

        delete currRPG;
        TemporalAnalysis::findActionTimestampLowerBounds(RPGBuilder::getHeuristic());

        return achievers.size();
    }

    size_t DisjunctiveLiteralFormula::get_achievers(TEventSet &achievers, EpsilonResolutionTimestamp max_time) const {
        for (LiteralSet::const_iterator it = disjuncts.begin(); it != disjuncts.end(); ++it) {
            get_literal_achievers(*it, achievers, max_time);
        }
        return achievers.size();
    }

    size_t SingleLiteralFormula::get_achievers(TEventSet &achievers, EpsilonResolutionTimestamp max_time) const {
        return get_literal_achievers(literal, achievers, max_time);
    }

    size_t DisjunctiveLiteralFormula::get_first_achievers(TEventSet &achievers) const {
        get_first_achievers_of_literals(disjuncts, achievers);
        return achievers.size();
    }

    size_t SingleLiteralFormula::get_first_achievers(TEventSet &achievers) const {
        LiteralSet s;
        s.insert(const_cast<Literal *>(literal));

        get_first_achievers_of_literals(s, achievers);
        return achievers.size();
    }

    bool literal_deleted_by(const Literal *literal, const TEvent &event) {
        const list<Literal *> &deleted_literals = (event.get_event_type() == E_AT_START ?
                                                   RPGBuilder::getStartPropositionDeletes()[event.get_opid()] :
                                                   RPGBuilder::getEndPropositionDeletes()[event.get_opid()]);

        return ::find(deleted_literals.begin(), deleted_literals.end(), literal) != deleted_literals.end();
    }

    bool literal_added_by(const Literal *literal, const TEvent &event) {
        const list<Literal *> &added_literals = (event.get_event_type() == E_AT_START ?
                                                 RPGBuilder::getStartPropositionAdds()[event.get_opid()] :
                                                 RPGBuilder::getEndPropositionAdds()[event.get_opid()]);

        return ::find(added_literals.begin(), added_literals.end(), literal) != added_literals.end();
    }

    void SingleLiteralFormula::get_literals_in_formula_added_by_event(
            const TEvent &event, LiteralSet &added_literals) const {
        if (literal_added_by(literal, event)) {
            Literal *nclit = const_cast<Literal *>(literal);
            added_literals.insert(nclit);
        }
    }

    void DisjunctiveLiteralFormula::get_literals_in_formula_added_by_event(
            const TEvent &event, LiteralSet &added_literals) const {
        for (LiteralSet::const_iterator it = disjuncts.begin(); it != disjuncts.end(); ++it) {
            if (literal_added_by(*it, event)) {
                Literal *nclit = const_cast<Literal *>(*it);
                added_literals.insert(nclit);
            }
        }
    }

//TODO try with goal as not an abstract landmarks maybe from here
//LiteralSet SingleLiteralFormula::update_if_abstract_formula(const LiteralFormula *target_formula) const {
//    LiteralSet *updated_set = new LiteralSet();
//    const SingleLiteralFormula *sl = dynamic_cast<const SingleLiteralFormula *>(target_formula);
//    if (sl != 0) {
//        Literal *target_literal = const_cast<Literal *>(sl->get_literal());
//        Literal *goal_literal = const_cast<Literal *>(literal);
//        auto received_literal = update_if_abstract_literal(target_literal, goal_literal);
//        Literal *updated_literal = new Literal(received_literal.getProp(), received_literal.getEnv());
//        updated_literal->abstract = received_literal.abstract;
//        updated_set->insert(updated_literal);
//    }
//    return *updated_set;
//}
//
//LiteralSet DisjunctiveLiteralFormula::update_if_abstract_formula(const LiteralFormula *other) const {
//    return disjuncts;
//}
//
//Literal LiteralFormula::update_if_abstract_literal(Literal *target_literal, Literal *goal_literal) const {
//    VAL::LiteralParameterIterator<VAL::parameter_symbol_list::iterator> it1 = target_literal->begin();
//    VAL::LiteralParameterIterator<VAL::parameter_symbol_list::iterator> it2 = goal_literal->begin();
//    int i = target_literal->getProp()->args->size();
//    int j = goal_literal->getProp()->args->size();
//    VAL::parameter_symbol_list *final_parameter_symbol_list = new VAL::parameter_symbol_list;
//    bool abstract = false;
//    for(;i > 0;--i,++it1){
//        bool found = false;
//        VAL::parameter_symbol *parameter_symbol_target = *it1;
//        for(;j > 0;--j,++it2){
//            VAL::parameter_symbol *parameter_symbol_goal = *it2;
//            if (parameter_symbol_target->getName().compare(parameter_symbol_goal->getName()) == 0) {
//                const std::string parameter_symbol_name = parameter_symbol_goal->type->getName();
//                VAL::const_symbol *parameter_symbol_final = new VAL::const_symbol(parameter_symbol_name);
//                parameter_symbol_final->type = parameter_symbol_target->type;
//                parameter_symbol_final->either_types = parameter_symbol_target->either_types;
//                final_parameter_symbol_list->push_back(parameter_symbol_final);
//                abstract = true;
//                found = true;
//            }
//        }
//        if(!found)final_parameter_symbol_list->push_back(parameter_symbol_target);
//    }
//    if (abstract) {
//        const VAL::proposition *final_proposition = new VAL::proposition(target_literal->getProp()->head,
//                                                                         final_parameter_symbol_list);
//        Literal *updated_literal = new Literal(final_proposition, target_literal->getEnv());
//        updated_literal->abstract = abstract;
//        return *updated_literal;
//    }
//    return *target_literal;
//}

LiteralSet SingleLiteralFormula::update_if_abstract_formula(vector<RPGBuilder::agent> agents_structure) const {
    LiteralSet *updated_set = new LiteralSet();
    Literal *target_literal = const_cast<Literal *>(literal);
    Literal *received_literal = update_if_abstract_literal(target_literal, agents_structure);
    updated_set->insert(received_literal);
    return *updated_set;
}

LiteralSet DisjunctiveLiteralFormula::update_if_abstract_formula(vector<RPGBuilder::agent> agents_structure) const {
    LiteralSet *updated_set = new LiteralSet();
    for(LiteralSet::const_iterator lit = disjuncts.begin(); lit != disjuncts.end(); ++lit){
        Literal *target_literal = const_cast<Literal *>(*lit);
        Literal *received_literal = update_if_abstract_literal(target_literal, agents_structure);
        updated_set->insert(received_literal);
    }
    std::cout << disjuncts.size() << std::endl;
    std::cout << updated_set->size() << std::endl;
    return *updated_set;
}

Literal *LiteralFormula::update_if_abstract_literal(Literal *target_literal, vector<RPGBuilder::agent> agents_structure) const {
            VAL::LiteralParameterIterator<VAL::parameter_symbol_list::iterator> it1 = target_literal->begin();
    int i = target_literal->getProp()->args->size();
    VAL::parameter_symbol_list *final_parameter_symbol_list = new VAL::parameter_symbol_list;
    bool abstract = false;
    for(;i > 0;--i,++it1){
        bool found = false;
        VAL::parameter_symbol *parameter_symbol_target = *it1;
        for(int j = 0; j < agents_structure.size(); j++){
            if (parameter_symbol_target->type->getName().compare(agents_structure[j].type) == 0) {
                const std::string parameter_symbol_name = parameter_symbol_target->type->getName();
                VAL::const_symbol *parameter_symbol_final = new VAL::const_symbol(parameter_symbol_name);
                parameter_symbol_final->type = parameter_symbol_target->type;
                parameter_symbol_final->either_types = parameter_symbol_target->either_types;
                final_parameter_symbol_list->push_back(parameter_symbol_final);
                abstract = true;
                found = true;
            }
        }
        if(!found)final_parameter_symbol_list->push_back(parameter_symbol_target);
    }
    if (abstract) {
        const VAL::proposition *final_proposition = new VAL::proposition(target_literal->getProp()->head,
                                                                         final_parameter_symbol_list);
        //Literal *updated_literal = new Literal(final_proposition, target_literal->getEnv());
        target_literal->abstract = abstract;
        target_literal->abstract_prop = const_cast<VAL::proposition *>(final_proposition);
        //return updated_literal;
    }
    return target_literal;
}



//TEventSet SingleLiteralFormula::update_if_abstract_event(const TEventSet events) const {
//    TEventSet* updated_set = new TEventSet();
//    TEventSet::iterator eit = events.begin();
//    TEvent event = *eit;
// //   TEvent updated_event = update_if_abstract_operator(literal,event);
//  //  updated_set->insert(updated_event);
//    return *updated_set;
//}
//
//    TEventSet DisjunctiveLiteralFormula::update_if_abstract_event(const TEventSet events) const {
//        TEventSet* updated_set = new TEventSet();
//        return* updated_set;
//    }

//TEvent LiteralFormula::update_if_abstract_operator(const Literal *literal, const TEvent event) const {
//    VAL::operator_ *operator_ = const_cast<VAL::operator_ *>(RPGBuilder::getInstantiatedOp(
//            event.get_opid())->getOp());
//    VAL::FastEnvironment* bindings = const_cast<VAL::FastEnvironment*>(RPGBuilder::getInstantiatedOp(
//            event.get_opid())->getEnv());
//    vector<VAL::const_symbol *> target_values = bindings->getCore();
//    vector<VAL::const_symbol *> root_values = literal->getEnv()->getCore();
//    bool abstract = false;
//    for (int i = 0; i < target_values.size(); i++){
//        for (int j = 0; j < root_values.size(); j++){
//            if (target_values[i]->getName().compare(root_values[j]->getName()) == 0) {
//                VAL::const_symbol *const_symbol_final = new VAL::const_symbol(root_values[j]->type->getName());
//                const_symbol_final->type = root_values[j]->type;
//                const_symbol_final->either_types = root_values[j]->either_types;
//                target_values[i] = const_symbol_final;
//                abstract = true;
//            }
//        }
//    }
//
//    Planner::time_spec ev_type = event.get_event_type();
//    TEvent* updated_event = new TEvent(event.get_opid(),ev_type);
//    std::string op_name = operator_->name->getName();
//    updated_event->abstract = abstract;
//    updated_event->op_name = op_name;
//    updated_event->values = target_values;
//    return *updated_event;
//}



TEventSet SingleLiteralFormula::update_if_abstract_event(const TEventSet events) const {

    cout << " uppp_deb 3 a" << std::endl;
    vector<RPGBuilder::agent> agents_structure;

        TEventSet* updated_set = new TEventSet();
    vector<VAL::const_symbol *> root_values = literal->getEnv()->getCore();
    bool abstract = false;

    for (int i = 0; i < root_values.size(); i++){
        for (int j = 0; j < agents_structure.size(); j++){
            if (root_values[i]->type->getName().compare(agents_structure[j].type) == 0) {
                VAL::const_symbol *const_symbol_final = new VAL::const_symbol(root_values[i]->type->getName());
                const_symbol_final->type = root_values[i]->type;
                const_symbol_final->either_types = root_values[i]->either_types;
                root_values[i] = const_symbol_final;
                abstract = true;
            }
        }
    }
    cout << " uppp_deb 3 b" << std::endl;
    TEventSet::iterator eit = events.begin();
    TEvent event = *eit;
    Planner::time_spec ev_type = event.get_event_type();
    TEvent* updated_event = new TEvent(event.get_opid(),ev_type);
    VAL::operator_ *operator_ = const_cast<VAL::operator_ *>(RPGBuilder::getInstantiatedOp(
            event.get_opid())->getOp());
    std::string op_name = operator_->name->getName();
    updated_event->abstract = abstract;
    updated_event->op_name = op_name;
    updated_event->values = root_values;
    updated_set->insert(*updated_event);
    return *updated_set;
}

TEventSet DisjunctiveLiteralFormula::update_if_abstract_event(const TEventSet events) const {
    cout << " uppp_deb 3 c" << std::endl;
        TEventSet* updated_set = new TEventSet();
    return* updated_set;
}



bool SingleLiteralFormula::subsumed_by(const LiteralFormula *other) const {
	const SingleLiteralFormula* sl = dynamic_cast<const SingleLiteralFormula*>(other);
	if (sl != 0) {
        std::stringstream ss1,ss2;
        if((abstract) && (other->abstract)){
            abstract_literal->output_to_stringstream(ss1);
            sl->abstract_literal->output_to_stringstream(ss2);
            return (ss1.str().compare(ss2.str()) == 0);
        }
        else{
            return literal == sl->get_literal();
        }
	}
	const DisjunctiveLiteralFormula* df = dynamic_cast<const DisjunctiveLiteralFormula*>(other);
	if (df != 0) {
		const LiteralSet &disjuncts = df->get_disjuncts();
		return ((disjuncts.size() == 1) && (*disjuncts.begin() == literal));
	}
	abort();
	return false;
}

bool DisjunctiveLiteralFormula::subsumed_by(const LiteralFormula *other) const {
	const SingleLiteralFormula* sl = dynamic_cast<const SingleLiteralFormula*>(other);
	if (sl != 0) {
		Literal *lit = const_cast<Literal *>(sl->get_literal());
		return (disjuncts.find(lit) != disjuncts.end());
	}
	const DisjunctiveLiteralFormula* df = dynamic_cast<const DisjunctiveLiteralFormula*>(other);
	if (df != 0) {
		for (LiteralSet::const_iterator it = df->get_disjuncts().begin(); it != df->get_disjuncts().end(); ++it) {
			if (disjuncts.find(*it) == disjuncts.end()) {
				return false;
			}
		}
		return true;
	}
	abort();
	return false;
}

bool SingleLiteralFormula::satisfied_by_state(const LiteralSet & state, const vector<double> & fluents) const
{
	Literal *lit = const_cast<Literal *>(literal);
	return state.find(lit) != state.end();
}

bool DisjunctiveLiteralFormula::satisfied_by_state(const LiteralSet & state, const vector<double> & fluents) const
{
	for (LiteralSet::const_iterator it = disjuncts.begin(); it != disjuncts.end(); ++it) {
		if (state.find(*it) != state.end())
			return true;
	}
	return false;
}

    size_t get_common_implicant(
            const TEventSet &events,
            bool get_invariant, bool get_effect,
            vector<LiteralFormula *> &common_imp) {

        map<const Literal*, size_t, LiteralLT> condition_of_count;

        for (TEventSet::const_iterator it = events.begin(); it != events.end(); ++it) {
            const TEvent &event = *it;

            const list<Literal *> &condition = event.get_conditions(get_invariant, get_effect);
            // if there's an empty condition, we will not find a common literal
            if (condition.empty())
                return 0;

            for (list<Literal *>::const_iterator lit_it = condition.begin(); lit_it != condition.end(); ++lit_it) {
                const Literal *lit = *lit_it;

                map<const Literal*, size_t, LiteralLT>::iterator map_it = condition_of_count.find(lit);
                if (map_it == condition_of_count.end()) {
                    condition_of_count.insert(make_pair(lit, 1));
                } else {
                    map_it->second++;
                }
            }
        }

        for (map<const Literal*, size_t, LiteralLT>::const_iterator common_lit_it = condition_of_count.begin();
             common_lit_it != condition_of_count.end(); ++common_lit_it) {
            const Literal *lit = common_lit_it->first;
            if (common_lit_it->second == events.size()) {
                LiteralFormula *formula = new SingleLiteralFormula(lit);
                cout << "common implicant 1";
                formula->write(cout);
                common_imp.push_back(formula);
            }
        }

        if (common_imp.size() == 0) {
            // TODO: for now, if there's no common condition, just take a disjunction of the the highest count literal from each condition


            LiteralSet disjuncts;

            for (TEventSet::const_iterator it = events.begin(); it != events.end(); ++it) {
                const TEvent &event = *it;

                const list<Literal *> &condition = event.get_conditions(get_invariant, get_effect);

                const Literal *best_literal = condition.front();

                size_t best_literal_count = condition_of_count[best_literal];

                for (list<Literal *>::const_iterator lit_it = condition.begin(); lit_it != condition.end(); ++lit_it) {
                    const Literal *lit = *lit_it;
                    if (condition_of_count[lit] > condition_of_count[best_literal]) {
                        best_literal_count = condition_of_count[lit];
                        best_literal = lit;
                    }
                }

                Literal *nclit = const_cast<Literal *>(best_literal);

//                cout << "debug common imp size 0: ";
//                nclit->write(cout);
//                cout << "endl";
                disjuncts.insert(nclit);
            }


            // improvement to the above todo: keep only the dijuncts that have the most common literal among disjuncts
            map<string,int> disjunct_count;
            for(LiteralSet::iterator lit = disjuncts.begin(); lit != disjuncts.end(); lit++){
                string prop_head = (*lit)->toProposition()->head->getName();
                map<string,int>::iterator map_it = disjunct_count.find(prop_head);
                if (map_it == disjunct_count.end()) {
                    disjunct_count[prop_head] = 1;
                } else {
                    disjunct_count[prop_head]++;
                }
            }
            string best_prop_head = "";
            int max_count = 0;
            for(map<string,int>::iterator map_it = disjunct_count.begin(); map_it != disjunct_count.end(); map_it++){
                if ((*map_it).second > max_count) {
                    best_prop_head = (*map_it).first;
                    max_count = (*map_it).second;
                }
            }
            LiteralSet best_disjuncts;
            for(LiteralSet::iterator lit = disjuncts.begin(); lit != disjuncts.end(); lit++){
                if((*lit)->toProposition()->head->getName().compare(best_prop_head) == 0){
                    best_disjuncts.insert((*lit));
                }
            }
            if(best_disjuncts.size() > 1){
                common_imp.push_back(new DisjunctiveLiteralFormula(best_disjuncts));
            }
            else{
                LiteralSet::iterator lit = best_disjuncts.begin();
                common_imp.push_back(new SingleLiteralFormula((*lit)));
            }

        }

        return common_imp.size();
    }

bool SingleLiteralFormula::operator==(const LiteralFormula &other) const {
	const SingleLiteralFormula *other_single = dynamic_cast<const SingleLiteralFormula*> (&other);
	if (other_single) {
		return literal == other_single->get_literal();
	}
//	const DisjunctiveLiteralFormula *other_disj = dynamic_cast<const DisjunctiveLiteralFormula*> (&other);
//	if (other_disj) {
//		return ((other_disj->get_disjuncts().size() == 1) &&
//				(literal == *other_disj->get_disjuncts().begin()));
//	}
	return false;
}

bool DisjunctiveLiteralFormula::operator==(const LiteralFormula &other) const {
//	const SingleLiteralFormula *other_single = dynamic_cast<const SingleLiteralFormula*> (&other);
//	if (other_single) {
//		return (other == *this);
//	}
	const DisjunctiveLiteralFormula *other_disj = dynamic_cast<const DisjunctiveLiteralFormula*> (&other);
	if (other_disj) {
		return (disjuncts.size() == other_disj->get_disjuncts().size() &&
				std::equal(disjuncts.begin(), disjuncts.end(), other_disj->get_disjuncts().begin()));
	}
	return false;
}

bool SingleLiteralFormula::operator<(const LiteralFormula &other) const {
	const SingleLiteralFormula *other_single = dynamic_cast<const SingleLiteralFormula*> (&other);
	if (other_single) {
//		std::cout << "\t" << literal->getGlobalID() << " --- " << other_single->get_literal()->getGlobalID() << std::endl;
		return literal->getGlobalID() < other_single->get_literal()->getGlobalID();
	}
	const DisjunctiveLiteralFormula *other_disj = dynamic_cast<const DisjunctiveLiteralFormula*> (&other);
	if (other_disj) {
		return true;
	}
	return false;
}

bool DisjunctiveLiteralFormula::operator<(const LiteralFormula &other) const {
	const SingleLiteralFormula *other_single = dynamic_cast<const SingleLiteralFormula*> (&other);
	if (other_single) {
		return false;
	}
	const DisjunctiveLiteralFormula *other_disj = dynamic_cast<const DisjunctiveLiteralFormula*> (&other);
	if (other_disj) {
		if (disjuncts.size() < other_disj->get_disjuncts().size()) {
			return true;
		} else if (disjuncts.size() > other_disj->get_disjuncts().size()) {
			return false;
		} else {
			LiteralSet::const_iterator it1 = disjuncts.begin();
			LiteralSet::const_iterator it2 = other_disj->get_disjuncts().begin();
			while (it1 != disjuncts.end() && it2 != other_disj->get_disjuncts().end()) {
				if ((*it1)->getGlobalID() < (*it2)->getGlobalID()) {
					return true;
				} else if ((*it1)->getGlobalID() > (*it2)->getGlobalID()) {
					return false;
				}
				++it1; ++it2;
			}
		}
	}
	return false;
}


} /* namespace temporal_landmarks */

