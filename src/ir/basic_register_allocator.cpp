#include "basic_register_allocator.h"
#include "debugging.h"
#include "tools/misc_tools.h"

struct save_state_group
{
	module_save_state gp;
	module_save_state vec;

	static save_state_group create(basic_register_allocator_context* source)
	{
		save_state_group result;

		result.gp = module_save_state::create_save_state(source->gp_allocator->host_registers, source->gp_allocator->host_register_count);
		result.vec = module_save_state::create_save_state(source->vec_allocator->host_registers, source->vec_allocator->host_register_count);

		return result;
	}
};

enum known_global_data
{
	base_register 	= 1 << 0,
	vector_register = 1 << 1,
};

static void create_register_save_state(ir_control_flow_node* node, basic_register_allocator_context* context, std::unordered_map<ir_control_flow_node*, save_state_group>* groups)
{
	save_state_group group = save_state_group::create(context);

	if (groups->find(node) != groups->end())
	{
		throw_error();
	}

	(*groups)[node] = group;
}

static void load_register_save_state(ir_control_flow_node* node, basic_register_allocator_context* context, std::unordered_map<ir_control_flow_node*, save_state_group>* groups)
{
	if (groups->find(node) == groups->end())
	{
		throw_error();
	}

	save_state_group group = (*groups)[node];

	module_save_state::load_save_state(context->gp_allocator->host_registers, &group.gp);
	module_save_state::load_save_state(context->vec_allocator->host_registers, &group.vec);
}

static register_allocator_module* create_allocator_module(basic_register_allocator_context* main_unit, arena_allocator* allocator, int host_count, int guest_count, uint64_t guest_type)
{
	register_allocator_module* result = arena_allocator::allocate_struct< register_allocator_module>(allocator);

	result->allocator_unit = main_unit;

	result->host_register_count = host_count;
	result->host_registers = arena_allocator::allocate_struct<host_register>(allocator, host_count);

	for (int i = 0; i < host_count; ++i)
	{
		host_register* working_guest_register = &result->host_registers[i];

		working_guest_register->host_index = i;
		working_guest_register->guest_offset = -1;

		working_guest_register->lock_data = lock_mode::unlocked;

		working_guest_register->working_mode = register_mode::none;
	}

	result->guest_count = guest_count;
	result->guest_type = guest_type;

	return result;
}

static register_allocator_module* get_module(basic_register_allocator_context* context, ir_operand test)
{
	if (ir_operand::is_vector(&test))
	{
		return context->vec_allocator;
	}

	return context->gp_allocator;
}

static void init_p_lock(basic_register_allocator_context* context, ir_operand reg)
{
	register_allocator_module* working_module = get_module(context, reg);

	assert(ir_operand::is_hardware(&reg));

	host_register* host = &working_module->host_registers[reg.value];

	if (host_register::is_locked(host))
	{
		throw_error();
	}
	else
	{
		register_allocator_module::emit_host_unload(working_module, host->host_index);

		assert(host->guest_offset == -1);

		host->lock_data = lock_mode::force_lock;
		host->guest_offset = -1;
		host->working_mode = register_mode::none;
	}
}

static void init_p_unlock(basic_register_allocator_context* context, ir_operand reg)
{
	register_allocator_module* working_module = get_module(context, reg);

	assert(ir_operand::is_hardware(&reg));

	host_register* host = &working_module->host_registers[reg.value];

	if (host->lock_data & lock_mode::force_lock)
	{
		host_register::unset_lock_bit(host, lock_mode::force_lock);
	}
	else
	{
		throw_error();
	}
}

static int get_guest_offset(register_allocator_module* module, int guest_index)
{
	int byte_count = 1 << module->guest_type;

	return module->stack_offset + (guest_index * byte_count);
}

static int request_guest_register(register_allocator_module* module, int guest_register, register_mode mode)
{
	guest_register = get_guest_offset(module, guest_register);

	for (int i = 0; i < module->host_register_count; ++i)
	{
		host_register* working_host_register = &module->host_registers[i];

		if (working_host_register->guest_offset == guest_register)
		{
			working_host_register->working_mode = (register_mode)(working_host_register->working_mode | mode);
			working_host_register->lock_data = lock_mode::basic_lock;

			working_host_register->hits++;

			return i;
		}
	}

	for (int i = 0; i < module->host_register_count; ++i)
	{
		host_register* working_host_register = &module->host_registers[i];

		if (host_register::is_free(working_host_register))
		{
			register_allocator_module::emit_host_load(module, i, guest_register, mode);

			return i;
		}
	}

	//Is there a faster way than just a linier
	//search?
	int lowest_hit			= INT32_MAX;
	int lowest_hit_index	= -1;

	for (int i = 0; i < module->host_register_count; ++i)
	{
		host_register* working_host_register = &module->host_registers[i];

		if (host_register::is_locked(working_host_register))
			continue;

		if (working_host_register->hits < lowest_hit)
		{
			lowest_hit = working_host_register->hits;
			lowest_hit_index = i;

			if (working_host_register->hits == 0)
			{
				break;
			}
		}
	}

	if (lowest_hit_index == -1)
	{
		throw_error();
	}

	register_allocator_module::emit_host_load(module, lowest_hit_index, guest_register, mode);

	return lowest_hit_index;
}

