/*
 * DerivationRule.cpp
 *
 *  Created on: Apr 11, 2014
 *      Author: karpase
 */

#include "DerivationRule.h"

namespace temporal_landmarks {

DerivationRule::DerivationRule(TemporalLandmarksAndConstraints &known_landmarks_constraints_):
		known_landmarks_constraints(known_landmarks_constraints_)
{

}

DerivationRule::~DerivationRule() {
}

} /* namespace temporal_landmarks */
