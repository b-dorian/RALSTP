/*
 * TemporalActionLandmark.cpp
 *
 *  Created on: Apr 10, 2014
 *      Author: karpase
 */

#include "TemporalActionLandmark.h"
#include <algorithm>
#include <cassert>

using namespace std;

namespace temporal_landmarks {

    TemporalActionLandmark::TemporalActionLandmark(TEventSet &events_, TimePointVariable t_):
            events(events_), t(t_) {
        assert(!events.empty());
    }

    TemporalActionLandmark::~TemporalActionLandmark() {
    }

    TEventSet TemporalActionLandmark::update_if_abstract_event(vector<RPGBuilder::agent> agents_structure){
        TEventSet* updated_set = new TEventSet();
        for(TEventSet::iterator eit = events.begin(); eit != events.end(); ++eit){
            TEvent event = *eit;
            Planner::time_spec ev_type = event.get_event_type();
            TEvent* updated_event = new TEvent(event.get_opid(),ev_type);
            VAL::operator_ *operator_ = const_cast<VAL::operator_ *>(RPGBuilder::getInstantiatedOp(event.get_opid())->getOp());
            VAL::FastEnvironment* bindings = const_cast<VAL::FastEnvironment*>(RPGBuilder::getInstantiatedOp(event.get_opid())->getEnv());

            vector<VAL::const_symbol *> root_values = bindings->getCore();
            bool abstract = false;

            for (int i = 0; i < root_values.size(); i++){
                for (int j = 0; j < agents_structure.size(); j++){
                    if (root_values[i]->type->getName().compare(agents_structure[j].type) == 0) {
                        VAL::const_symbol *const_symbol_final = new VAL::const_symbol(root_values[i]->type->getName());
                        const_symbol_final->type = root_values[i]->type;
                        const_symbol_final->either_types = root_values[i]->either_types;
                        root_values[i] = const_symbol_final;
                        abstract = true;
                        continue;
                    }
                }
            }
            std::string op_name = operator_->name->getName();
            updated_event->abstract = abstract;
            updated_event->op_name = op_name;
            updated_event->values = root_values;
            updated_set->insert(*updated_event);
        }
        return *updated_set;
    }

    void TemporalActionLandmark::write(std::ostream & o, std::pair<double,double> bounds) const {

        int correct_id = id;
        std::string timepoint_location = "(e)";
        if(events.size() > 1)o << "Disjunctive\n";
        if(FirstAchiever)o << "First Achiever\n";
        if (type==4){
            correct_id = id+1;
            timepoint_location = "(s)";

            if(abstract)o << "---------A-END LM-----------\n";
            else o << "-----------END LM-----------\n";
        }
        else{
            if(abstract)o << "---------A-START LM-----------\n";
            else o << "-----------START LM-----------\n";
        }

        if(abstract) o << "A-";
        o << "LM-" << id << " | Root: Goal-" << rootGoal << " | Parent: LM-" << parentLandmark->id << "| Derivation Rule: " << rule << "\nACTION | Timepoint" << timepoint_location << ": t" << t.getID() << " = " << bounds.first << " | ";
        o << "Disjunctions: " << events.size() << " | OCCURS (";
        o << events;
        o << ")";


    }

    void TemporalActionLandmark::output_to_stringstream(std::stringstream & ss, std::pair<double,double> bounds) const {
        int correct_id = id;
        std::string timepoint_location = "(e)";
        if(events.size() > 1)ss << "Disjunctive\n";
        if(FirstAchiever)ss << "First Achiever\n";
        if (type==4){
            correct_id = id+1;
            timepoint_location = "(s)";
            if(abstract)ss << "---------A-END LM-----------\n";
            else ss << "-----------END LM-----------\n";
        }
        else{
            if(abstract)ss << "---------A-START LM-----------\n";
            else ss << "-----------START LM-----------\n";
        }

        if(abstract) ss << "A-";
        ss << "LM-" << id << " | Root: Goal-" << rootGoal << " | Parent: LM-" << parentLandmark->id << "\nACTION | Timepoint" << timepoint_location << ": t" << t.getID() << " = " << bounds.first << " | ";
        ss << "Disjunctions: " << events.size() << " | OCCURS (";
        ss << events;
        ss << ")";
    }

    void TemporalActionLandmark::set_type(){
        TEventSet::const_iterator it = events.begin();
        for (; it != events.end(); ++it) {
            if (it->get_event_type() == E_AT_START)
                type = 3;
            else if (it->get_event_type() == E_AT_END)
                type = 4;
        }
    }

    void TemporalActionLandmark::write_lisp(std::ostream & o) const {
        o << "LM-" << id << " | Timepoint: t" << t.getID() << " | (OCCURS";
        TEventSet::const_iterator it = events.begin();
        for (; it != events.end(); ++it) {
            o << "( ";
            if (it->get_event_type() == E_AT_START)
                o << "START ";
            else if (it->get_event_type() == E_AT_END)
                o << "END ";
            RPGBuilder::getInstantiatedOp(it->get_opid())->write(o);
            o << ") ";
        }
        o << ")";
        o << std::endl;
        for (set<SimpleTemporalConstraint>::const_iterator it = constraints.begin(); it != constraints.end(); ++it)	{
            if(it->lb.toDouble() >= 0){
                it->write(o);
                o << std::endl;
            }
        }
    }

    bool TemporalActionLandmark::subsumed_by(const TemporalLandmark *other) const {
        const TemporalActionLandmark* talm = dynamic_cast<const TemporalActionLandmark*>(other);
        if (talm == 0)
            return false;

        for (TEventSet::const_iterator it = talm->get_events().begin(); it != talm->get_events().end(); ++it) {
            TEvent other_event = *it;
            for (TEventSet::const_iterator it2 = events.begin(); it2 != events.end(); ++it2) {
                TEvent event = *it2;
                if ((event.abstract) && (other_event.abstract)){
                    std::stringstream ss1,ss2;
                    event.output_to_stringstream(ss1);
                    other_event.output_to_stringstream(ss2);
                    return (ss1.str().compare(ss2.str()) == 0);
                    //std::cout << endl << abstract << endl;
                }
                else{
                    if (events.find(other_event) == events.end()) {
//			write(std::cout);
//			std::cout << " not subsumbed by  ";
//			other->write(std::cout);
//			std::cout << std::endl;
                        return false;
                    }
                }
            }
        }
//	write(std::cout);
//	std::cout << " subsumbed by  ";
//	other->write(std::cout);
//	std::cout << std::endl;
        return true;
    }

    EpsilonResolutionTimestamp TemporalActionLandmark::get_earliest() const {
        EpsilonResolutionTimestamp ret = EpsilonResolutionTimestamp::infinite();
        for (TEventSet::const_iterator it = get_events().begin(); it != get_events().end(); ++it) {
            if (it->get_earliest() < ret) {
                ret = it->get_earliest();
            }
        }
        return ret;
    }


} /* namespace temporal_landmarks */
