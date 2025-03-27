

#ifndef IR_H
#define IR_H

#include <unordered_map>
#include <unordered_set>
#include <string>

#include "memory_tools.h"

enum ir_instructions : uint64_t
{
	//Binary Operations
	ir_binary_start,

	ir_add,
	ir_bitwise_and,
	ir_bitwise_exclusive_or,
	ir_bitwise_or,
	ir_compare_equal,
	ir_compare_greater_equal_signed,
	ir_compare_greater_equal_unsigned,
	ir_compare_greater_signed,
	ir_compare_greater_unsigned,
	ir_compare_less_equal_signed,
	ir_compare_less_equal_unsigned,
	ir_compare_less_signed,
	ir_compare_less_unsigned,
	ir_compare_not_equal,
	ir_divide_signed,
	ir_divide_unsigned,
	ir_multiply,
	ir_multiply_hi_signed,
	ir_multiply_hi_unsigned,
	ir_rotate_right,
	ir_shift_left,
	ir_shift_right_signed,
	ir_shift_right_unsigned,
	ir_subtract,
	ir_floating_point_add,
	ir_floating_point_subtract,
	ir_floating_point_multiply,
	ir_floating_point_divide,
	ir_floating_point_select_min,
	ir_floating_point_select_max,
	ir_floating_point_compare_equal,
	ir_floating_point_compare_not_equal,
	ir_floating_point_compare_less,
	ir_floating_point_compare_less_equal,
	ir_floating_point_compare_greater,
	ir_floating_point_compare_greater_equal,

	ir_binary_end,

	//Unary Operations
	ir_unary_start,

	ir_bitwise_not,
	ir_incrament,
	ir_decrament,
	ir_move,
	ir_negate,
	ir_sign_extend, 
	ir_zero_extend,
	ir_logical_not,
	ir_convert_to_float_signed,
	ir_convert_to_float_unsigned,
	ir_convert_to_integer_signed,
	ir_convert_to_integer_unsigned,
	ir_floating_point_square_root,

	ir_unary_end,

	//Ternary Operations
	ir_ternary_begin,

	ir_conditional_select,
	
	/*
	ir_conditional_select_equal,
	ir_conditional_select_not_equal,
	ir_conditional_select_less_signed,
	ir_conditional_select_less_unsigned,
	ir_conditional_select_greater_signed,
	ir_conditional_select_greater_unsigned,
	ir_conditional_select_less_equal_signed,
	ir_conditional_select_less_equal_unsigned,
	ir_conditional_select_greater_equal_signed,
	ir_conditional_select_greater_equal_unsigned,
	*/

	ir_double_shift_right,

	ir_ternary_end,

	//Jumping
	ir_jump_if,
	ir_jump_if_equal,
	ir_jump_if_not_equal,
	ir_jump_if_less_signed,	
	ir_jump_if_less_equal_signed,
	ir_jump_if_less_unsigned,	
	ir_jump_if_less_equal_unsigned,
	ir_jump_if_greater_signed,	
	ir_jump_if_greater_equal_signed,
	ir_jump_if_greater_unsigned,	
	ir_jump_if_greater_equal_unsigned,
	ir_mark_label,

	//Abi
	ir_close_and_return,
	ir_get_argument,
	ir_external_call,
	ir_internal_call,
	ir_table_jump,

	//Memory
	ir_load,
	ir_store,

	//Ir Helpers
	ir_no_operation,
	ir_open_context,
	ir_register_allocator_p_lock,
	ir_register_allocator_p_unlock,
	ir_register_allocator_hint_global,
	ir_ssa_phi,

	//Vectors
	ir_vector_extract,
	ir_vector_insert,
	ir_vector_zero,
	ir_vector_one,

	//Asserts
	ir_assert_false,
	ir_assert_true,

	//X86
	x86_cqo,
	x86_cdq,
	x86_cwd,
	x86_lea,
	x86_cvtsi2ss,
	x86_cvtsi2sd,
	x86_cvtsd2si,
	x86_cvtss2si,
	x86_movq_to_gp,
	x86_movq_to_vec,

	x86_addpd,
	x86_addps,
	x86_addsd,
	x86_addss,
	x86_divpd,
	x86_divps,
	x86_divsd,
	x86_divss,
	x86_maxpd,
	x86_maxps,
	x86_maxsd,
	x86_maxss,
	x86_minpd,
	x86_minps,
	x86_minsd,
	x86_minss,
	x86_mulpd,
	x86_mulps,
	x86_mulsd,
	x86_mulss,
	x86_subpd,
	x86_subps,
	x86_subsd,
	x86_subss,

	x86_paddb,
	x86_paddw,
	x86_paddd,
	x86_paddq,

