/*
 * Formula.h
 *
 *  Created on: Apr 11, 2014
 *      Author: karpase
 */

#ifndef FORMULA_H_
#define FORMULA_H_

#include "instantiation.h"
#include "../globals.h"
#include "TEvent.h"
#include <set>
#include <vector>
#include <sstream>

using namespace Inst;
using namespace Planner;


namespace temporal_landmarks {

bool literal_deleted_by(const Literal *literal, const TEvent &event);
bool literal_added_by(const Literal *literal, const TEvent &event);

class LiteralFormula {
public:
    bool abstract = false;
	LiteralFormula() {}
	virtual ~LiteralFormula() {}

    virtual void output_to_stringstream(std::stringstream & ss) const = 0;
	virtual void write(std::ostream & o) const = 0;
	virtual void write_lisp(std::ostream & o) const = 0;
	virtual size_t get_achievers(TEventSet &achievers, EpsilonResolutionTimestamp max_time) const = 0;
	virtual size_t get_first_achievers(TEventSet &achievers) const = 0;
	virtual bool subsumed_by(const LiteralFormula *other) const = 0;
	virtual bool satisfied_by_state(const LiteralSet & state, const vector<double> & fluents) const = 0;
	virtual void get_literals_in_formula_added_by_event(const TEvent &event, LiteralSet &added_literals) const = 0;

	virtual bool operator==(const LiteralFormula &other) const = 0;
	virtual bool operator<(const LiteralFormula &other) const = 0;
	virtual size_t get_num_disjuncts() const = 0;

//        virtual LiteralSet update_if_abstract_formula(const LiteralFormula *other) const = 0;
    virtual LiteralSet update_if_abstract_formula(vector<RPGBuilder::agent> agents_structure) const = 0;


//        Literal update_if_abstract_literal(Literal *target_literal, Literal *goal_literal) const;
    Literal * update_if_abstract_literal(Literal *target_literal, vector<RPGBuilder::agent> agents_structure) const;


      virtual TEventSet update_if_abstract_event(const TEventSet events) const = 0;
//    virtual TEventSet update_if_abstract_event(const TEventSet events, vector<RPGBuilder::agent> agents_structure) const = 0;


        TEvent update_if_abstract_operator(const Literal *literal, const TEvent event) const;


};

class DisjunctiveLiteralFormula: public temporal_landmarks::LiteralFormula {
protected:
    LiteralSet disjuncts;
public:

	DisjunctiveLiteralFormula(LiteralSet &disjuncts_):disjuncts(disjuncts_) {}
	virtual ~DisjunctiveLiteralFormula() {}

    LiteralSet abstract_disjuncts;

	const LiteralSet &get_disjuncts() const {return disjuncts;}

    virtual void output_to_stringstream(std::stringstream & ss) const {
	    LiteralSet current_disjuncts;
        if(abstract)current_disjuncts = abstract_disjuncts;
        else current_disjuncts = disjuncts;
        ss << "OR(";
        for (LiteralSet::const_iterator it = current_disjuncts.begin(); it != current_disjuncts.end(); ++it) {
            (*it)->output_to_stringstream(ss);
            ss << " ";
        }
        ss << ")";
	}

    virtual void make_abstract() {
        if(abstract_disjuncts.size() > 0){
            vector<string> propositions;
            for(LiteralSet::iterator lit = abstract_disjuncts.begin(); lit != abstract_disjuncts.end(); lit++){
                std::stringstream ss;
                ss << "";
                (*lit)->output_to_stringstream(ss);
                propositions.push_back(ss.str());
            }
            std::sort(propositions.begin(), propositions.end());
            propositions.erase(std::unique(propositions.begin(), propositions.end()), propositions.end());
            if(propositions.size() == 1){
                LiteralSet::iterator lit = abstract_disjuncts.begin();
                LiteralSet compressed_disjuncts;
                compressed_disjuncts.insert(*lit);
                disjuncts = compressed_disjuncts;
            }
        }
    }

	virtual void write(std::ostream & o) const {
		o << "OR(";
		for (LiteralSet::const_iterator it = disjuncts.begin(); it != disjuncts.end(); ++it) {
			(*it)->write(o);
			o << " ";
		}
		o << ")";
	}

	virtual void write_lisp(std::ostream & o) const {
		o << "(OR ";
		for (LiteralSet::const_iterator it = disjuncts.begin(); it != disjuncts.end(); ++it) {
			(*it)->write(o);
			o << " ";
		}
		o << ")";
	}

	virtual size_t get_achievers(TEventSet &achievers, EpsilonResolutionTimestamp max_time) const;
	virtual size_t get_first_achievers(TEventSet &achievers) const;
	virtual bool subsumed_by(const LiteralFormula *other) const;
	virtual bool satisfied_by_state(const LiteralSet & state, const vector<double> & fluents) const;
	virtual void get_literals_in_formula_added_by_event(const TEvent &event, LiteralSet &added_literals) const;

	virtual bool operator==(const LiteralFormula &other) const;
	virtual bool operator<(const LiteralFormula &other) const;
	virtual size_t get_num_disjuncts() const {return disjuncts.size();}

//    virtual LiteralSet update_if_abstract_formula(const LiteralFormula *other) const;
        virtual LiteralSet update_if_abstract_formula(vector<RPGBuilder::agent> agents_structure) const;


    virtual TEventSet update_if_abstract_event(const TEventSet events) const;
//    virtual TEventSet update_if_abstract_event(const TEventSet events, vector<RPGBuilder::agent> agents_structure) const;
};

class SingleLiteralFormula: public temporal_landmarks::LiteralFormula {
    const Literal *literal;
public:
    Literal *abstract_literal;
    SingleLiteralFormula(const Literal *lit) : literal(lit) {}

    virtual ~SingleLiteralFormula() {}

    const Literal *get_literal() const { return literal; }

    virtual void output_to_stringstream(std::stringstream &ss) const {
        if(abstract)abstract_literal->output_to_stringstream(ss);
        else literal->output_to_stringstream(ss);
    }

    virtual void write(std::ostream &o) const {
        if(abstract)abstract_literal->write(o);
		else literal->write(o);
	}

	virtual void write_lisp(std::ostream & o) const {
		literal->write(o);
	}

	virtual size_t get_achievers(TEventSet &achievers, EpsilonResolutionTimestamp max_time) const;
	virtual size_t get_first_achievers(TEventSet &achievers) const;
	virtual bool subsumed_by(const LiteralFormula *other) const;
	virtual bool satisfied_by_state(const LiteralSet & state, const vector<double> & fluents) const;
	virtual void get_literals_in_formula_added_by_event(const TEvent &event, LiteralSet &added_literals) const;

	virtual bool operator==(const LiteralFormula &other) const;
	virtual bool operator<(const LiteralFormula &other) const;

	virtual size_t get_num_disjuncts() const {return 1;}

//    virtual LiteralSet update_if_abstract_formula(const LiteralFormula *other) const;
	    virtual LiteralSet update_if_abstract_formula(vector<RPGBuilder::agent> agents_structure) const;

	virtual TEventSet update_if_abstract_event(const TEventSet events) const;
//    virtual TEventSet update_if_abstract_event(const TEventSet events, vector<RPGBuilder::agent> f) const;
};


size_t get_common_implicant(
		const TEventSet &events,
		bool get_invariant, bool get_effect,
		vector<LiteralFormula *> &common_imp);

} /* namespace temporal_landmarks */

#endif /* FORMULA_H_ */
