#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

template <typename T>
struct fixed_length_decoder_entry {
    T instruction;
    T mask;

    void* emit;
    void* interpret;
    void* decoding_help;

    std::string name;

    std::unordered_set<std::string> collisions;

    fixed_length_decoder_entry(T instruction, T mask, void* emit, void* interpret, void* decoding_help, std::string name) {
        this->instruction = instruction;
        this->mask = mask;

        this->emit = emit;
        this->interpret = interpret;
        this->decoding_help = decoding_help;
        this->name = name;
    }

    static bool test(fixed_length_decoder_entry* entry, T value) {
        bool working = (value & entry->mask) == entry->instruction;

        if (entry->decoding_help != nullptr) {
            working = working && ((bool (*)(T))entry->decoding_help)(value);
        }

        return working;
    }

    static std::string create_string_encoding(fixed_length_decoder_entry* entry) {
        std::string result;

        int ins = entry->instruction;
        int mask = entry->mask;

        for (int i = 0; i < 32; ++i) {
            if (((mask >> i) & 1) == 0) {
                result = "-" + result;
            } else {
                if ((ins >> i) & 1) {
                    result = "1" + result;
                } else {
                    result = "0" + result;
                }
            }
        }

        return result;
    }
};

template <typename T>
struct fixed_length_decoder;

template <typename T>
struct fixed_length_decoder {
    std::vector<fixed_length_decoder_entry<T>> entries;
    std::vector<std::vector<fixed_length_decoder_entry<T>>> fast_entries;
    std::unordered_map<std::string, int> entry_map;
    std::mutex append_lock;

    static uint32_t fast_table_lookup(uint32_t instruction) {
        return ((instruction >> 10) & 0x00F) | ((instruction >> 18) & 0xFF0);
    }

    fixed_length_decoder() {
        for (int i = 0; i < 4096; ++i) {
            fast_entries.push_back(std::vector<fixed_length_decoder_entry<T>>());
        }
    }

    static fixed_length_decoder_entry<T>* decode_slow(fixed_length_decoder<T>* decoder, T instruction) {
        for (int i = 0; i < decoder->entries.size(); ++i) {
            if (fixed_length_decoder_entry<T>::test(&decoder->entries[i], instruction)) {
                return &decoder->entries[i];
            }
        }

        return nullptr;
    }

    static fixed_length_decoder_entry<T>* decode_fast(fixed_length_decoder<T>* decoder, T instruction) {
        int index = fast_table_lookup(instruction);

        auto entries = &decoder->fast_entries[index];

        for (int i = 0; i < entries->size(); ++i) {
            if (fixed_length_decoder_entry<T>::test(&(*entries)[i], instruction)) {
                return &(*entries)[i];
            }
        }

        auto result = decode_slow(decoder, instruction);

        decoder->append_lock.lock();

        if (result != nullptr) {
            entries->push_back(*result);
        }

        decoder->append_lock.unlock();

        return result;
    }

    static void insert_entry(fixed_length_decoder* decoder, T instruction, T mask, void* emit, void* interpret, void* decoder_helper, std::string name) {
        fixed_length_decoder_entry<T> working_entry = fixed_length_decoder_entry<T>(instruction, mask, emit, interpret, decoder_helper, name);

        decoder->entries.push_back(working_entry);
        decoder->entry_map[working_entry.name] = decoder->entries.size() - 1;
    }

    static void test_collision_single(fixed_length_decoder* decoder, int index) {
        std::string original_mask = fixed_length_decoder_entry<T>::create_string_encoding(&decoder->entries[index]);

        for (int i = 0; i < decoder->entries.size(); ++i) {
            if (i == index)
                continue;

            std::vector<int> checks;

            std::string test_mask = fixed_length_decoder_entry<T>::create_string_encoding(&decoder->entries[i]);

            for (int c = 0; c < original_mask.size(); ++c) {
                char o = original_mask[c];
                char t = test_mask[c];

                if (o != '-' && t != '-') {
                    checks.push_back(c);
                }
            }

            bool intersects = true;

            if (decoder->entries[i].decoding_help != nullptr || decoder->entries[index].decoding_help != nullptr)
                continue;

            for (auto c : checks) {
                if (original_mask[c] != test_mask[c]) {
                    intersects = false;
                    break;
                }
            }

            if (intersects) {
                decoder->entries[index].collisions.insert(decoder->entries[i].name);
                decoder->entries[i].collisions.insert(decoder->entries[index].name);
            }
        }
    }

    static void test_collisions(fixed_length_decoder<T>* decoder) {
        for (int i = 0; i < decoder->entries.size(); ++i) {
            test_collision_single(decoder, i);
        }

        std::unordered_set<std::string> visited;

        for (int i = 0; i < decoder->entries.size(); ++i) {
            auto entry = decoder->entries[i];

            for (auto c : entry.collisions) {
                auto f = c + entry.name;
                auto b = entry.name + c;

                if (visited.find(f) == visited.end()) {
                    std::cout << entry.name << " -> " << c << std::endl;

                    throw 0;
                }

                visited.insert(f);
                visited.insert(b);
            }
        }
    }

    static fixed_length_decoder_entry<T> get_table_by_name(fixed_length_decoder<T>* decoder, std::string name) {
        return decoder->entries[decoder->entry_map[name]];
    }
};
