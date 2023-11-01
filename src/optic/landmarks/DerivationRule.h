/*
 * DerivationRule.h
 *
 *  Created on: Apr 11, 2014
 *      Author: karpase
 */

#ifndef DERIVATIONRULE_H_
#define DERIVATIONRULE_H_

#include <vector>
#include "TemporalLandmark.h"
#include "SimpleTemporalConstraint.h"

namespace temporal_landmarks {

class TemporalLandmarksAndConstraints;

class DerivationRule {
protected:
	TemporalLandmarksAndConstraints &known_landmarks_constraints;
public:
	DerivationRule(TemporalLandmarksAndConstraints &known_landmarks_constraints_);
	virtual ~DerivationRule();

	virtual bool apply_rule(const TemporalLandmark *lm,
			std::vector<TemporalLandmark *> &new_lms,
			std::set<SimpleTemporalConstraint> &new_constraints) = 0;
};

} /* namespace temporal_landmarks */
#endif /* DERIVATIONRULE_H_ */