static void allocate_register(basic_register_allocator_context* context, ir_operand* result, ir_operand* source, register_mode mode)
{
	if (ir_operand::is_constant(source))
	{
		*result = *source;
	}
	else if (ir_operand::is_hardware(source))
	{
		*result = ir_operand::create_reg(source->value, source->meta_data);
	}
	else
	{
		register_allocator_module* working_module = get_module(context, *source);

		int host_register = request_guest_register(working_module, source->value, mode);

		*result = ir_operand::create_reg(host_register, source->meta_data);
	}
}

static void allocate_registers(basic_register_allocator_context* context, ir_operand* result, int result_max, ir_operand* sources, int source_count, register_mode mode)
{
	assert(source_count <= result_max);

	for (int i = 0; i < source_count; ++i)
	{
		allocate_register(context, &result[i], &sources[i], mode);
	}
}

static void unlock_host_basic(host_register* host)
{
	host_register::unset_lock_bit(host, lock_mode::basic_lock);
}

static void unlock_all_basic(register_allocator_module* context)
{
	for (int i = 0; i < context->host_register_count; ++i)
	{
		unlock_host_basic(&context->host_registers[i]);
	}
}

static void unload_all(register_allocator_module* context, bool is_quet, bool is_branch)
{
	for (int i = 0; i < context->host_register_count; ++i)
	{
		register_allocator_module::emit_host_unload(context, i, is_quet, is_branch);
	}
}

static void unlock_all_basic(basic_register_allocator_context* context)
{
	unlock_all_basic(context->gp_allocator);
	unlock_all_basic(context->vec_allocator);
}

static void unload_all(basic_register_allocator_context* context, bool is_quet = false, bool is_branch = false)
{
	unload_all(context->gp_allocator, is_quet, is_branch);
	unload_all(context->vec_allocator, is_quet, is_branch);
}

void host_register::set_lock_bit(host_register* guest, lock_mode mode)
{
	guest->lock_data = (lock_mode)(guest->lock_data | mode);
}

void host_register::unset_lock_bit(host_register* guest, lock_mode mode)
{
	guest->lock_data = (lock_mode)(guest->lock_data & ~mode);
}

bool host_register::is_locked(host_register* guest)
{
	return guest->lock_data != lock_mode::unlocked;
}

bool host_register::is_loaded(host_register* host)
{
	return host->guest_offset != -1;
}

bool host_register::is_free(host_register* host)
{
	return !is_locked(host) && !is_loaded(host);
}

void register_allocator_module::emit_host_load(register_allocator_module* module, int host_index, int guest_offset, register_mode working_mode)
{
	ir_operation_block* result_ir = module->allocator_unit->result_ir;
	host_register* working_host = &module->host_registers[host_index];

	emit_host_unload(module, host_index);

	working_host->guest_offset = guest_offset;
	working_host->working_mode = working_mode;

	host_register::set_lock_bit(working_host, lock_mode::basic_lock);

	if (working_mode & register_mode::read)
	{
		ir_operand offset = ir_operand::create_con(working_host->guest_offset);
		ir_operand result_location = ir_operand::create_reg(working_host->host_index, module->guest_type);

		ir_operation_block::emitds(result_ir, ir_instructions::ir_load, result_location, module->allocator_unit->context_register, offset);
	}
}

static bool is_vector_module(register_allocator_module* module)
{
	return module->guest_type >= int128;
}

void register_allocator_module::emit_host_unload(register_allocator_module* module, int host_index, bool is_quet, bool is_branch)
{
	ir_operation_block* result_ir = module->allocator_unit->result_ir;
	host_register* working_host = &module->host_registers[host_index];

	if (host_register::is_locked(working_host) && host_register::is_loaded(working_host))
	{
		throw_error();
	}

	if (!host_register::is_loaded(working_host))
	{
		return;
	}

	int type_byte_count = 8 << module->guest_type;

	bool ignore = false;

	if (is_branch && module->allocator_unit->use_lrsa_hints)
	{
		ignore = !in_set(&module->allocator_unit->current_lrsa_known_globals, working_host->guest_offset);
	}
	
	if (working_host->working_mode & register_mode::write && !is_quet && !ignore)
	{
		ir_operand offset = ir_operand::create_con(working_host->guest_offset);
		ir_operand to_store = ir_operand::create_reg(working_host->host_index, module->guest_type);

		ir_operation_block::emits(result_ir, ir_instructions::ir_store, module->allocator_unit->context_register, offset, to_store);
	}

	working_host->guest_offset = -1;
	working_host->lock_data = lock_mode::unlocked;
	working_host->working_mode = register_mode::none;
	working_host->hits = 0;
}

static void unload_basic(basic_register_allocator_context* result_register_allocator, bool is_quiet = false, bool is_branch = false)
{
	unlock_all_basic(result_register_allocator);

	unload_all(result_register_allocator, is_quiet, is_branch);
}

