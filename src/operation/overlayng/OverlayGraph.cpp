/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/overlayng/OverlayGraph.h>
#include <geos/operation/overlayng/OverlayEdge.h>
#include <geos/operation/overlayng/Edge.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>

namespace geos {      // geos
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

using namespace geos::geom;

/**
* Creates a new graph for a set of noded, labelled {@link Edge}s.
*/
//std::vector<std::unique_ptr<Edge>> && edges
OverlayGraph::OverlayGraph()
{}

/*public*/
std::vector<OverlayEdge*>&
OverlayGraph::getEdges()
{
    return edges;
}

/*public*/
std::unique_ptr<std::vector<OverlayEdge*>>
OverlayGraph::getNodeEdges()
{
    std::unique_ptr<std::vector<OverlayEdge*>> nodeEdges(new std::vector<OverlayEdge*>);
    for (auto nodeMapPair : nodeMap) {
        nodeEdges->push_back(nodeMapPair.second);
    }
    return nodeEdges;
}

/*public*/
OverlayEdge*
OverlayGraph::getNodeEdge(const Coordinate& nodePt) const
{
    auto it = nodeMap.find(nodePt);
    if (it == nodeMap.end()) {
        return nullptr;
    }
    return it->second;
}

/*public*/
std::unique_ptr<std::vector<OverlayEdge*>>
OverlayGraph::getResultAreaEdges()
{
    std::unique_ptr<std::vector<OverlayEdge*>> resultEdges(new std::vector<OverlayEdge*>());
    for (OverlayEdge* edge : getEdges()) {
        if (edge->isInResultArea()) {
            resultEdges->push_back(edge);
        }
    }
    return resultEdges;
}

/*public*/
OverlayEdge*
OverlayGraph::addEdge(Edge* edge)
{
    // CoordinateSequence* pts = = edge->getCoordinates().release();
    CoordinateSequence* pts = edge->releaseCoordinates();
    OverlayEdge* e = createEdgePair(pts, createOverlayLabel(edge));
    insert(e);
    insert(e->symOE());
    return e;
}

/*private*/
OverlayEdge*
OverlayGraph::createEdgePair(const CoordinateSequence *pts, const OverlayLabel *lbl)
{
    csQue.emplace_back(pts);
    OverlayEdge* e0 = createOverlayEdge(pts, lbl, true);
    OverlayEdge* e1 = createOverlayEdge(pts, lbl, false);
    e0->link(e1);
    return e0;
}

/*private*/
OverlayEdge*
OverlayGraph::createOverlayEdge(const CoordinateSequence* pts, const OverlayLabel* lbl, bool direction)
{
    Coordinate origin;
    Coordinate dirPt;
    if (direction) {
        origin = pts->getAt(0);
        dirPt = pts->getAt(1);
    }
    else {
        std::size_t ilast = pts->size() - 1;
        origin = pts->getAt(ilast);
        dirPt = pts->getAt(ilast-1);
    }
    ovEdgeQue.emplace_back(origin, dirPt, direction, lbl, pts);
    OverlayEdge& ove = ovEdgeQue.back();
    return &ove;
}

/*public*/
const OverlayLabel*
OverlayGraph::createOverlayLabel(const Edge* edge)
{
    // Instantate OverlayLabel on the std::deque
    ovLabelQue.emplace_back();
    // Read back a reference
    OverlayLabel& ovl = ovLabelQue.back();
    // Initialize the reference with values from edge
    edge->populateLabel(ovl);
    // Return as pointer.
    return &ovl;
}

/*private*/
void
OverlayGraph::insert(OverlayEdge* e)
{
    edges.push_back(e);

    /**
     * If the edge origin node is already in the graph,
     * insert the edge into the star of edges around the node.
     * Otherwise, add a new node for the origin.
     */
    auto it = nodeMap.find(e->orig());
    if (it != nodeMap.end()) {
        // found in map
        OverlayEdge* nodeEdge = it->second;
        nodeEdge->insert(e);
    }
    else {
        nodeMap[e->orig()] = e;
    }
}





} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos
