/*
 * DerivationRuleTEventEffect.cpp
 *
 *  Created on: Apr 15, 2014
 *      Author: karpase
 */

#include "DerivationRuleTEventEffect.h"
#include "DerivationRuleTEventCondition.h"
#include "TemporalStateLandmark.h"
#include "TemporalActionLandmark.h"
#include "TimePointVariable.h"
#include "TemporalLandmarksAndConstraints.h"
#include "RPGBuilder.h"
#include "TEvent.h"

namespace temporal_landmarks {

DerivationRuleTEventEffect::DerivationRuleTEventEffect(
		TemporalLandmarksAndConstraints &known_landmarks_constraints_):
		DerivationRule(known_landmarks_constraints_) {
}

DerivationRuleTEventEffect::~DerivationRuleTEventEffect() {
}

bool DerivationRuleTEventEffect::apply_rule(const TemporalLandmark *lm,
			std::vector<TemporalLandmark *> &new_lms,
			std::set<SimpleTemporalConstraint> &new_constraints) {

	const TemporalActionLandmark* talm = dynamic_cast<const TemporalActionLandmark*>(lm);

	if(talm != 0) {
		const TEventSet &events = talm->get_events();

		vector<LiteralFormula *> common_cond;
		get_common_implicant(events, false, true, common_cond);

		bool found_something = false;
		for (size_t i = 0; i < common_cond.size(); i++) {
			found_something = true;
			LiteralFormula *formula = common_cond[i];
			TimePointVariable ts(known_landmarks_constraints.get_tpv_factory().getNewTimepointVariable());
			TimePointVariable te(known_landmarks_constraints.get_tpv_factory().getNewTimepointVariable());
			TemporalStateLandmark *new_lm = new TemporalStateLandmark(formula, ts, te);
            new_lm->type=2;

			new_lms.push_back(new_lm);
			new_constraints.insert(
					SimpleTemporalConstraint::is_less_than(ts, te));
			//TODO: mutex check
		}
		return found_something;
	}
	return false;
}

} /* namespace temporal_landmarks */
