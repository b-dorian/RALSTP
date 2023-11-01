/*
 * DerivationRuleAchieverOfFormula.h
 *
 *  Created on: Apr 11, 2014
 *      Author: karpase
 */

#ifndef DERIVATIONRULEACHIEVEROFFORMULA_H_
#define DERIVATIONRULEACHIEVEROFFORMULA_H_

#include "DerivationRule.h"

namespace temporal_landmarks {

class DerivationRuleAchieverOfFormula: public temporal_landmarks::DerivationRule {
public:
	DerivationRuleAchieverOfFormula(TemporalLandmarksAndConstraints &known_landmarks_constraints_);
	virtual ~DerivationRuleAchieverOfFormula();

	virtual bool apply_rule(const TemporalLandmark *lm,
			std::vector<TemporalLandmark *> &new_lms,
			std::set<SimpleTemporalConstraint> &new_constraints);
};

} /* namespace temporal_landmarks */
#endif /* DERIVATIONRULEACHIEVEROFFORMULA_H_ */
