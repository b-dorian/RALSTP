/*
 * TemporalStateLandmark.cpp
 *
 *  Created on: Apr 10, 2014
 *      Author: karpase
 */

#include "TemporalStateLandmark.h"

namespace temporal_landmarks {

    TemporalStateLandmark::TemporalStateLandmark(LiteralFormula *formula_, TimePointVariable ts_, TimePointVariable te_):
            formula(formula_), ts(ts_), te(te_)
    {
        assert(formula != NULL);
    }

    TemporalStateLandmark::~TemporalStateLandmark() {
    }

    void TemporalStateLandmark::write(std::ostream & o, std::pair<double,double> bounds) const {
        assert(formula != NULL);
        if(formula->get_num_disjuncts() > 1)o << "Disjunctive\n";
        if(type==1)o << "-----------GOAL LM-----------\n";
        if(type==2){
            if(abstract)o << "---------A-FACT LM-----------\n";
            else o << "-----------FACT LM-----------\n";
        }

        if(abstract) o << "A-";
        o << "LM-" << id << " | Root: Goal-" << rootGoal << " | Parent: LM-" << parentLandmark->id << " | Derivation Rule: " << rule << "\nFACT | Timepoint(s): t" << ts.getID() << " = " << bounds.first << " | Timepoint(e): t"  << te.getID() << " = " << bounds.second << " | ";
        o << "Disjunctions: " << formula->get_num_disjuncts() << " | HOLDS (";
        formula->write(o);
        o << ")";

    }
    //TODO replace output_to_stringstream to:
    //    std::stringstream ss;
    //    ss << o.rdbuf();
    void TemporalStateLandmark::output_to_stringstream(std::stringstream & ss, std::pair<double,double> bounds) const {
        assert(formula != NULL);
        if(formula->get_num_disjuncts() > 1)ss << "Disjunctive\n";
        if(type==1)ss << "-----------GOAL LM-----------\n";
        if(type==2){
            if(abstract)ss << "---------A-FACT LM-----------\n";
            else ss << "-----------FACT LM-----------\n";
        }

        if(abstract) ss << "A-";
        ss << "LM-" << id << " | Root: Goal-" << rootGoal << " | Parent: LM-" << parentLandmark->id << "\nFACT | Timepoint(s): t" << ts.getID() << " = " << bounds.first << " | Timepoint(e): t"  << te.getID() << " = " << bounds.second << " | ";
        ss << "Disjunctions: " << formula->get_num_disjuncts() << " | HOLDS (";
        formula->output_to_stringstream(ss);
        ss << ")";
    }

    void TemporalStateLandmark::write_lisp(std::ostream & o) const {
        assert(formula != NULL);
        o << "LM-" << id << " | Timepoint(s): t" << ts.getID() << " | Timepoint(e): t" << te.getID() << " | (HOLDS ";
        formula->write_lisp(o);
        o << ")";
        o << std::endl;
        for (set<SimpleTemporalConstraint>::const_iterator it = constraints.begin(); it != constraints.end(); ++it)	{
            if(it->lb.toDouble() >= 0){
                it->write(o);
                o << std::endl;
            }

        }
    }

    bool TemporalStateLandmark::subsumed_by(const TemporalLandmark *other) const {
        const TemporalStateLandmark* tslm = dynamic_cast<const TemporalStateLandmark*>(other);
        if (tslm == 0)
            return false;
        return formula->subsumed_by(tslm->get_formula());
    }



} /* namespace temporal_landmarks */
