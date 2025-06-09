
/*
Written by Antoine Savine in 2018

This code is the strict IP of Antoine Savine

License to use and alter this code for personal and commercial applications
is freely granted to any person or company who purchased a copy of the book

Modern Computational Finance: AAD and Parallel Simulations
Antoine Savine
Wiley, 2018

As long as this comment is preserved at the top of the file
*/

#ifndef FEE08509_4CE4_4ED9_9905_9014AE04FC88
#define FEE08509_4CE4_4ED9_9905_9014AE04FC88

//  AAD implementation of chapter 10
//  (With multi-dimensional additions of chapter 14)

//  Implementation of the Tape

//  Unchanged for AADET of chapter 15

#include "blocklist.h"
#include "AADNode.h"

constexpr size_t BLOCKSIZE = 16384; //	Number of nodes
constexpr size_t ADJSIZE = 32768;   //	Number of adjoints
constexpr size_t DATASIZE = 65536;  //	Data in bytes

class Tape
{
    //	Working with multiple results / adjoints?
    static bool multi;

    //  Storage for adjoints in multi-dimensional case (chapter 14)
    blocklist<double, ADJSIZE> myAdjointsMulti;

    //  Storage for derivatives and child adjoint pointers
    blocklist<double, DATASIZE> myDers;
    blocklist<double *, DATASIZE> myArgPtrs;

    //  Storage for the nodes
    blocklist<ADNode, BLOCKSIZE> myADNodes;

    //	Padding so tapes in a vector don't interfere
    char myPad[64];

    friend auto setNumResultsForAAD(const bool, const size_t);
    friend struct numResultsResetterForAAD;
    friend class Number;

public:
    //  Build note in place and return a pointer
    //	N : number of childs (arguments)
    template <size_t N>
    ADNode *recordADNode()
    {
        //  Construct the node in place on tape
        ADNode *node = myADNodes.emplace_back(N);

        //  Store and zero the adjoint(s)
        if (multi)
        {
            node->pAdjoints = myAdjointsMulti.emplace_back_multi(ADNode::numAdj);
            fill(node->pAdjoints, node->pAdjoints + ADNode::numAdj, 0.0);
        }

        //	Store the derivatives and child adjoint pointers unless leaf
        if constexpr (N > 0)
        {
            node->pDerivatives = myDers.emplace_back_multi<N>();
            node->pAdjPtrs = myArgPtrs.emplace_back_multi<N>();
        }

        return node;
    }

    //  Reset all adjoints to 0
    void resetAdjoints()
    {
        if (multi)
        {
            myAdjointsMulti.memset(0);
        }
        else
        {
            for (ADNode &node : myADNodes)
            {
                node.mAdjoint = 0;
            }
        }
    }

    //  Clear
    void clear()
    {
        myAdjointsMulti.clear();
        myDers.clear();
        myArgPtrs.clear();
        myADNodes.clear();
    }

    //  Rewind
    void rewind()
    {

#ifdef _DEBUG

        //  In debug mode, always wipe
        //      makes it easier to identify errors

        clear();

#else
        //  In release mode, rewind and reuse

        if (multi)
        {
            myAdjointsMulti.rewind();
        }
        myDers.rewind();
        myArgPtrs.rewind();
        myADNodes.rewind();

#endif
    }

    //  Set mark
    void mark()
    {
        if (multi)
        {
            myAdjointsMulti.setmark();
        }
        myDers.setmark();
        myArgPtrs.setmark();
        myADNodes.setmark();
    }

    //  Rewind to mark
    void rewindToMark()
    {
        if (multi)
        {
            myAdjointsMulti.rewind_to_mark();
        }
        myDers.rewind_to_mark();
        myArgPtrs.rewind_to_mark();
        myADNodes.rewind_to_mark();
    }

    //  Iterators

    using iterator = blocklist<ADNode, BLOCKSIZE>::iterator;

    auto begin()
    {
        return myADNodes.begin();
    }

    auto end()
    {
        return myADNodes.end();
    }

    auto markIt()
    {
        return myADNodes.mark();
    }

    auto find(ADNode *node)
    {
        return myADNodes.find(node);
    }
};

#endif /* FEE08509_4CE4_4ED9_9905_9014AE04FC88 */
