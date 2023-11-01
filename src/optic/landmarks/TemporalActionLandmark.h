/*
 * TemporalActionLandmark.h
 *
 *  Created on: Apr 10, 2014
 *      Author: karpase
 */

#ifndef TEMPORALACTIONLANDMARK_H_
#define TEMPORALACTIONLANDMARK_H_

#include "TemporalLandmark.h"
#include "Formula.h"
#include <cassert>
#include <set>

namespace temporal_landmarks {



    class TemporalActionLandmark: public temporal_landmarks::TemporalLandmark {
    protected:

    public:
        bool FirstAchiever = false;
        TEventSet events;
        TimePointVariable t;
        TemporalActionLandmark(TEventSet &events_, TimePointVariable t_);
        virtual ~TemporalActionLandmark();

        virtual size_t get_num_timepoint_variables() {return 1;}
        virtual const TimePointVariable & get_timepoint_variables(size_t index) {
            assert(index < get_num_timepoint_variables());
            return t;
        }

        virtual void write(std::ostream & o, std::pair<double,double> bounds) const;
        virtual void output_to_stringstream(std::stringstream & ss, std::pair<double,double> bounds) const;
        virtual void write_lisp(std::ostream & o) const;

        void set_type();
        virtual TEventSet &get_events_non_const() {return events;}
        const TEventSet &get_events() const {return events;}
        TEventSet update_if_abstract_event(vector<RPGBuilder::agent> agents_structure);
        const TimePointVariable &get_occur_time() const {return t;}
        virtual bool subsumed_by(const TemporalLandmark *other) const;
        virtual EpsilonResolutionTimestamp get_earliest() const;

        virtual int getScore() {return 1;};
    };

} /* namespace temporal_landmarks */
#endif /* TEMPORALACTIONLANDMARK_H_ */
