/*
 * DerivationRuleFirstAchieverOfFormula.h
 *
 *  Created on: May 15, 2014
 *      Author: karpase
 */

#ifndef DERIVATIONRULEFIRSTACHIEVEROFFORMULA_H_
#define DERIVATIONRULEFIRSTACHIEVEROFFORMULA_H_

#include "DerivationRule.h"
#include "Formula.h"
#include <map>

namespace temporal_landmarks {

struct LiteralFormulaLT {

    bool operator()(const LiteralFormula* const & a, const LiteralFormula* const & b) const {
        if (!a && !b) {
            return false;
        }
        if (!a && b) {
            return true;
        }
        if (a && !b) {
            return false;
        }
//        a->write(std::cout);
//        std::cout << " < ";
//        b->write(std::cout);
//        std::cout << "  --  " << (*a < *b) << "   /   " << (*b < *a) << std::endl;
        return (*a < *b);
    }

};

class DerivationRuleFirstAchieverOfFormula: public temporal_landmarks::DerivationRule {
protected:
	std::map<const LiteralFormula *, TEventSet, LiteralFormulaLT> first_achievers_cache;

	const TEventSet &get_first_achievers(const LiteralFormula *formula);

public:
	DerivationRuleFirstAchieverOfFormula(TemporalLandmarksAndConstraints &known_landmarks_constraints_);
	virtual ~DerivationRuleFirstAchieverOfFormula();

	virtual bool apply_rule(const TemporalLandmark *lm,
			std::vector<TemporalLandmark *> &new_lms,
			std::set<SimpleTemporalConstraint> &new_constraints);
};

} /* namespace temporal_landmarks */
#endif /* DERIVATIONRULEFIRSTACHIEVEROFFORMULA_H_ */
