/*
 * Event.h
 *
 *  Created on: Apr 11, 2014
 *      Author: karpase
 */

#ifndef TEVENT_H_
#define TEVENT_H_

#include "../globals.h"
#include "../RPGBuilder.h"

using namespace Planner;

namespace temporal_landmarks {

struct TEvent {
private:
	int op_id;
	Planner::time_spec ev_type;
public:
    std::string op_name;
    VAL::var_symbol_list* keys;
    vector<VAL::const_symbol *> values;
    bool abstract = false;
	TEvent(int op_id_, Planner::time_spec type_):op_id(op_id_), ev_type(type_) {}

	int get_opid() const {return op_id;}
	Planner::time_spec get_event_type() const {return ev_type;}

	bool operator==(const TEvent &other) const {
		return ((get_opid()==other.get_opid()) &&
			(get_event_type()==other.get_event_type()));
	}

	bool operator!=(const TEvent &other) const {return !(*this == other);}
	bool operator<(const TEvent &other) const {
		if (get_opid() < other.get_opid())
			return true;
		if (get_opid() > other.get_opid())
			return false;
		if (get_opid() == other.get_opid())
			return get_event_type() < other.get_event_type();
		abort();
		return false;
	}


	TEvent other_end() const;
	void write(std::ostream &o) const;
    void output_to_stringstream(std::stringstream &ss) const;

	EpsilonResolutionTimestamp get_earliest() const;
	const list<Literal *> &get_conditions(bool get_invariant, bool get_effect) const;
};

typedef std::set<TEvent> TEventSet;

std::ostream & operator<<(std::ostream &o, const TEventSet & ev);










} /* namespace temporal_landmarks */
#endif /* EVENT_H_ */
