(define (domain driverlog)
  (:requirements :typing :durative-actions) 
  (:types           location locatable - object
		driver truck obj - locatable)

  (:predicates 
		(at ?obj - locatable ?loc - location)
		(in ?obj1 - obj ?obj - truck)
		(driving ?d - driver ?v - truck)
		(link ?x ?y - location) (path ?x ?y - location)
		(empty ?v - truck)
)


static predicate: (exists location1)
static predicate: (exists location2)

dynamic predicate (has woman key)  - selected
dynamic predicate (has man hammer) - discarded

dynamic predicate (opens key access1)    - selected
dynamic predicate (opens hammer access2) - discarded

dynamic predicate:  (available access1 location1) - selected
dynamic predicate:  (available access2 location2) - discarded

1. not (available access1 location1)

2. fail

dynamic object: access

static object: location


for problem for which we compute relaxed plan
we remove static object facts and goals
  if fails: necessary
  if success:
    if found in relaxed: necessary
    if not found in relaxed: 



(:durative-action LOAD-TRUCK
  :parameters
   (?obj - obj
    ?truck - truck
    ?loc - location)
  :duration (= ?duration 2)
  :condition
   (and 
   (over all (at ?truck ?loc)) (at start (at ?obj ?loc)))
  :effect
   (and (at start (not (at ?obj ?loc))) (at end (in ?obj ?truck))))

(:durative-action UNLOAD-TRUCK
  :parameters
   (?obj - obj
    ?truck - truck
    ?loc - location)
  :duration (= ?duration 2)
  :condition
   (and
        (over all (at ?truck ?loc)) (at start (in ?obj ?truck)))
  :effect
   (and (at start (not (in ?obj ?truck))) (at end (at ?obj ?loc))))

(:durative-action BOARD-TRUCK
  :parameters
   (?driver - driver
    ?truck - truck
    ?loc - location)
  :duration (= ?duration 1)
  :condition
   (and 
   (over all (at ?truck ?loc)) (at start (at ?driver ?loc)) 
	(at start (empty ?truck)))
  :effect
   (and (at start (not (at ?driver ?loc))) 
	(at end (driving ?driver ?truck)) (at start (not (empty ?truck)))))

(:durative-action DISEMBARK-TRUCK
  :parameters
   (?driver - driver
    ?truck - truck
    ?loc - location)
  :duration (= ?duration 1)
  :condition
   (and (over all (at ?truck ?loc)) (at start (driving ?driver ?truck)))
  :effect
   (and (at start (not (driving ?driver ?truck))) 
	(at end (at ?driver ?loc)) (at end (empty ?truck))))

(:durative-action DRIVE-TRUCK
  :parameters
   (?truck - truck
    ?loc-from - location
    ?loc-to - location
    ?driver - driver)
  :duration (= ?duration 10)
  :condition
   (and (at start (at ?truck ?loc-from))
   (over all (driving ?driver ?truck)) (at start (link ?loc-from ?loc-to)))
  :effect
   (and (at start (not (at ?truck ?loc-from))) 
	(at end (at ?truck ?loc-to))))

(:durative-action WALK
  :parameters
   (?driver - driver
    ?loc-from - location
    ?loc-to - location)
  :duration (= ?duration 20)
  :condition
   (and (at start (at ?driver ?loc-from)) 
	(at start (path ?loc-from ?loc-to)))
  :effect
   (and (at start (not (at ?driver ?loc-from)))
	(at end (at ?driver ?loc-to))))
 
)
