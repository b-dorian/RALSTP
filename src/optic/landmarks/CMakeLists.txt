cmake_minimum_required(VERSION 2.4)

project(temporal_landmarks)

set( CMAKE_MODULE_PATH    ${CMAKE_MODULE_PATH}
                          ${CMAKE_CURRENT_SOURCE_DIR}
                          ${CMAKE_CURRENT_SOURCE_DIR}/..
                          ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules)

set(DOXYFILE_TARGET_SUFFIX temporal_landmarks)
include(UseDoxygen OPTIONAL)


include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
					${val_SOURCE_DIR} ${val_SOURCE_DIR}  ${val_SOURCE_DIR}/parsing ${val_BINARY_DIR}
					${optic_SOURCE_DIR} ${optic_BINARY_DIR})

set(libTemporalLandmarks_SRCS
      TimePointVariable.cpp TemporalLandmark.cpp SimpleTemporalConstraint.cpp 
      TEvent.cpp TemporalActionLandmark.cpp TemporalCompleteActionLandmark.cpp
		SimpleAgent.cpp ProblemGenerator.cpp GoalSet.cpp Formula.cpp TemporalStateLandmark.cpp
      TemporalLandmarksAndConstraints.cpp
      DerivationRule.cpp DerivationRuleAchieverOfFormula.cpp 
      DerivationRuleTEventCondition.cpp DerivationRuleActionStartEnd.cpp
      DerivationRuleTEventEffect.cpp DerivationRuleFirstAchieverOfFormula.cpp
)

add_library(TemporalLandmarks STATIC ${libTemporalLandmarks_SRCS})
target_link_libraries(TemporalLandmarks Inst ParsePDDL)

