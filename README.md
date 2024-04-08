# RALSTP
Recursive Agents and Landmarks Strategic-Tactical Planning (RALSTP) - Proof of Concept


Below is a bare-bones install description that will be updated soon:

1. paths to optic-cplex, VAL_LATEST and TPSHE must be updated in ProblemGenerator.cpp according to local path

2. paths to optic-clp must be updated in opticMain.cpp according to local path

3. potentially rebuild VAL_LATEST and TPSHE individually

4. run ./compile to build RALSTP (needs same libraries as optic-clp to be preinstalled)

5. run ./ralstp <pddl_domain_file_name> <pddl_problem_file_name>