	x86_cmpss,
	x86_cmpsd,
	x86_cmpps,
	x86_cmppd,
	x86_haddps,
	x86_haddpd,

	x86_xorps,
	x86_pand,
	x86_orps,
	x86_pandn,

	x86_shufps,
	x86_shufpd,
	x86_roundss,
	x86_roundsd,

	x86_sqrtss,
	x86_sqrtsd,	
	x86_sqrtps,
	x86_sqrtpd,
	x86_popcnt,
	x86_cvtsd2ss,
	x86_cvtss2sd,

	x86_add_flags,
	x86_sub_flags,

	x86_cmpxchg,
	
	x86_lzcnt,

	//Emulator Helpers
	ir_guest_store_context,
	ir_guest_load_context,
};

static std::string instruction_names[] = {
	//Binary Operations
	"ir_binary_start",

	"ir_add",
	"ir_bitwise_and",
	"ir_bitwise_exclusive_or",
	"ir_bitwise_or",
	"ir_compare_equal",
	"ir_compare_greater_equal_signed",
	"ir_compare_greater_equal_unsigned",
	"ir_compare_greater_signed",
	"ir_compare_greater_unsigned",
	"ir_compare_less_equal_signed",
	"ir_compare_less_equal_unsigned",
	"ir_compare_less_signed",
	"ir_compare_less_unsigned",
	"ir_compare_not_equal",
	"ir_divide_signed",
	"ir_divide_unsigned",
	"ir_multiply",
	"ir_multiply_hi_signed",
	"ir_multiply_hi_unsigned",
	"ir_rotate_right",
	"ir_shift_left",
	"ir_shift_right_signed",
	"ir_shift_right_unsigned",
	"ir_subtract",
	"ir_floating_point_add",
	"ir_floating_point_subtract",
	"ir_floating_point_multiply",
	"ir_floating_point_divide",
	"ir_floating_point_select_min",
	"ir_floating_point_select_max",
	"ir_floating_point_compare_equal",
	"ir_floating_point_compare_not_equal",
	"ir_floating_point_compare_less",
	"ir_floating_point_compare_less_equal",
	"ir_floating_point_compare_greater",
	"ir_floating_point_compare_greater_equal",

	"ir_binary_end",

	//Unary Operations
	"ir_unary_start",

	"ir_bitwise_not",
	"ir_incrament",
	"ir_decrament",
	"ir_move",
	"ir_negate",
	"ir_sign_extend", 
	"ir_zero_extend",
	"ir_logical_not",
	"ir_convert_to_float_signed",
	"ir_convert_to_float_unsigned",
	"ir_convert_to_integer_signed",
	"ir_convert_to_integer_unsigned",
	"ir_floating_point_square_root",

	"ir_unary_end",

	//Ternary Operations
	"ir_ternary_begin",

	"ir_conditional_select",
	"ir_double_shift_right",

	"ir_ternary_end",

	//Jumping
	"ir_jump_if",
	"ir_jump_if_equal",
	"ir_jump_if_not_equal",
	"ir_jump_if_less_signed",	
	"ir_jump_if_less_equal_signed",
	"ir_jump_if_less_unsigned",	
	"ir_jump_if_less_equal_unsigned",
	"ir_jump_if_greater_signed",	
	"ir_jump_if_greater_equal_signed",
	"ir_jump_if_greater_unsigned",	
	"ir_jump_if_greater_equal_unsigned",
	"ir_mark_label",

	//Abi
	"ir_close_and_return",
	"ir_get_argument",
	"ir_external_call",
	"ir_internal_call",
	"ir_table_jump",

	//Memory
	"ir_load",
	"ir_store",

	//Ir Helpers
	"ir_no_operation",
	"ir_open_context",
	"ir_register_allocator_p_lock",
	"ir_register_allocator_p_unlock",
	"ir_register_allocator_hint_global",
	"ir_ssa_phi",

	//Vectors
	"ir_vector_extract",
	"ir_vector_insert",
	"ir_vector_zero",
	"ir_vector_one",

	//Asserts
	"ir_assert_false",
	"ir_assert_true",

	//X86
	"x86_cqo",
	"x86_cdq",
	"x86_cwd",
	"x86_lea",
	"x86_cvtsi2ss",
	"x86_cvtsi2sd",
	"x86_cvtsd2si",
	"x86_cvtss2si",
	"x86_movq_to_gp",
	"x86_movq_to_vec",

	"x86_addpd",
	"x86_addps",
	"x86_addsd",
	"x86_addss",
	"x86_divpd",
	"x86_divps",
	"x86_divsd",
	"x86_divss",
	"x86_maxpd",
	"x86_maxps",
	"x86_maxsd",
	"x86_maxss",
	"x86_minpd",
	"x86_minps",
	"x86_minsd",
	"x86_minss",
	"x86_mulpd",
	"x86_mulps",
	"x86_mulsd",
	"x86_mulss",
	"x86_subpd",
	"x86_subps",
	"x86_subsd",
	"x86_subss",

	"x86_paddb",
	"x86_paddw",
	"x86_paddd",
	"x86_paddq",

	"x86_cmpss",
	"x86_cmpsd",
	"x86_cmpps",
	"x86_cmppd",
	"x86_haddps",
	"x86_haddpd",

	"x86_xorps",
	"x86_pand",
	"x86_orps",
	"x86_pandn",

	"x86_shufps",
	"x86_shufpd",
	"x86_roundss",
	"x86_roundsd",

	"x86_sqrtss",
	"x86_sqrtsd",	
	"x86_sqrtps",
	"x86_sqrtpd",
	"x86_popcnt",
	"x86_cvtsd2ss",
	"x86_cvtss2sd",

	"x86_add_flags",
	"x86_sub_flags",

	"x86_cmpxchg",
	
	"x86_lzcnt",

	//Emulator Helpers
	"ir_guest_store_context",
	"ir_guest_load_context",
};

