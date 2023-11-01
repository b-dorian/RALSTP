/*
 * DerivationRuleAchieverOfFormula.cpp
 *
 *  Created on: Apr 11, 2014
 *      Author: karpase
 */

#include "DerivationRuleAchieverOfFormula.h"
#include "TemporalStateLandmark.h"
#include "TemporalActionLandmark.h"
#include "TimePointVariable.h"
#include "TemporalLandmarksAndConstraints.h"
#include "RPGBuilder.h"

using namespace Planner;

namespace temporal_landmarks {


DerivationRuleAchieverOfFormula::DerivationRuleAchieverOfFormula(
		TemporalLandmarksAndConstraints &known_landmarks_constraints_):
		DerivationRule(known_landmarks_constraints_) {
}

DerivationRuleAchieverOfFormula::~DerivationRuleAchieverOfFormula() {
}

bool DerivationRuleAchieverOfFormula::apply_rule(const TemporalLandmark *lm,
										std::vector<TemporalLandmark *> &new_lms,
										std::set<SimpleTemporalConstraint> &new_constraints) {
	const TemporalStateLandmark* tslm = dynamic_cast<const TemporalStateLandmark*>(lm);
	if(tslm != 0) {
		LiteralSet state;
		vector<double> fluents;
		RPGBuilder::getInitialState(state, fluents);
		if (tslm->get_formula()->satisfied_by_state(state, fluents)) {
			return false;
		}
		EpsilonResolutionTimestamp bound = known_landmarks_constraints.get_upper_bound(tslm->get_start_time());
		TEventSet achievers;
		const LiteralFormula *formula = tslm->get_formula();

		formula->get_achievers(achievers, bound);
		TEventSet filtered_achievers;
		for (TEventSet::const_iterator it = achievers.begin(); it != achievers.end(); ++it) {
			TEvent event = *it;
			bool eligible = true;
			if ((event.get_event_type() == E_AT_START) && (formula->get_num_disjuncts() == 1)) {
				TEvent end_event = event.other_end();
				LiteralSet relevant_achived_literals;
				formula->get_literals_in_formula_added_by_event(event, relevant_achived_literals);
				bool all_relevant_deleted = true;
				for (LiteralSet::const_iterator lit_it = relevant_achived_literals.begin();
						lit_it != relevant_achived_literals.end(); ++lit_it) {
					if (!literal_deleted_by(*lit_it, end_event)) {
						all_relevant_deleted = false;
						break;
					}
				}
				if (all_relevant_deleted) {
					EpsilonResolutionTimestamp dur = known_landmarks_constraints.get_lower_bound(
							tslm->get_end_time(), tslm->get_start_time() );
					double max_duration = RPGBuilder::getOpMaxDuration(event.get_opid(), -1);
					EpsilonResolutionTimestamp es_max_duration(max_duration, false);
					if (es_max_duration < dur) {
						eligible = false;
					}
				}
			}

			if (eligible) {
				filtered_achievers.insert(event);
			}

		}
		TemporalActionLandmark *ach_lm = new TemporalActionLandmark(filtered_achievers, tslm->get_start_time());
        ach_lm->set_type();
		new_lms.push_back(ach_lm);

		return true;
	}
	return false;
}

} /* namespace temporal_landmarks */
