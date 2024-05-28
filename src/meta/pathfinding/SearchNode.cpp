#include "SearchNode.h"

SearchNode::SearchNode() {
   _id = 1;
}

void SearchNode::addConnection(float cost, SearchNode connection) {
   auto check = _connectedNodes.find(connection.getId());
   //check if node is already connected;
   if(check != _connectedNodes.end()){
      return;
   }
   //add to connections
   connections[cost] = connection;
   //add to _connectedNodes
   _connectedNodes[connection.getId()] = connection;
}
