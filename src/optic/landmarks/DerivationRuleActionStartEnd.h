/*
 * DerivationRuleActionStartEnd.h
 *
 *  Created on: Apr 14, 2014
 *      Author: karpase
 */

#ifndef DERIVATIONRULEACTIONSTARTEND_H_
#define DERIVATIONRULEACTIONSTARTEND_H_

#include "DerivationRule.h"

namespace temporal_landmarks {

class DerivationRuleActionStartEnd: public temporal_landmarks::DerivationRule {
public:
	DerivationRuleActionStartEnd(TemporalLandmarksAndConstraints &known_landmarks_constraints_);
	virtual ~DerivationRuleActionStartEnd();

	virtual bool apply_rule(const TemporalLandmark *lm,
				std::vector<TemporalLandmark *> &new_lms,
				std::set<SimpleTemporalConstraint> &new_constraints);
};

} /* namespace temporal_landmarks */
#endif /* DERIVATIONRULEACTIONSTARTEND_H_ */
