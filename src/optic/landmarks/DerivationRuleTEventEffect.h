/*
 * DerivationRuleTEventEffect.h
 *
 *  Created on: Apr 15, 2014
 *      Author: karpase
 */

#ifndef DERIVATIONRULETEVENTEFFECT_H_
#define DERIVATIONRULETEVENTEFFECT_H_

#include "DerivationRule.h"

namespace temporal_landmarks {

class DerivationRuleTEventEffect: public temporal_landmarks::DerivationRule {
public:
	DerivationRuleTEventEffect(TemporalLandmarksAndConstraints &known_landmarks_constraints_);
	virtual ~DerivationRuleTEventEffect();

	virtual bool apply_rule(const TemporalLandmark *lm,
				std::vector<TemporalLandmark *> &new_lms,
				std::set<SimpleTemporalConstraint> &new_constraints);
};

} /* namespace temporal_landmarks */
#endif /* DERIVATIONRULETEVENTEFFECT_H_ */
