/*
 * DerivationRuleFirstAchieverOfFormula.cpp
 *
 *  Created on: May 15, 2014
 *      Author: karpase
 */

#include "DerivationRuleFirstAchieverOfFormula.h"
#include "TemporalStateLandmark.h"
#include "TemporalActionLandmark.h"
#include "TimePointVariable.h"
#include "TemporalLandmarksAndConstraints.h"
#include "RPGBuilder.h"

using namespace Planner;

namespace temporal_landmarks {

    DerivationRuleFirstAchieverOfFormula::DerivationRuleFirstAchieverOfFormula(
            TemporalLandmarksAndConstraints &known_landmarks_constraints_):
            DerivationRule(known_landmarks_constraints_) {
    }

    DerivationRuleFirstAchieverOfFormula::~DerivationRuleFirstAchieverOfFormula() {
    }

    const TEventSet &DerivationRuleFirstAchieverOfFormula::get_first_achievers(const LiteralFormula *formula) {
        TEventSet &achievers = first_achievers_cache[formula];
        if (achievers.size() == 0) {
//		std::cout << "getting first achievers of ";
//		formula->write(std::cout);
//		std::cout << std::endl;
            formula->get_first_achievers(achievers);
//		std::cout << " ---- " << achievers << std::endl;
        }
//	else {
//		std::cout << "saved time" << std::endl;
//	}
        return achievers;
    }

    bool DerivationRuleFirstAchieverOfFormula::apply_rule(const TemporalLandmark *lm,
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

            achievers = get_first_achievers(formula);

//		std::cout << "First achievers of: ";
//		formula->write(std::cout);
//		std::cout << " ---- " << achievers << std::endl;

            TEventSet filtered_achievers;
            for (TEventSet::const_iterator it = achievers.begin(); it != achievers.end(); ++it) {
                TEvent event = *it;
                if (event.get_earliest() <= bound) {
                    filtered_achievers.insert(event);
                }
            }
            TimePointVariable act_t = known_landmarks_constraints.get_tpv_factory().getNewTimepointVariable();
            TemporalActionLandmark *ach_lm = new TemporalActionLandmark(filtered_achievers, act_t);
            ach_lm->FirstAchiever = true;
            ach_lm->set_type();
            new_lms.push_back(ach_lm);
            new_constraints.insert(SimpleTemporalConstraint::is_less_than(act_t, tslm->get_start_time()));

            return true;
        }
        return false;
    }

} /* namespace temporal_landmarks */
