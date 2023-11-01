/*
 * TemporalLandmarksAndConstraints.h
 *
 *  Created on: Apr 10, 2014
 *      Author: karpase
 */

#ifndef TEMPORALLANDMARKSANDCONSTRAINTS_H_
#define TEMPORALLANDMARKSANDCONSTRAINTS_H_

#include <vector>
#include <set>
#include <map>
#include "TemporalLandmark.h"
#include "SimpleTemporalConstraint.h"
#include "TimePointVariable.h"
#include "DerivationRule.h"
#include "GoalSet.h"
//#include "bloom_filter.hpp"
#include "ProblemGenerator.h"

class MILPSolver;

using namespace strategic_tactical;

namespace temporal_landmarks {


    class TemporalLandmarksAndConstraints {
    protected:
        int lm_id = 0;
        bool unique_landmarks_only;
        std::vector<TemporalLandmark *> landmarks;
        std::map<int,std::vector<TemporalLandmark *> > landmarks_map;


        std::vector<TemporalLandmark *> ordered_landmarks;
        std::set<SimpleTemporalConstraint> constraints;
        std::set<TimePointVariable> used_time_points;
        std::map<int, std::vector<int>> timepoint_landmarks;
        std::vector<double> happenings;

        std::map<int,TemporalLandmark *> common_landmarks;
        std::map<string,GoalSet *> goal_sets_map;
        std::map<string,GoalSet *> goal_sets_map_fast;
        std::vector<pair<string,GoalSet *>> goal_sets_vector;
        int subgoal_max_size;
        int last_head_subgoal;
        ProblemGenerator* problemGenerator;

        bool emtpy = true;
        TimePointVariableFactory tpv_factory;
        TimePointVariable start_time_point, end_time_point;

        std::vector<DerivationRule *> derivation_rules;

        MILPSolver *solver;
    public:
        TemporalLandmarksAndConstraints();
        virtual ~TemporalLandmarksAndConstraints();

        string original_domain_file_name;
        string original_problem_file_name;
        double procedure_start_time;
        vector<RPGBuilder::agent> agents_structure;
        vector<string> dynamic_types_vector;
        std::chrono::time_point<std::chrono::system_clock> generate_lm_start;

        const TimePointVariable get_start_timepoint_variable() {return start_time_point;}
        const TimePointVariable get_end_timepoint_variable() {return end_time_point;}

        bool is_not_subsumed(TemporalLandmark *lm);

        void add_temporal_landmark(TemporalLandmark *lm);
        void add_temporal_constraint(SimpleTemporalConstraint constraint, TemporalLandmark *lm);
        bool is_used(TimePointVariable v) const {return used_time_points.find(v) != used_time_points.end();}
        void mark_as_used(TimePointVariable v) {used_time_points.insert(v);}
        void assign_to_timepoint(int t_id,int lm_id) {timepoint_landmarks[t_id].push_back(lm_id);}
        void compute_inherited_score(bool fast);
        bool bloom_filter_found(int parent, int child);
        bool contains_goalset(int parent,int child);
        bool have_common_goals(int parent,int child);
        void eliminate_sets_with_common_goals_and_send_to_ProblemGenerator(std::ostream & o, int decomposition);

        void output_timpepoints();
        void landmark_based_decomposition_into_subgoals(bool include_abstract,bool random, bool connected_map, std::ostream & o, std::chrono::_V2::system_clock::time_point begin, double procedure_start_time, bool all_dead_end_agent_goals_problem, string node);
        void compute_common_lm_goalsets(std::ostream & o);
        void complete_goal_sets(std::map<int,TemporalLandmark *>::iterator it, int i, GoalSet parent_set, std::ostream & o);
        void markDisjunctiveLandmarks();
        void compressAbstractDisjunctiveLandmarks();
        void update_if_abstract();
        void convert_to_vector(std::map<string,GoalSet *> map);
        std::map <int, std::vector < TemporalLandmark *>> make_identical_if_duplicate(std::map <int, std::vector < TemporalLandmark * >> map);
        TemporalLandmark* port_static_details(TemporalLandmark* target, TemporalLandmark* origin);
        void check_common_landmarks(std::ostream & o);
        void assign_weights(bool fast, std::ostream & o);

        bool isParentAgentGoal(std::string goal, vector<RPGBuilder::agent> agents_structure);
        void generate_landmarks( string node);
        void backchain_from(TemporalLandmark *lm, vector<TemporalLandmark *> chain);

        void generate_happenings(std::ostream & o) ;
        void write(std::ostream & o) ;
        std::pair<double,double> get_landmark_bounds(TemporalLandmark* lm);
        void write_lisp(std::ostream & o) ;

        // get bounds on v
        EpsilonResolutionTimestamp get_lower_bound(const TimePointVariable &v);
        EpsilonResolutionTimestamp get_upper_bound(const TimePointVariable &v);

        // get bounds on t1 - t2
        EpsilonResolutionTimestamp get_lower_bound(const TimePointVariable &t1, const TimePointVariable &t2);
        EpsilonResolutionTimestamp get_upper_bound(const TimePointVariable &t1, const TimePointVariable &t2);

        TimePointVariableFactory &get_tpv_factory() {return tpv_factory;}


    };

} /* namespace temporal_landmarks */
#endif /* TEMPORALLANDMARKSANDCONSTRAINTS_H_ */