static void emit_basic_block(basic_register_allocator_context* result_register_allocator, ir_operation_block* result_ir, ir_control_flow_node* node, std::unordered_map<ir_control_flow_node*, save_state_group>* save_state_groups)
{
	if (save_state_groups->find(node) != save_state_groups->end())
	{
		load_register_save_state(node, result_register_allocator, save_state_groups);
	}

	for (auto i = node->entry_instruction; i != node->final_instruction->next; i = i->next)
	{
		ir_operation working_operation = i->data;

		unlock_all_basic(result_register_allocator);

		switch (working_operation.instruction)
		{
		case ir_instructions::ir_register_allocator_p_lock:
		{
			init_p_lock(result_register_allocator, working_operation.sources[0]);

		}; break;

		case ir_instructions::ir_register_allocator_p_unlock:
		{
			init_p_unlock(result_register_allocator, working_operation.sources[0]);
		}; break;

		case ir_instructions::ir_register_allocator_hint_global:
		{
			if (!result_register_allocator->use_lrsa_hints)
			{
				throw_error();
			}

			ir_operation* working_operation = &i->data;

			result_register_allocator->current_lrsa_known_globals.clear();

			for (int o = 0; o < working_operation->sources.count; ++o)
			{
				ir_operand operand = working_operation->sources[o];

				register_allocator_module* module = get_module(result_register_allocator, operand);

				int offset = get_guest_offset(module, operand.value);

				if (in_set(&result_register_allocator->current_lrsa_known_globals, offset))
				{
					continue;
				}

				result_register_allocator->current_lrsa_known_globals.insert(offset);
			}
		}; break;

		default:
		{
			const int stack_max = 10;

			ir_operand new_sources[stack_max];
			ir_operand new_destinations[stack_max];

			uint64_t instruction = working_operation.instruction;

			allocate_registers(result_register_allocator, new_sources, stack_max, working_operation.sources.data, working_operation.sources.count, register_mode::read);
			allocate_registers(result_register_allocator, new_destinations, stack_max, working_operation.destinations.data, working_operation.destinations.count, register_mode::write);

			if (instruction == ir_external_call || instruction == ir_internal_call)
			{
				unload_basic(result_register_allocator);
			}

			if (i == node->final_instruction)
			{
				switch (node->exit_count)
				{
					case 1:
					{
						ir_control_flow_node* next = node->exits->first->next->data;

						bool is_quiet = false;

						if (next->entry_count == 1)
						{
							create_register_save_state(next, result_register_allocator,save_state_groups);	

							is_quiet = true;		
						}

						unload_basic(result_register_allocator, is_quiet, true);
					}; break;

					case 2:
					{
						ir_control_flow_node* do_branch = node->exits->first->next->data;
						ir_control_flow_node* dont_branch = node->exits->last->prev->data;

						bool is_quiet = false;

						if (do_branch->entry_count == 1 && dont_branch->entry_count == 1)
						{
							create_register_save_state(do_branch, result_register_allocator,save_state_groups);
							create_register_save_state(dont_branch, result_register_allocator,save_state_groups);

							is_quiet = true;
						}

						unload_basic(result_register_allocator, is_quiet, true);
					}; break;

					case 0:
					{
						//DO NOTHING
					}; break;

					default:
					{
						throw_error();
					}; break;
				}
			}

			ir_operation_block::emit_with(result_ir, instruction, new_destinations, working_operation.destinations.count, new_sources, working_operation.sources.count);
		}; break;
		}
	}
}

void basic_register_allocator_context::run_pass(basic_register_allocator_context* result_register_allocator, ir_operation_block* result_ir, ir_operation_block* pre_allocated_code, int gp_count, guest_data gp_data, int vec_count, guest_data vec_data, ir_operand context_register, bool use_lrsa_hints)
{
	arena_allocator* allocator = result_ir->allocator;

	result_register_allocator->use_lrsa_hints = use_lrsa_hints;

	result_register_allocator->gp_allocator = create_allocator_module(result_register_allocator, allocator, gp_count, gp_data.guest_count, gp_data.guest_type);
	result_register_allocator->vec_allocator = create_allocator_module(result_register_allocator, allocator, vec_count, vec_data.guest_count, vec_data.guest_type);

	result_register_allocator->gp_allocator->stack_offset = 0;
	result_register_allocator->vec_allocator->stack_offset = (gp_data.guest_count * 8);

	result_register_allocator->context_register = context_register;

	result_register_allocator->result_ir = result_ir;

	ir_control_flow_graph* cfg = ir_control_flow_graph::create(pre_allocated_code);

	std::unordered_map<ir_control_flow_node*, save_state_group> saves;

	//ir_operation_block::log(pre_allocated_code);

	for (auto i = cfg->linier_nodes->first; i != nullptr; i = i->next)
	{
		if (i->data == nullptr)	
			continue;

		auto node = i->data;

		emit_basic_block(result_register_allocator, result_ir, i->data, &saves);
	}

	//ir_operation_block::log(result_ir);
}