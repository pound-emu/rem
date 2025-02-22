#ifndef MISC_TOOLS_H
#define MISC_TOOLS_H

#include <unordered_map>

template<typename A, typename B>
static bool in_map(std::unordered_map<A, B>* context, A key)
{
    return context->find(key) != context->end();
}

template<typename A>
static bool in_set(std::unordered_set<A>* context, A key)
{
    return context->find(key) != context->end();
}

#endif