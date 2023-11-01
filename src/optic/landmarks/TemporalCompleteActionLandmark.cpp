
/*
 * TemporalCompleteActionLandmark.cpp
 *
 *  Created on: Apr 10, 2014
 *      Author: karpase
 */

#include "TemporalCompleteActionLandmark.h"
#include <algorithm>
#include <cassert>

using namespace std;

namespace temporal_landmarks {

    TemporalCompleteActionLandmark::TemporalCompleteActionLandmark(TEventSet &start_events_, TEventSet &end_events_,TimePointVariable ts_, TimePointVariable te_):
            start_events(start_events_), end_events(end_events_), ts(ts_), te(te_) {
        assert(!start_events.empty());
        assert(!end_events.empty());
    }

    TemporalCompleteActionLandmark::~TemporalCompleteActionLandmark() {
    }

    void TemporalCompleteActionLandmark::write(std::ostream & o, std::pair<double,double> bounds) const {
    }

    void TemporalCompleteActionLandmark::write_lisp(std::ostream & o) const {
    }

    bool TemporalCompleteActionLandmark::subsumed_by(const TemporalLandmark *other) const {
        const TemporalCompleteActionLandmark* talm = dynamic_cast<const TemporalCompleteActionLandmark*>(other);
        if (talm == 0)
            return false;

        for (TEventSet::const_iterator it = talm->get_events().begin(); it != talm->get_events().end(); ++it) {
            if (start_events.find(*it) == start_events.end()) {
//			write(std::cout);
//			std::cout << " not subsumbed by  ";
//			other->write(std::cout);
//			std::cout << std::endl;
                return false;
            }
        }
//	write(std::cout);
//	std::cout << " subsumbed by  ";
//	other->write(std::cout);
//	std::cout << std::endl;
        return true;
    }

    EpsilonResolutionTimestamp TemporalCompleteActionLandmark::get_earliest() const {
        EpsilonResolutionTimestamp ret = EpsilonResolutionTimestamp::infinite();
        for (TEventSet::const_iterator it = get_events().begin(); it != get_events().end(); ++it) {
            if (it->get_earliest() < ret) {
                ret = it->get_earliest();
            }
        }
        return ret;
    }

} /* namespace temporal_landmarks */
