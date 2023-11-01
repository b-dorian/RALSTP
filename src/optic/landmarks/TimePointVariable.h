/*
 * TimePointVariable.h
 *
 *  Created on: Apr 10, 2014
 *      Author: karpase
 */

#ifndef TIMEPOINTVARIABLE_H_
#define TIMEPOINTVARIABLE_H_

#include <fstream>
#include <iostream>
#include "Formula.h"

namespace temporal_landmarks {

    struct TimePointVariable {
    protected:

    public:
        int id;
        int lb_id, ub_id;
        double lb_value, ub_value;

        TimePointVariable(int id_);
        virtual ~TimePointVariable();

        int getID() const {return id;}

        bool operator==(const TimePointVariable &other) const {return id == other.getID();}
        bool operator!=(const TimePointVariable &other) const {return id != other.getID();}
        bool operator<(const TimePointVariable &other) const {return id < other.getID();}

        void write(std::ostream & o) const;
    };

    class TimePointVariableFactory {
    protected:
        int last_id;
    public:
        TimePointVariableFactory();
        virtual ~TimePointVariableFactory();

        TimePointVariable getNewTimepointVariable();
        int get_last_id() const {return last_id;}
    };






} /* namespace temporal_landmarks */
#endif /* TIMEPOINTVARIABLE_H_ */