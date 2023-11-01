//
// code from https://www.geeksforgeeks.org/depth-first-search-or-dfs-for-a-graph/
//

#ifndef STP_GRAPH_H
#define STP_GRAPH_H


using namespace std;
namespace strategic_tactical {
    // Graph class represents a directed graph
// using adjacency list representation
    class Graph {
        int V; // No. of vertices
        map <int,string> typeMap;

        // Pointer to an array containing
        // adjacency lists
        list<int>* adj;

        // This function is a variation of DFSUtil() in https://www.geeksforgeeks.org/archives/18212
        bool isCyclicUtil(int v, bool visited[], bool *recStack, pair <int,int> prev_dependency)
        {

            if(visited[v] == false){
                // Mark the current node as visited and part of recursion stack
                visited[v] = true;
                recStack[v] = true;

                //cout << "   marked " << v << " as visited";
                //cout << " -> " << v ;

                if(prev_dependency.first < 0)prev_dependency.first = v;
                else{
                    prev_dependency.second = v;
                    cycleVertices.push_back(prev_dependency);
                    cout << "graph dependency added " << typeMap[prev_dependency.first] << " -> " << typeMap[prev_dependency.second] << endl;
                    prev_dependency.first = v;
                }

                // Recur for all the vertices adjacent to this vertex
                list<int>::iterator i;
                for(i = adj[v].begin(); i != adj[v].end(); ++i){
                    if ( !visited[*i] && isCyclicUtil(*i, visited, recStack, prev_dependency) ){
                        return true;
                    }

                    else if (recStack[*i]){
                        //cout << " -> " << *i << " cycle";
                        cycle_child = *i;
                        prev_dependency.second = *i;
                        cycleVertices.push_back(prev_dependency);
                        cout << "graph dependency added " << typeMap[prev_dependency.first] << " -> " << typeMap[prev_dependency.second] << endl;
                        return true;
                    }

                }

            }
            recStack[v] = false;  // remove the vertex from recursion stack
            return false;
        };

    public:
        int cycle_parent;
        int cycle_child;
        vector<pair<int,int>> children;
        vector<pair<int,int>> cycleVertices;
        Graph(int V_, map <int,string> typeMap_) : V(V_), typeMap(typeMap_){
            adj = new list<int>[V];
        };

        // function to add an edge to graph
        void addEdge(int v, int w){
            adj[v].push_back(w);
        };


        // Function to add the child for
        // the given node
        void add_child(int vNumber, int length)
        {
            pair<int, int> p;
            p.first = vNumber;
            p.second = length;
            children.push_back(p);
        }

        // Returns true if the graph contains a cycle, else false.
        // This function is a variation of DFS() in https://www.geeksforgeeks.org/archives/18212
        bool isCyclic(){
            cout << endl << "Searching for cycles in the Graph..." << endl;
            // Mark all the vertices as not visited and not part of recursion
            // stack
            bool *visited = new bool[V];
            bool *recStack = new bool[V];
            for(int i = 0; i < V; i++){
                visited[i] = false;
                recStack[i] = false;
            }

            // Call the recursive helper function to detect cycle in different
            // DFS trees
            for(int i = 0; i < V; i++){
                pair<int,int> dependency;
                dependency.first = -1;
                dependency.second = -1;
                if (isCyclicUtil(i, visited, recStack, dependency))
                    return true;
            }

            return false;
        }
    };

}






#endif //STP_GRAPH_H
