#pragma once
#include <map>

using namespace std;
//class to hold node info to use in the pathfinding algorithm
class SearchNode {
public:
   bool expanded = false;
   //first = cost of connection second = connected node
   map<float, SearchNode> connections;

   SearchNode();

   int getId(){
      return _id;
   }

   /**
    * adds a connection to the map with the given cost
    * @param cost : The cost of the connection
    * @param connection : The node to connect
    */
   void addConnection(float cost, SearchNode connection);

private:
   int _id;
   //a map to determine if the node is already in the connections
   map<int, SearchNode> _connectedNodes;
};