enum ir_operand_meta : uint64_t
{
	int8,
	int16,
	int32,
	int64,
	int128,

	top,

	is_hardware_register	= 1ULL << 32,
	is_constant				= 1ULL << 33
};

struct ir_operand
{
	uint64_t value;
	uint64_t meta_data;

	static bool			is_register(ir_operand* test);
	static bool			is_constant(ir_operand* test);
	static bool			is_hardware(ir_operand* test);
	static bool			is_vector(ir_operand* test);

	static uint64_t		get_masked_constant(ir_operand* test);

	static uint64_t		get_raw_size(ir_operand* test);
	static uint64_t		get_meta(ir_operand* test);

	static void			set_raw_size(ir_operand* test, uint64_t new_size);

	static ir_operand	create_con(uint64_t value, uint64_t size = ir_operand_meta::int64);
	static ir_operand	create_reg(uint64_t value, uint64_t size = ir_operand_meta::int64);
	static ir_operand	create_hardware_reg(uint64_t value, uint64_t size = ir_operand_meta::int64);
	static ir_operand	copy_new_raw_size(ir_operand source, uint64_t new_size);

	static bool 		are_equal(ir_operand left, ir_operand right);
	static bool 		same_size(ir_operand left, ir_operand right);
};

struct ir_operation
{
	ir_instructions			instruction;
	fast_array<ir_operand>	destinations;
	fast_array<ir_operand>	sources;
	int 					node_id;
};

struct ir_operation_block
{
	arena_allocator*									allocator;
	intrusive_linked_list<ir_operation>*				operations;
	uint64_t											label_index;
	uint64_t											local_index;

	static void											create(ir_operation_block** result,arena_allocator* allocator);
	static void											create_raw_operation(arena_allocator* allocator, ir_operation* result, uint64_t instruction, int destination_count, int source_count);
	static void 										create_vector_gp_remap_scheme(arena_allocator* allocator, std::unordered_map<uint64_t, uint64_t>* remap_store, std::unordered_map<uint64_t, uint64_t>** remap_redirect);
	static void 										clamp_operands(ir_operation_block* ir, bool use_bit_register_allocations, int* size_counts = nullptr);
	static void 										ssa_remap(ir_operation_block* ir, std::unordered_map<uint64_t, uint64_t>* remap_data);

	static ir_operand 									create_label(ir_operation_block* block);
	static void 										mark_label(ir_operation_block* block, ir_operand label);
	static void 										jump(ir_operation_block* block, ir_operand label);
	static void 										jump_if(ir_operation_block* block, ir_operand label, ir_operand condition);
	static void 										jump_if_not(ir_operation_block* block, ir_operand label, ir_operand condition);
	static void 										get_used_registers(std::unordered_set<uint64_t>* result, ir_operation_block* block);

	static void 										copy(ir_operation_block* result, ir_operation_block* source);

	static std::string 									get_block_log(ir_operation_block* ir);
	static void 										log(ir_operation_block* ctx);
	static void 										remove_redundant_moves(ir_operation_block* ctx);

	static bool 										is_label(ir_operation* operation);
	static bool 										is_jump(ir_operation* operation);
	static bool 										is_compare(ir_operation* operation);
	static bool 										ends_control_flow(ir_operation* operation);

