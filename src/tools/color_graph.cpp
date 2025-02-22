#include "color_graph.h"
#include "debugging.h"
#include "tools/misc_tools.h"

void color_graph::create(color_graph* result)
{
    result->all_nodes = std::vector<color_node*>();
}

static void insert_link(color_node* left, color_node* to_add)
{
    if (left == to_add)
        return;

    left->links.insert(to_add);
}

void color_node::create_link(color_node* x, color_node* y)
{
    if (x == y)
        return;

    insert_link(x, y);
    insert_link(y, x);
}

void color_graph::destroy(color_graph* result)
{
    for (auto i : result->all_nodes)
    {
        delete i;
    }
}

color_node* color_graph::create_node(color_graph* context)
{
    color_node* result = new color_node();

    context->all_nodes.push_back(result);

    result->color = -1;
    result->context = context;

    return result;
}

static void color_node(color_node* to_color)
{
    std::unordered_set<int> cant_be;

    for (auto i : to_color->links)
    {
        int cant_be_current = i->color;

        if (cant_be_current == -1)
            continue;

        cant_be.insert(cant_be_current);
    }

    for (int i = 0; ; ++i)
    {
        if (in_set(&cant_be,i))
            continue;

        to_color->color = i;

        break;
    }
}

void color_graph::color(color_graph* context)
{
    for (auto i : context->all_nodes)
    {
        if (i->color != -1)
        {
            throw_error();
        }

        color_node(i);
    }
}