/*
 * DerivationRuleActionStartEnd.cpp
 *
 *  Created on: Apr 14, 2014
 *      Author: karpase
 */

#include "DerivationRuleActionStartEnd.h"
#include "DerivationRuleTEventCondition.h"
#include "TemporalStateLandmark.h"
#include "TemporalActionLandmark.h"
#include "TimePointVariable.h"
#include "TemporalLandmarksAndConstraints.h"
#include "RPGBuilder.h"
#include "TEvent.h"
#include "globals.h"
#include <limits>
#include <cassert>
#include <vector>

using namespace std;
using namespace Planner;

namespace temporal_landmarks {

DerivationRuleActionStartEnd::DerivationRuleActionStartEnd(
		TemporalLandmarksAndConstraints &known_landmarks_constraints_):
				DerivationRule(known_landmarks_constraints_) {
}

DerivationRuleActionStartEnd::~DerivationRuleActionStartEnd() {
}

bool DerivationRuleActionStartEnd::apply_rule(const TemporalLandmark *lm,
			std::vector<TemporalLandmark *> &new_lms,
			std::set<SimpleTemporalConstraint> &new_constraints)
{
	const TemporalActionLandmark* talm = dynamic_cast<const TemporalActionLandmark*>(lm);

	if(talm != 0) {
		TEventSet other_end_events;
		bool all_start = true;
		bool all_end = true;
		EpsilonResolutionTimestamp max_dur_end = EpsilonResolutionTimestamp::zero();
		EpsilonResolutionTimestamp max_dur_start = EpsilonResolutionTimestamp::zero();
		EpsilonResolutionTimestamp min_dur_end = EpsilonResolutionTimestamp::infinite();
		EpsilonResolutionTimestamp min_dur_start = EpsilonResolutionTimestamp::infinite();

		const TEventSet &events = talm->get_events();
		const TimePointVariable &event_time = talm->get_occur_time();

		for (TEventSet::const_iterator it = events.begin(); it != events.end(); ++it) {
			const TEvent &event = *it;
			EpsilonResolutionTimestamp min_duration = EpsilonResolutionTimestamp(
					RPGBuilder::getOpMinDuration(event.get_opid(), -1), false);
			EpsilonResolutionTimestamp max_duration = EpsilonResolutionTimestamp(
					RPGBuilder::getOpMaxDuration(event.get_opid(), -1), false);

			if (event.get_event_type() == E_AT_START) {
				all_end = false;
				if (min_duration < min_dur_start) {min_dur_start = min_duration;}
				if (max_duration > max_dur_start) {max_dur_start = max_duration;}
			}
			if (event.get_event_type() == E_AT_END) {
				all_start = false;
				if (min_duration < min_dur_end) {min_dur_end = min_duration;}
				if (max_duration > max_dur_end) {max_dur_end = max_duration;}
			}
			other_end_events.insert(event.other_end());
		}

		TimePointVariable other_end_timepoint(known_landmarks_constraints.get_tpv_factory().getNewTimepointVariable());

		TemporalActionLandmark *other_end_lm = new TemporalActionLandmark(other_end_events, other_end_timepoint);
        other_end_lm->set_type();
		new_lms.push_back(other_end_lm);
		if (all_end) {
			new_constraints.insert(
					SimpleTemporalConstraint(event_time, other_end_timepoint,
							min_dur_end, max_dur_end));
		} else if (all_start) {
			new_constraints.insert(
					SimpleTemporalConstraint(other_end_timepoint, event_time,
							min_dur_start,
							max_dur_start));
		} else {
			assert(!all_end);
			assert(!all_start);

			new_constraints.insert(
					SimpleTemporalConstraint(event_time, other_end_timepoint,
							-EpsilonResolutionTimestamp::infinite(), max_dur_end));
			new_constraints.insert(
					SimpleTemporalConstraint(other_end_timepoint, event_time,
							-EpsilonResolutionTimestamp::infinite(), max_dur_start));
		}

		vector<LiteralFormula *> common_inv;
		get_common_implicant(events, true, false, common_inv);
		for (size_t i = 0; i < common_inv.size(); i++) {
			TimePointVariable inv_start(known_landmarks_constraints.get_tpv_factory().getNewTimepointVariable());
			TimePointVariable inv_end(known_landmarks_constraints.get_tpv_factory().getNewTimepointVariable());

			LiteralFormula *inv_formula = common_inv[i];

			TemporalStateLandmark *invariant_lm = new TemporalStateLandmark(inv_formula, inv_start, inv_end);
			invariant_lm->type=2;

			new_lms.push_back(invariant_lm);

			if (all_end) {
				new_constraints.insert(
						SimpleTemporalConstraint::is_less_than(inv_start, other_end_timepoint));
				new_constraints.insert(
						SimpleTemporalConstraint(event_time, inv_end,
								EpsilonResolutionTimestamp::zero(),
								EpsilonResolutionTimestamp::epsilon() ));
			} else if (all_start) {
				new_constraints.insert(
						SimpleTemporalConstraint::is_less_than(inv_start, event_time));
				new_constraints.insert(
						SimpleTemporalConstraint(other_end_timepoint, inv_end,
								EpsilonResolutionTimestamp::zero(),
								EpsilonResolutionTimestamp::epsilon() ));
			} else {
				assert(!all_end);
				assert(!all_start);

//				new_constraints.insert(
//						SimpleTemporalConstraint(inv_end, event_time,
//								min_dur_end, EpsilonResolutionTimestamp::infinite()));
				new_constraints.insert(
						SimpleTemporalConstraint::is_less_than(inv_start, event_time));
				new_constraints.insert(
						SimpleTemporalConstraint::is_less_than(event_time, inv_end));
				if (min_dur_end < min_dur_start) {
					new_constraints.insert(
						SimpleTemporalConstraint(inv_end, inv_start,
								min_dur_end, EpsilonResolutionTimestamp::infinite()));
				} else {
					new_constraints.insert(
							SimpleTemporalConstraint(inv_end, inv_start,
									min_dur_start, EpsilonResolutionTimestamp::infinite()));
				}
			}
		}

		return true;
	}
	return false;
}



} /* namespace temporal_landmarks */
