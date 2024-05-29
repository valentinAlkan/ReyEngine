#include "SearchNode.h"
#include "Application.h"

SearchNode::SearchNode(float heuristic) {
   _id = Application::generateUniqueValue();
   this->heuristic = heuristic;
}

void SearchNode::addConnection(float cost, SearchNode connection) {
   auto check = _connectedNodes.find(connection.getId());
   //check if node is already connected;
   if(check != _connectedNodes.end()){
      return;
   }
   //add to connections
   connections.insert(pair<float, SearchNode>(cost, connection));
   //add to _connectedNodes
   _connectedNodes.insert(pair<int, SearchNode>(connection.getId(), connection));
}
