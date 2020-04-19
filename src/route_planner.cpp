#include "route_planner.h"
#include <algorithm>

RoutePlanner::RoutePlanner(RouteModel &model, float start_x, float start_y, float end_x, float end_y): m_Model(model) {
    // Convert inputs to percentage:
    start_x *= 0.01;
    start_y *= 0.01;
    end_x *= 0.01;
    end_y *= 0.01;

    // Finds the closest nodes to the starting and ending coordinates & store it in the RoutePlanner's start_node and end_node attributes.
    start_node = &m_Model.FindClosestNode(start_x, start_y);
    end_node = &m_Model.FindClosestNode(end_x, end_y); 
}


  // The CalculateHValue method use the distance to the end_node for the h value.
  float RoutePlanner::CalculateHValue(RouteModel::Node const *node) {
	return node->distance(*end_node);
}


  // The AddNeighbors method expand the current node by adding all unvisited neighbors to the open list.
  void RoutePlanner::AddNeighbors(RouteModel::Node *current_node) {
    //Find the valid neighbor of current_node
    current_node->FindNeighbors(); 
    //set to the neighbor the: parent, h_value, g_value, mark as visited, add to open_list.
      for(RouteModel::Node* nnode: current_node->neighbors){
        nnode->parent = current_node;
        nnode->g_value = current_node->g_value + current_node->distance(*nnode);
        nnode->h_value = CalculateHValue(nnode);		//implement the h-Value calculation.
        nnode->visited = true;		
        open_list.emplace_back(nnode);
    }
  }


  //Compare two nodes f=g+h values during sorting
  bool Compare(const RouteModel::Node* a, const RouteModel::Node* b) {
    float fa = a->h_value + a->g_value; 
    float fb = b->h_value + b->g_value; 
    return fa > fb; 
	}

  // The NextNode method sorts the open list and return the next node.
  RouteModel::Node *RoutePlanner::NextNode() {
    sort(open_list.begin(),open_list.end(), Compare);
    RouteModel::Node* p=open_list.back();
    open_list.pop_back();
    return p;
}


// Return the final path found for the center method A* search.
// follows the chain of parents of nodes of the final node until the starting node is found. 
//For each node in the chain, add the distance from the node to its parent to the distance variable.
std::vector<RouteModel::Node> RoutePlanner::ConstructFinalPath(RouteModel::Node *current_node) {
  // Create path_found vector
  std::vector<RouteModel::Node> path_found;
  float dist = 0.0f;
  // - For each node in the chain, add the distance from the node to its parent to the distance variable.
  while(current_node->parent != nullptr){
    float f = current_node->g_value;
    path_found.push_back(*current_node);
    dist += current_node->distance(*(current_node->parent));
    current_node = current_node->parent;
    current_node->g_value += f;
  }
  distance = dist;
  //insert start node to path found vector end reverse the order of the list
  path_found.push_back(*current_node);
  std::reverse(path_found.begin(), path_found.end());
  // Multiply the distance by the scale of the map to get meters.
  distance *= m_Model.MetricScale(); 
  return path_found;
}


// The A* Search mathod using all previous methods for:
// - AddNeighbors: Add all of the neighbors of the current node to the open_list.
// - NextNode: Sort the open_list and return the next node.
// - ConstructFinalPath: When the search has reached the end_node, return the final path that was found.
// - Store the final path in the m_Model.path attribute. This path will then be displayed on the map tile.
void RoutePlanner::AStarSearch() {
  RouteModel::Node *current_node = nullptr;
  //mark start node status as visited and add him first to the open list
  start_node->visited = true;
  current_node = start_node;
  open_list.push_back(start_node);
  
  while(open_list.size() > 0){		
    // If current node is the goal node, then construct the path 
	if(current_node->distance(*end_node) == 0){
    m_Model.path = ConstructFinalPath(current_node);
      return;
    }
    //else find node's neighbors
    AddNeighbors(current_node);
    current_node = NextNode();
  }
  return;
}