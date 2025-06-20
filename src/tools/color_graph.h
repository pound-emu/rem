#pragma once

#include <unordered_set>
#include <vector>
#include <inttypes.h>

struct color_node;
struct color_graph;

struct color_node {
    color_graph* context;
    uint64_t data;
    int color;
    std::unordered_set<color_node*> links;

    static void create_link(color_node* x, color_node* y);
};

struct color_graph {
    std::vector<color_node*> all_nodes;

    static void create(color_graph* result);
    static void destroy(color_graph* to_destroy);
    static color_node* create_node(color_graph* context);
    static void color(color_graph* context);
};
