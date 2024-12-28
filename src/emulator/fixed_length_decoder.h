#ifndef FIXED_LENGHT_DECODER_H
#define FIXED_LENGHT_DECODER_H

#include <vector>
#include <string>
#include <unordered_map>

template <typename T>
struct fixed_length_decoder_entry
{
    T           instruction;
    T           mask;

    void*       emit;
    void*       interpret;

    std::string name;

    static bool test(fixed_length_decoder_entry* entry, T value)
    {
        return (value & entry->mask) == entry->instruction;
    }
};

template <typename T>
struct fixed_length_decoder;

template <typename T>
struct fixed_length_decoder
{
    std::vector<fixed_length_decoder_entry<T>>  entries;
    std::unordered_map<std::string, int>        entry_map;

    static fixed_length_decoder_entry<T>* decode_slow(fixed_length_decoder<T>* decoder, T instruction)
    {
        for (int i = 0; i < decoder->entries.size(); ++i)
        {
            if (fixed_length_decoder_entry<T>::test(&decoder->entries[i], instruction))
            {
                return &decoder->entries[i];
            }
        }
        
        return nullptr;
    }

    static void insert_entry(fixed_length_decoder* decoder, T instruction, T mask, void* emit, void* interpret, std::string name)
    {
        fixed_length_decoder_entry<T> working_entry = {instruction, mask, emit, interpret, name};

        decoder->entries.push_back(working_entry);
        decoder->entry_map[working_entry.name] = decoder->entries.size() - 1;
    }

    static fixed_length_decoder_entry<T> get_table_by_name(fixed_length_decoder<T>* decoder, std::string name)
    {
        return decoder->entries[decoder->entry_map[name]];
    }
};

#endif