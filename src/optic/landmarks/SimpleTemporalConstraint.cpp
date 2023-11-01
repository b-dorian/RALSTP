/*
 * SimpleTemporalConstraint.cpp
 *
 *  Created on: Apr 10, 2014
 *      Author: karpase
 */

#include "SimpleTemporalConstraint.h"

namespace temporal_landmarks {

    SimpleTemporalConstraint::SimpleTemporalConstraint(
            TimePointVariable t1_, TimePointVariable t2_,
            const EpsilonResolutionTimestamp lb_, const EpsilonResolutionTimestamp ub_):
            t1(t1_), t2(t2_), lb(lb_), ub(ub_)
    {

    }


    SimpleTemporalConstraint::~SimpleTemporalConstraint() {
    }

    void SimpleTemporalConstraint::write(std::ostream & o) const {
        if ((lb.isZero()) && (ub == EpsilonResolutionTimestamp::infinite())) {
            t2.write(o);
            o << " <= ";
            t1.write(o);
        }
        else {
            if (lb > -EpsilonResolutionTimestamp::infinite()) {
                lb.write(o);
                o << " <= ";
            }
            t1.write(o);
            o << " - ";
            t2.write(o);
            if (ub < EpsilonResolutionTimestamp::infinite()) {
                o << " <= ";
                ub.write(o);
            }
        }
    }



} /* namespace temporal_landmarks */
