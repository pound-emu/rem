#ifndef LINIER_SCAN_REGISTER_ALLOCATOR_H
#define LINIER_SCAN_REGISTER_ALLOCATOR_H

struct ir_control_flow_graph;

void linier_scan_register_allocator_pass(ir_control_flow_graph* source);

#endif