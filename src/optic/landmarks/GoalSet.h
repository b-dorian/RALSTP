//
// Created by Dorian Buksz on 25/04/2020.
//

#ifndef TEMPORAL_LANDMARKS_GoalSetS_H
#define TEMPORAL_LANDMARKS_GoalSetS_H


//#include "TemporalActionLandmark.h"
//#include "TemporalStateLandmark.h"
#include "TemporalLandmark.h"

using namespace Planner;
using namespace std;

namespace temporal_landmarks {
    class GoalSet final{
    private:
        const string name;
        const GoalSet* parent_set;
        const vector<int> goals_in_set;
        const bool parent;
        bool contained;
        long int normal_fact_lm_score;
        long int normal_action_lm_score;
        long int abstract_fact_lm_score;
        long int abstract_action_lm_score;
        long int inherited_normal_fact_lm_score;
        long int inherited_normal_action_lm_score;
        long int inherited_abstract_fact_lm_score;
        long int inherited_abstract_action_lm_score;
        vector<TemporalLandmark *> landmarks;
    public:
        //child constructor
        GoalSet(GoalSet* parent_set_,string name_,vector<int>goals_in_set_) :
        parent_set(parent_set_),name(name_),goals_in_set(goals_in_set_),
        normal_fact_lm_score(0),normal_action_lm_score(0),abstract_fact_lm_score(0),abstract_action_lm_score(0),
        inherited_normal_fact_lm_score(0),inherited_normal_action_lm_score(0),inherited_abstract_fact_lm_score(0),inherited_abstract_action_lm_score(0),
        contained(false),parent(false){}
        //parent constructor
        GoalSet(string name_,vector<int>goals_in_set_) : name(name_),goals_in_set(goals_in_set_),
        normal_fact_lm_score(0),normal_action_lm_score(0),abstract_fact_lm_score(0),abstract_action_lm_score(0),
        inherited_normal_fact_lm_score(0),inherited_normal_action_lm_score(0),inherited_abstract_fact_lm_score(0),inherited_abstract_action_lm_score(0),
        contained(false),parent(true){}
        virtual ~GoalSet() {}

        //getters
        bool isParent() {return parent;};
        bool isContained(){return contained;};
        const GoalSet* getParent(){return parent_set;};
        string getName(){return name;};
        vector<int> getGoalsInSet(){return goals_in_set;};
        vector<TemporalLandmark *> getLandmarks(){return landmarks;};
        long int getNormalFactScore(){return normal_fact_lm_score;};
        long int getNormalActionScore(){return normal_action_lm_score;};
        long int getAbstractFactScore(){return abstract_fact_lm_score;};
        long int getAbstractActionScore(){return abstract_action_lm_score;};
        long int getNormalandAbstractScore(){return getNormalScore() + getAbstractScore();};
        long int getNormalScore(){return normal_fact_lm_score + normal_action_lm_score;};
        long int getAbstractScore(){return abstract_fact_lm_score + abstract_action_lm_score;};
        long int getInheritedNormalFactScore(){return inherited_normal_fact_lm_score;};
        long int getInheritedNormalActionScore(){return inherited_normal_action_lm_score;};
        long int getInheritedAbstractFactScore(){return inherited_abstract_fact_lm_score;};
        long int getInheritedAbstractActionScore(){return inherited_abstract_action_lm_score;};
        long int getInheritedNormalandAbstractScore(){return getInheritedNormalScore() + getInheritedAbstractScore();};
        long int getInheritedNormalScore(){return inherited_normal_fact_lm_score + inherited_normal_action_lm_score;};
        long int getInheritedAbstractScore(){return inherited_abstract_fact_lm_score + inherited_abstract_action_lm_score;};

        //setters
        void setIsContained(bool contained_){contained = contained_;}
        void setNormalFactScore(int normal_fact_lm_score_){normal_fact_lm_score = normal_fact_lm_score_;}
        void setNormalActionScore(int normal_action_lm_score_){normal_action_lm_score = normal_action_lm_score_;}
        void setAbstractFactScore(int abstract_fact_lm_score_){abstract_fact_lm_score = abstract_fact_lm_score_;}
        void setAbstractActionScore(int abstract_action_lm_score_){abstract_action_lm_score = abstract_action_lm_score_;}
        void inheritNormalFactScore(GoalSet* goal_set_){inherited_normal_fact_lm_score = inherited_normal_fact_lm_score + goal_set_->getNormalFactScore();}
        void inheritNormalActionScore(GoalSet* goal_set_){inherited_normal_action_lm_score = inherited_normal_action_lm_score + goal_set_->getNormalActionScore();}
        void inheritAbstractFactScore(GoalSet* goal_set_){inherited_abstract_fact_lm_score = inherited_abstract_fact_lm_score + goal_set_->getAbstractFactScore();}
        void inheritAbstractActionScore(GoalSet* goal_set_){inherited_abstract_action_lm_score = inherited_abstract_action_lm_score + goal_set_->getAbstractActionScore();}
        void inheritAllBaseScores(GoalSet* goal_set_){inheritNormalFactScore(goal_set_);inheritNormalActionScore(goal_set_);inheritAbstractFactScore(goal_set_);inheritAbstractActionScore(goal_set_);}
        void inheritInheritedNormalFactScore(GoalSet* goal_set_){inherited_normal_fact_lm_score = inherited_normal_fact_lm_score + goal_set_->getInheritedNormalFactScore();}
        void inheritInheritedNormalActionScore(GoalSet* goal_set_){inherited_normal_action_lm_score = inherited_normal_action_lm_score + goal_set_->getInheritedNormalActionScore();}
        void inheritInheritedAbstractFactScore(GoalSet* goal_set_){inherited_abstract_fact_lm_score = inherited_abstract_fact_lm_score + goal_set_->getInheritedAbstractFactScore();}
        void inheritInheritedAbstractActionScore(GoalSet* goal_set_){inherited_abstract_action_lm_score = inherited_abstract_action_lm_score + goal_set_->getInheritedAbstractActionScore();}
        void inheritAllInheritedScores(GoalSet* goal_set_){inheritInheritedNormalFactScore(goal_set_);inheritInheritedNormalActionScore(goal_set_);inheritInheritedAbstractFactScore(goal_set_);inheritInheritedAbstractActionScore(goal_set_);}
        void clearInheritedScore(){inherited_normal_fact_lm_score=0;inherited_normal_action_lm_score=0;inherited_abstract_fact_lm_score=0;inherited_abstract_action_lm_score=0;}
        void push_back(TemporalLandmark* landmark){landmarks.push_back(landmark);};
    };
}



#endif //TEMPORAL_LANDMARKS_GoalSetS_H
