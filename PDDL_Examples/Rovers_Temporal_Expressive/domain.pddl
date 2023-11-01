(define (domain Rover)
(:requirements :typing :durative-actions)
(:types rover waypoint store lander camera mode objective - object

    ;new types
    sample - object
    rock_sample soil_sample - sample) 

(:predicates (at ?x - rover ?y - waypoint) 
             (at_lander ?x - lander ?y - waypoint)
             (can_traverse ?r - rover ?x - waypoint ?y - waypoint)
         (equipped_for_soil_analysis ?r - rover)
             (equipped_for_rock_analysis ?r - rover)
             (equipped_for_imaging ?r - rover)
             (empty ?s - store)
             (have_rock_analysis ?r - rover ?s - sample)  ; (have_rock_analysis ?r - rover ?w - waypoint) - modification to have individual sample analysis instead of waypoint
             (have_soil_analysis ?r - rover ?s - sample)  ; (have_soil_analysis ?r - rover ?w - waypoint) - modification to have individual sample analysis instead of waypoint
             (full ?s - store)
         (calibrated ?c - camera ?r - rover) 
         (supports ?c - camera ?m - mode)
             (available ?r - rover)
             (visible ?w - waypoint ?p - waypoint)
             (have_image ?r - rover ?o - objective ?m - mode)
             (communicated_soil_data ?s - soil_sample ?w - waypoint) ; (communicated_soil_data ?w - waypoint)
             (communicated_rock_data ?s - rock_sample ?w - waypoint) ; (communicated_rock_data ?w - waypoint)
             (communicated_image_data ?o - objective ?m - mode)
         (at_sample ?s - sample ?w - waypoint) ; (at_soil_sample ?w - waypoint) and (at_rock_sample ?w - waypoint)
             (visible_from ?o - objective ?w - waypoint)
         (store_of ?s - store ?r - rover)
         (calibration_target ?i - camera ?o - objective)
         (on_board ?i - camera ?r - rover)
         (channel_free ?l - lander)

         ;new predicates
         (l_e_d_off ?r - rover)
         (unilluminated ?s - sample)
         (illuminated ?s - sample)

)

;new action
(:durative-action illuminate_sample
 :parameters (?r - rover ?s - sample ?p - waypoint)
 :duration (= ?duration 11)
 :condition (and 
    (at start (l_e_d_off ?r))
    (at start (at_sample ?s ?p))
    (at start (unilluminated ?s))
    (over all (at ?r ?p))
    )
 :effect (and 
    (at start (not (l_e_d_off ?r)))
    (at start (not (unilluminated ?s)))
    (at start (illuminated ?s))
    (at end (unilluminated ?s))
    (at end (not (illuminated ?s)))
    (at end (l_e_d_off ?r))
    )
 )

;old actions
(:durative-action sample_soil
:parameters (?x - rover ?st - store ?s - soil_sample ?p - waypoint)
:duration (= ?duration 10)
:condition (and 
    (over all (illuminated ?s)) ; new condition

    (over all (at ?x ?p))
    (at start (at ?x ?p)) 
    (at start (at_sample ?s ?p)) 
    (at start (equipped_for_soil_analysis ?x)) 
    (at start (store_of ?st ?x)) 
    (at start (empty ?st))
        )
:effect (and (at start (not (empty ?st)))
 (at end (full ?st)) 
  (at end (have_soil_analysis ?x ?s)) 
  (at end (not (at_sample ?s ?p)))
        )
)

(:durative-action sample_rock
:parameters (?x - rover ?st - store ?s - rock_sample ?p - waypoint)
:duration (= ?duration 8)
:condition (and 
    (over all (illuminated ?s))  ; new condition

    (over all (at ?x ?p)) 
    (at start (at ?x ?p))  (at start (at_sample ?s ?p)) 
    (at start (equipped_for_rock_analysis ?x)) 
    (at start (store_of ?st ?x)) 
    (at start (empty ?st))
        )
:effect (and (at start (not (empty ?st))) (at end (full ?st)) (at end (have_rock_analysis ?x ?s)) 
    (at end (not (at_sample ?s ?p)))
        )
)

(:durative-action navigate
:parameters (?x - rover ?y - waypoint ?z - waypoint) 
:duration (= ?duration 5)
:condition (and (over all (can_traverse ?x ?y ?z)) (at start (available ?x)) (at start (at ?x ?y))  
                (over all (visible ?y ?z))
        )
:effect (and 
    (at start (not (at ?x ?y))) 
    (at end (at ?x ?z))))

(:durative-action drop
:parameters (?x - rover ?y - store)
:duration (= ?duration 1)
:condition (and (at start (store_of ?y ?x)) (at start (full ?y))
        )
:effect (and (at end (not (full ?y))) (at end (empty ?y))
    )
)

(:durative-action calibrate
 :parameters (?r - rover ?i - camera ?t - objective ?w - waypoint)
 :duration (= ?duration 17)
 :condition (and (at start (equipped_for_imaging ?r))  (at start (calibration_target ?i ?t)) (over all (at ?r ?w)) (at start (visible_from ?t ?w)) (at start (on_board ?i ?r))
        )
 :effect (and (at end (calibrated ?i ?r)))
)




(:durative-action take_image
 :parameters (?r - rover ?p - waypoint ?o - objective ?i - camera ?m - mode)
 :duration (= ?duration 7)
 :condition (and 
    (over all (calibrated ?i ?r))
    (at start (on_board ?i ?r))
    (over all (equipped_for_imaging ?r))
    (over all (supports ?i ?m) )
    (over all (visible_from ?o ?p))
    (over all (at ?r ?p))
               )
 :effect (and 
    (at end (have_image ?r ?o ?m)) 
    (at end (not (calibrated ?i ?r)))
        )
)


(:durative-action communicate_soil_data
 :parameters (?r - rover ?l - lander ?s - soil_sample ?p - waypoint ?x - waypoint ?y - waypoint )
 :duration (= ?duration 10)
 :condition (and 
    (over all (at ?r ?x)) 
    (over all (at_lander ?l ?y)) 
    (at start (have_soil_analysis ?r ?s)) 
    (at start (visible ?x ?y)) 
    (at start (available ?r))
    (at start (channel_free ?l))

            )
 :effect (and 
    (at start (not (available ?r))) 
    (at start (not (channel_free ?l))) 
    (at end (channel_free ?l))
    (at end (communicated_soil_data ?s ?p))
    (at end (available ?r))
    )
)

(:durative-action communicate_rock_data
 :parameters (?r - rover ?l - lander ?s - rock_sample ?p - waypoint ?x - waypoint ?y - waypoint)
 :duration (= ?duration 10)
 :condition (and (over all (at ?r ?x)) (over all (at_lander ?l ?y)) (at start (have_rock_analysis ?r ?s)) 
                   (at start (visible ?x ?y)) (at start (available ?r))(at start (channel_free ?l)) 
            )
 :effect (and (at start (not (available ?r))) (at start (not (channel_free ?l))) (at end (channel_free ?l))(at end (communicated_rock_data ?s ?p))(at end (available ?r)) 
          )
)


(:durative-action communicate_image_data
 :parameters (?r - rover ?l - lander ?o - objective ?m - mode ?x - waypoint ?y - waypoint)
 :duration (= ?duration 15)
 :condition (and (over all (at ?r ?x)) (over all (at_lander ?l ?y)) (at start (have_image ?r ?o ?m)) 
                   (at start (visible ?x ?y)) (at start (available ?r)) (at start (channel_free ?l))
            )
 :effect (and (at start (not (available ?r))) (at start (not (channel_free ?l))) (at end (channel_free ?l)) (at end (communicated_image_data ?o ?m)) (at end (available ?r))
          )
)

)
