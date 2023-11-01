/*
 * Event.cpp
 *
 *  Created on: Apr 11, 2014
 *      Author: karpase
 */

#include "TEvent.h"

#include "RPGBuilder.h"
#include "globals.h"

using namespace Planner;

namespace temporal_landmarks {

void TEvent::write(std::ostream &o) const {
	switch (get_event_type()) {
		case E_AT_START:
			o << "START";
			break;
		case E_AT_END:
			o << "END";
			break;
		default:
			assert(false);
			break;
	}
    if(abstract){
        o << "(";
        o << op_name << " ";
        for (int i = 0; i < values.size(); i++){
            o << values[i]->getName();
            if(i != values.size()-1)o << " ";
        }
        o << ")";
    }
    else{
	assert(RPGBuilder::getInstantiatedOp(get_opid()) != NULL);
	RPGBuilder::getInstantiatedOp(get_opid())->write(o);
    }
}

    void TEvent::output_to_stringstream(std::stringstream &ss) const {
        switch (get_event_type()) {
            case E_AT_START:
                ss << "START";
                break;
            case E_AT_END:
                ss << "END";
                break;
            default:
                assert(false);
                break;
        }
        if(abstract){
            ss << "(";
            ss << op_name << " ";
            for (int i = 0; i < values.size(); i++){
                ss << values[i]->getName();
                if(i != values.size()-1)ss << " ";
            }
            ss << ")";
        }
        else{
            assert(RPGBuilder::getInstantiatedOp(get_opid()) != NULL);
            RPGBuilder::getInstantiatedOp(get_opid())->write(ss);
        }
    }


std::ostream & operator<<(std::ostream &o, const TEventSet & ev) {
	TEventSet::const_iterator it = ev.begin();
	if (it != ev.end()) {
		it->write(o);
		++it;
	}
	for (; it != ev.end(); ++it) {
		o << " ";
		it->write(o);
	}
	return o;
}

EpsilonResolutionTimestamp TEvent::get_earliest() const {
	switch (get_event_type()) {
		case E_AT_START:
			return RPGHeuristic::getEarliestForStarts()[get_opid()];
		case E_AT_END:
			return RPGHeuristic::getEarliestForEnds()[get_opid()];
		default:
			assert(false);
			break;
	}
	return EpsilonResolutionTimestamp::undefined();
}

    const list<Literal *> &TEvent::get_conditions(bool get_invariant, bool get_effect) const {
        assert(!(get_invariant && get_effect));
        if (get_invariant) {
            return RPGBuilder::getInvariantPropositionalPreconditions()[get_opid()];
        }
        if (get_effect) {
            // TODO: what to do about delete effects?
            switch (get_event_type()) {
                case E_AT_START:
                    return RPGBuilder::getStartPropositionAdds()[get_opid()];
                case E_AT_END:
                    return RPGBuilder::getEndPropositionAdds()[get_opid()];
                default:
                    assert(false);
                    break;
            }
        } else {
            switch (get_event_type()) {
                case E_AT_START:
                    return RPGBuilder::getStartPropositionalPreconditions()[get_opid()];
                case E_AT_END:
                    return RPGBuilder::getEndPropositionalPreconditions()[get_opid()];
                default:
                    assert(false);
                    break;
            }
        }
        return RPGBuilder::getStartPropositionalPreconditions()[get_opid()];
    }

TEvent TEvent::other_end() const {
	switch (get_event_type()) {
		case E_AT_START:
			return TEvent(get_opid(), E_AT_END);
			break;
		case E_AT_END:
			return TEvent(get_opid(), E_AT_START);
			break;
		default:
			assert(false);
			break;
	}
	return TEvent(0, E_AT_END);
}


} /* namespace temporal_landmarks */
