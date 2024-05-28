#include "SearchNode.h"
#include "Application.h"

SearchNode::SearchNode() {
   _id = Application::generateUniqueValue();
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
