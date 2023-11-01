/*
 * TimePointVariable.cpp
 *
 *  Created on: Apr 10, 2014
 *      Author: karpase
 */

#include "TimePointVariable.h"


namespace temporal_landmarks {


    TimePointVariableFactory::TimePointVariableFactory() {
        last_id = 0;
    }

    TimePointVariableFactory::~TimePointVariableFactory() {

    }

    TimePointVariable TimePointVariableFactory::getNewTimepointVariable() {
        return TimePointVariable(last_id++);
    }



    TimePointVariable::TimePointVariable(int id_):id(id_) {
    }

    TimePointVariable::~TimePointVariable() {
    }

    void TimePointVariable::write(std::ostream & o) const {
        o << "t" << id;
    }






} /* namespace temporal_landmarks */
