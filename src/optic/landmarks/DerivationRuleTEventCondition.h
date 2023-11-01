/*
 * TEventConditionDerivationRule.h
 *
 *  Created on: Apr 14, 2014
 *      Author: karpase
 */

#ifndef TEVENTCONDITIONDERIVATIONRULE_H_
#define TEVENTCONDITIONDERIVATIONRULE_H_

#include "DerivationRule.h"

namespace temporal_landmarks {

class DerivationRuleTEventCondition: public temporal_landmarks::DerivationRule {
public:
	DerivationRuleTEventCondition(TemporalLandmarksAndConstraints &known_landmarks_constraints_);
	virtual ~DerivationRuleTEventCondition();

	virtual bool apply_rule(const TemporalLandmark *lm,
				std::vector<TemporalLandmark *> &new_lms,
				std::set<SimpleTemporalConstraint> &new_constraints);
};

} /* namespace temporal_landmarks */
#endif /* TEVENTCONDITIONDERIVATIONRULE_H_ */
