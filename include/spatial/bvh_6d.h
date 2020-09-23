#pragma once

#include "bvh_types.h"
#include "bvh_data.h"
#include "data_tree.h"

#define BVH_N_CHILDREN 2

namespace rsurfaces
{
    class BVH6D
    {
    };

    class BVHNode6D
    {
    public:
        // Build a BVH of the given points
        BVHNode6D(std::vector<MassNormalPoint> &points, int axis);
        ~BVHNode6D();

        // Basic spatial data
        double totalMass;
        Vector3 centerOfMass;
        Vector3 averageNormal;
        Vector3 minCoords;
        Vector3 maxCoords;
        // Indexing and other metadata
        size_t elementID;
        size_t nodeID;
        BVHNodeType nodeType;
        size_t numNodesInBranch;
        size_t nElements;
        // Children
        BVHNode6D *children[BVH_N_CHILDREN];

        // Store the list of all indices of elements in this cluster
        std::vector<size_t> clusterIndices;

        inline GCFace getSingleFace(MeshPtr &mesh)
        {
            return mesh->face(elementID);
        }

        // Assign unique IDs to all nodes in this tree
        size_t assignIDsRecursively(size_t startID);
        // Recursively recompute all centers of mass in this tree
        void recomputeCentersOfMass(MeshPtr &mesh, GeomPtr &geom);
        bool isAdmissibleFrom(Vector3 vertPos, double thresholdTheta);
        void printSummary();
        MassNormalPoint GetMassNormalPoint();

        inline BVHData GetNodeDataAsStruct()
        {
            BVHData data{totalMass, centerOfMass, averageNormal, minCoords, maxCoords, elementID, nodeID, nodeType, numNodesInBranch, nElements, {0, 0}};
            if (nodeType == BVHNodeType::Interior)
            {
                data.child[0] = children[0]->nodeID;
                data.child[1] = children[1]->nodeID;
            }
            return data;
        }

        inline double nodeRatio(double d)
        {
            return nodeRatioBox(minCoords, maxCoords, d);
        }

        template <typename Data>
        void indexNodesForDataTree(DataTreeContainer<Data> *cont, DataTree<Data> *droot)
        {
            // Put the root in the correct spot
            cont->byIndex[droot->nodeID] = droot;
            // Recursively index children
            for (DataTree<Data> *child : droot->children)
            {
                indexNodesForDataTree(cont, child);
            }
        }

        // Creates an auxilliary DataTree structure for this BVH.
        template <typename Data, typename Init = DefaultInit<Data>>
        DataTreeContainer<Data> *CreateDataTree()
        {
            DataTree<Data> *droot = CreateDataTreeRecursive<Data, Init>();
            DataTreeContainer<Data> *cont = new DataTreeContainer<Data>(droot, numNodesInBranch);
            indexNodesForDataTree(cont, droot);
            return cont;
        }

    private:
        double AxisSplittingPlane(std::vector<MassNormalPoint> &points, int axis);
        void averageDataFromChildren();
        void mergeIndicesFromChildren();

        template <typename Data, typename Init = DefaultInit<Data>>
        DataTree<Data> *CreateDataTreeRecursive()
        {
            DataTree<Data> *droot = new DataTree<Data>(this);
            droot->nodeID = nodeID;
            Init::Init(droot->data, this);
            if (nodeType == BVHNodeType::Interior)
            {
                for (BVHNode6D *child : children)
                {
                    DataTree<Data> *childData = child->CreateDataTreeRecursive<Data, Init>();
                    droot->children.push_back(childData);
                }
            }

            return droot;
        }
    };

    BVHNode6D *Create6DBVHFromMeshFaces(MeshPtr &mesh, GeomPtr &geom);

    template <typename Data>
    DataTree<Data> *DataTreeContainer<Data>::GetDataNode(BVHNode6D *bvhNode)
    {
        return byIndex[bvhNode->nodeID];
    }

} // namespace rsurfaces
