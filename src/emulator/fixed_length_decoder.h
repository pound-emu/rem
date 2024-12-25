#ifndef FIXED_LENGHT_DECODER_H
#define FIXED_LENGHT_DECODER_H

#include <vector>

template <typename T>
struct fixed_length_decoder_entry
{
    T instruction;
    T mask;

    void* emit;
    void* interpret;

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
    std::vector<fixed_length_decoder_entry<T>> entries;

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

    static void insert_entry(fixed_length_decoder* decoder, T instruction, T mask, void* emit, void* interpret)
    {
        decoder->entries.push_back({instruction, mask, emit, interpret});
    }
};

#endif