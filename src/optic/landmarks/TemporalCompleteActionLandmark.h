/*
 * TemporalCompleteActionLandmark.h
 *
 *  Created on: Apr 10, 2014
 *      Author: karpase
 */

#ifndef TEMPORALCOMPLETEACTIONLANDMARK_H_
#define TEMPORALCOMPLETEACTIONLANDMARK_H_

#include "TemporalLandmark.h"
#include "Formula.h"
#include <cassert>
#include <set>

namespace temporal_landmarks {



    class TemporalCompleteActionLandmark: public temporal_landmarks::TemporalLandmark {
    protected:
    public:
        TEventSet start_events;
        TEventSet end_events;
        TimePointVariable ts, te;
        TemporalCompleteActionLandmark(TEventSet &start_events_, TEventSet &end_events_, TimePointVariable ts_, TimePointVariable te_);
        virtual ~TemporalCompleteActionLandmark();

        virtual size_t get_num_timepoint_variables() {return 2;}
        virtual const TimePointVariable & get_timepoint_variables(size_t index) {
            assert(index < get_num_timepoint_variables());
            if (index == 0) return ts;
            else return te;
        }

        virtual void write(std::ostream & o, std::pair<double,double> bounds) const;
        virtual void write_lisp(std::ostream & o) const;

        const TEventSet &get_events() const {return start_events;}
        virtual TEventSet &get_events_non_const() {return start_events;}
        TEventSet &get_start_events() {return start_events;}
        TEventSet &get_end_events() {return end_events;}
        const TimePointVariable &get_start_time() const {return ts;}
        const TimePointVariable &get_end_time() const {return te;}
        virtual bool subsumed_by(const TemporalLandmark *other) const;
        virtual EpsilonResolutionTimestamp get_earliest() const;
    };

} /* namespace temporal_landmarks */
#endif /* TEMPORALACTIONLANDMARK_H_ */
