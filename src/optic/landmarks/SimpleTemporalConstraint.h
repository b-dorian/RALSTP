/*
 * SimpleTemporalConstraint.h
 *
 *  Created on: Apr 10, 2014
 *      Author: karpase
 */

#ifndef SIMPLETEMPORALCONSTRAINT_H_
#define SIMPLETEMPORALCONSTRAINT_H_

#include "TimePointVariable.h"
#include "../globals.h"
#include <limits>
#include <functional>

using namespace Planner;

namespace temporal_landmarks {

// representes the constraint:  lb <= t1 - t2 <= ub
    struct SimpleTemporalConstraint {
    public:
        TimePointVariable t1, t2;
        const EpsilonResolutionTimestamp lb, ub;

        SimpleTemporalConstraint(
                TimePointVariable t1_, TimePointVariable t2_,
                const EpsilonResolutionTimestamp lb_, const EpsilonResolutionTimestamp ub_);


        virtual ~SimpleTemporalConstraint();

        void write(std::ostream & o) const;

        static SimpleTemporalConstraint is_less_than(const TimePointVariable &t1, const TimePointVariable &t2)
        {
            return SimpleTemporalConstraint(t2, t1,
                                            EpsilonResolutionTimestamp::zero(),
                                            EpsilonResolutionTimestamp::infinite());
        }

        static SimpleTemporalConstraint is_equal(const TimePointVariable &t1, const TimePointVariable &t2)
        {
            return SimpleTemporalConstraint(t2, t1,
                                            EpsilonResolutionTimestamp::zero(),
                                            EpsilonResolutionTimestamp::zero());
        }

        bool operator==(const SimpleTemporalConstraint &other) const {
            return (lb == other.lb) && (ub == other.ub) && (t1 == other.t1) && (t2 == other.t2);
        }
        bool operator!=(const SimpleTemporalConstraint &other) const {
            return !(*this == other);
        }
        bool operator<(const SimpleTemporalConstraint &other) const {
            if (lb < other.lb) return true;
            if (lb > other.lb) return false;

            if (ub < other.ub) return true;
            if (ub > other.ub) return false;

            if (t1.getID() < other.t1.getID()) return true;
            if (t1.getID() > other.t1.getID()) return false;

            if (t2.getID() < other.t2.getID()) return true;
            if (t2.getID() > other.t2.getID()) return false;

            return false;
        }
    };



} /* namespace temporal_landmarks */


#endif /* SIMPLETEMPORALCONSTRAINT_H_ */
