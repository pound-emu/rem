#ifndef BRANCH_TYPE_H
#define BRANCH_TYPE_H

enum branch_type
{
    no_branch       = 0,
    short_branch    = 1 << 0,
    long_branch     = 1 << 1
};

#endif