	static intrusive_linked_list_element<ir_operation>* emit(ir_operation_block* block,ir_operation operation, intrusive_linked_list_element<ir_operation>* point = nullptr);
	static intrusive_linked_list_element<ir_operation>* emit_with(ir_operation_block* ir, uint64_t instruction, ir_operand* destinations, int destination_count, ir_operand* sources, int source_count, intrusive_linked_list_element<ir_operation>* point = nullptr);

	static intrusive_linked_list_element<ir_operation>* emitds(ir_operation_block* ir, uint64_t instruction, ir_operand destination_0, intrusive_linked_list_element<ir_operation>* point = nullptr);
	static intrusive_linked_list_element<ir_operation>* emitds(ir_operation_block* ir, uint64_t instruction, ir_operand destination_0, ir_operand source_0, intrusive_linked_list_element<ir_operation>* point = nullptr);
	static intrusive_linked_list_element<ir_operation>* emitds(ir_operation_block* ir, uint64_t instruction, ir_operand destination_0, ir_operand source_0, ir_operand source_1, intrusive_linked_list_element<ir_operation>* point = nullptr);
	static intrusive_linked_list_element<ir_operation>* emitds(ir_operation_block* ir, uint64_t instruction, ir_operand destination_0, ir_operand source_0, ir_operand source_1, ir_operand source_2, intrusive_linked_list_element<ir_operation>* point = nullptr);
	static intrusive_linked_list_element<ir_operation>* emitds(ir_operation_block* ir, uint64_t instruction, ir_operand destination_0, ir_operand source_0, ir_operand source_1, ir_operand source_2, ir_operand source_3, intrusive_linked_list_element<ir_operation>* point = nullptr);
	static intrusive_linked_list_element<ir_operation>* emitds(ir_operation_block* ir, uint64_t instruction, ir_operand destination_0, ir_operand source_0, ir_operand source_1, ir_operand source_2, ir_operand source_3, ir_operand source_4, intrusive_linked_list_element<ir_operation>* point = nullptr);
	static intrusive_linked_list_element<ir_operation>* emitds(ir_operation_block* ir, uint64_t instruction, ir_operand destination_0, ir_operand source_0, ir_operand source_1, ir_operand source_2, ir_operand source_3, ir_operand source_4, ir_operand source_5, intrusive_linked_list_element<ir_operation>* point = nullptr);
	static intrusive_linked_list_element<ir_operation>* emitds(ir_operation_block* ir, uint64_t instruction, ir_operand destination_0, ir_operand source_0, ir_operand source_1, ir_operand source_2, ir_operand source_3, ir_operand source_4, ir_operand source_5,ir_operand source_6, intrusive_linked_list_element<ir_operation>* point = nullptr);
	static intrusive_linked_list_element<ir_operation>* emits(ir_operation_block* ir, uint64_t instruction, intrusive_linked_list_element<ir_operation>* point = nullptr);
	static intrusive_linked_list_element<ir_operation>* emits(ir_operation_block* ir, uint64_t instruction, ir_operand source_0, intrusive_linked_list_element<ir_operation>* point = nullptr);
	static intrusive_linked_list_element<ir_operation>* emits(ir_operation_block* ir, uint64_t instruction, ir_operand source_0, ir_operand source_1, intrusive_linked_list_element<ir_operation>* point = nullptr);
	static intrusive_linked_list_element<ir_operation>* emits(ir_operation_block* ir, uint64_t instruction, ir_operand source_0, ir_operand source_1, ir_operand source_2, intrusive_linked_list_element<ir_operation>* point = nullptr);
	static intrusive_linked_list_element<ir_operation>* emits(ir_operation_block* ir, uint64_t instruction, ir_operand source_0, ir_operand source_1, ir_operand source_2, ir_operand source_3, intrusive_linked_list_element<ir_operation>* point = nullptr);
	static intrusive_linked_list_element<ir_operation>* emits(ir_operation_block* ir, uint64_t instruction, ir_operand source_0, ir_operand source_1, ir_operand source_2, ir_operand source_3, ir_operand source_4, intrusive_linked_list_element<ir_operation>* point = nullptr);
};

struct ir_control_flow_node
{
	int 											label_id;

	intrusive_linked_list_element<ir_operation>* 	entry_instruction;
	intrusive_linked_list_element<ir_operation>* 	final_instruction;

	int 											entry_count;
	intrusive_linked_list<ir_control_flow_node*>*	entries;

	int 											exit_count;
	intrusive_linked_list<ir_control_flow_node*>*	exits;
};

struct ir_control_flow_graph
{
	ir_operation_block*																source_ir;
	intrusive_linked_list<ir_control_flow_node*>*									linier_nodes;

	static ir_control_flow_graph* 													create(ir_operation_block* source);
};

#endif