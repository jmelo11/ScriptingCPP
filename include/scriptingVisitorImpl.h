#pragma once

#include "scriptingVisitor.h"

//  Implementation of Visitor and constVisitor functions that require the Node definition

template <class V>
inline void Visitor<V>::visitNode(Node &node)
{
    node.accept(static_cast<V &>(*this));
}

template <class V>
inline void Visitor<V>::visitArguments(Node &node)
{
    for (auto &arg : node.arguments)
    {
        arg->accept(static_cast<V &>(*this));
    }
}

template <class V>
inline void Visitor<V>::visit(Node &node)
{
    visitArguments(node);
}

template <class V>
inline void constVisitor<V>::visitNode(const Node &node)
{
    node.accept(static_cast<V &>(*this));
}

template <class V>
inline void constVisitor<V>::visitArguments(const Node &node)
{
    for (const auto &arg : node.arguments)
    {
        arg->accept(static_cast<V &>(*this));
    }
}

template <class V>
template <class NODE>
inline void constVisitor<V>::visit(const NODE &node)
{
    static_assert(!hasNonConstVisit<V>::template forNodeType<NODE>(),
                  "CONST VISITOR DECLARES A NON-CONST VISIT");
    visitArguments(node);
}
