/*
 * TemporalLandmark.h
 *
 *  Created on: Apr 10, 2014
 *      Author: karpase
 */

#ifndef TEMPORALLANDMARK_H_
#define TEMPORALLANDMARK_H_

#include "TimePointVariable.h"
#include "SimpleTemporalConstraint.h"

#include <fstream>
#include <iostream>

#include "../globals.h"

namespace temporal_landmarks {

    class TemporalLandmark {

    public:
        int rootGoal;
        TemporalLandmark *rootLandmark;
        TemporalLandmark *parentLandmark;
        bool abstract = false;
        bool disjunctive = false;
        int type;
        int id;
        string rule;
        std::set<SimpleTemporalConstraint> constraints;
        TemporalLandmark();

        int no_of_goals = 0;
        vector<int> goals_containing_landmark;
        string goalset;

        virtual ~TemporalLandmark();
        LiteralFormula *formula;
        TEventSet events;

        virtual int getScore() = 0;

        virtual size_t get_num_timepoint_variables() = 0;
        virtual TEventSet &get_events_non_const() = 0;
        virtual const TimePointVariable & get_timepoint_variables(size_t index) = 0;
        virtual void write(std::ostream & o, std::pair<double,double> bounds) const = 0;
        virtual void output_to_stringstream(std::stringstream & ss, std::pair<double,double> bounds) const = 0;
        virtual void write_lisp(std::ostream & o) const = 0;
        virtual bool subsumed_by(const TemporalLandmark *other) const = 0;
        virtual Planner::EpsilonResolutionTimestamp get_earliest() const = 0;
    };



} /* namespace temporal_landmarks */
#endif /* TEMPORALLANDMARK_H_ */
