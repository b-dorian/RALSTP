/*
 * TemporalStateLandmark.h
 *
 *  Created on: Apr 10, 2014
 *      Author: karpase
 */

#ifndef TEMPORALSTATELANDMARK_H_
#define TEMPORALSTATELANDMARK_H_

#include "TemporalLandmark.h"
#include <vector>
#include <cassert>
#include "Formula.h"

namespace temporal_landmarks {

    class TemporalStateLandmark: public TemporalLandmark {
    protected:

    public:
        TEventSet events;
        LiteralFormula *formula;
        TimePointVariable ts, te;
        TemporalStateLandmark(LiteralFormula *formula_, TimePointVariable ts_, TimePointVariable te_);

        virtual ~TemporalStateLandmark();

        virtual size_t get_num_timepoint_variables() {return 2;}
        virtual const TimePointVariable & get_timepoint_variables(size_t index) {
            assert(index < get_num_timepoint_variables());
            if (index == 0) return ts;
            else return te;
        }

        virtual void write(std::ostream & o, std::pair<double,double> bounds) const;
        virtual void output_to_stringstream(std::stringstream & ss, std::pair<double,double> bounds) const;
        virtual void write_lisp(std::ostream & o) const;
        virtual TEventSet &get_events_non_const() {return events;}
        const LiteralFormula * get_formula() const {return formula;}
        const TimePointVariable &get_start_time() const {return ts;}
        const TimePointVariable &get_end_time() const {return te;}
        virtual bool subsumed_by(const TemporalLandmark *other) const;
        virtual EpsilonResolutionTimestamp get_earliest() const {return EpsilonResolutionTimestamp::zero();}

        virtual int getScore(){return 2;};
    };

} /* namespace temporal_landmarks */
#endif /* TEMPORALSTATELANDMARK_H_ */
