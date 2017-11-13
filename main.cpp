#include <stdio.h> 
#include <stdlib.h>
// scanf/printf is used in order to make IO operations faster
// Otherwise, Yandex tests fail with time limit exceeding

#include <limits.h> 
#include <random>
#include <vector>

using std::vector;

std::default_random_engine random_generator(12244);

class LinearHashFunction
{
public:
    explicit LinearHashFunction(int imageSize) : 
        image_size_(imageSize), multiplier_(1), increment_(0)
    {}

    void Randomize()
    {
        std::uniform_int_distribution<int> multiplier_distribution(1, PRIME_BASE_ - 1);
        std::uniform_int_distribution<int> increment_distribution(0, PRIME_BASE_ - 1);
        multiplier_ = multiplier_distribution(random_generator);
        increment_ = increment_distribution(random_generator);
    }

    size_t operator()(size_t value) const
    {
        return (multiplier_*value + increment_) % PRIME_BASE_ % image_size_;
    }

private:
    unsigned long long multiplier_, increment_;
    size_t image_size_;
    static const size_t PRIME_BASE_ = 2147483647;
};

class HashTable
{
public:
    HashTable() : hash_(0)
    {}

    void Initialize(const vector<int>& values, int size)
    {
        // If there is no elements, empty table is the result
        if (values.size() == 0 || size == 0)
        {
            table_.clear();
            return;
        }

        bool have_collisions;
        hash_ = LinearHashFunction(size);
        do
        {
            hash_.Randomize();
            table_.assign(size, UNDEFINED);
            // For each element, add it to the table
            // If the place is already occupied with another value, try hashing again 
            have_collisions = false;
            for (auto it = values.begin(); it != values.end() && !have_collisions; ++it)
            {
                int& current_value = table_[hash_(*it)];
                if (current_value != UNDEFINED && current_value != *it)
                    have_collisions = true;
                current_value = *it;
            }
        } while (have_collisions);
    }

    bool Contains(int element) const
    {
        if (table_.size() == 0)
            return false;
        return element == table_[hash_(element)];
    }

private:
    static const int UNDEFINED = INT_MAX;
    LinearHashFunction hash_;
    vector<int> table_;
};

class FixedSet
{
public:
    FixedSet() : primary_hash_(0)
    {}

    void Initialize(const vector<int>& input_values)
    {
        if (input_values.size() == 0)
            return;

        // Try different hashers until gaining appropriate table size
        primary_hash_ = LinearHashFunction(input_values.size());
        do
        {
            primary_hash_.Randomize();
        } while (!CheckPrimaryHash(input_values, primary_hash_));

        // Apply the first hash function to the input set 
        // and obtain a vector of the collision lists
        vector<vector<int>> secondary_subsets = CreatePrimaryHashTable(input_values, primary_hash_);
        secondary_hash_tables_.resize(secondary_subsets.size());

        // Generate and apply secondary hash functions to the collision lists
        for (int index = 0; index != secondary_subsets.size(); ++index)
        {
            const vector<int>& secondary_values = secondary_subsets[index];
            int table_size = secondary_values.size() * secondary_values.size();
            secondary_hash_tables_[index].Initialize(secondary_values, table_size);
        }
    }

    bool Contains(int element) const
    {
        if (secondary_hash_tables_.size() == 0)
            return false;
        int index = primary_hash_(element);
        return secondary_hash_tables_[index].Contains(element);
    }

private:
    LinearHashFunction primary_hash_;
    vector<HashTable> secondary_hash_tables_;
    static const int MEMORY_SIZE_FACTOR_ = 4;

    // Check if the total size of secondary hash tables is small enough 
    // in case of applying the specified hash function to the given set
    bool CheckPrimaryHash(const vector<int>& values, const LinearHashFunction& hash) const
    {
        vector<int> hash_table_sizes(values.size(), 0);
        for (auto value = values.begin(); value != values.end(); ++value)
        {
            ++hash_table_sizes[hash(*value)];
        }

        int total_size = 0;
        for (auto it = hash_table_sizes.begin(); it != hash_table_sizes.end(); ++it)
        {
            total_size += (*it) * (*it);
        }

        return total_size < MEMORY_SIZE_FACTOR_ * values.size();
    }

    // Create primary hash table (with collisions)
    // Returns vector of subsets, where i-th subset contains all elements with primary hash = i
    vector<vector<int>> CreatePrimaryHashTable(
        const vector<int>& values,
        const LinearHashFunction& hash
        ) const
    {
        vector<vector<int>> hash_table(values.size());
        for (vector<int>::const_iterator value = values.begin(); value != values.end(); ++value)
        {
            hash_table[hash(*value)].push_back(*value);
        }
        return hash_table;
    }
};

vector<int> ReadSequence()
{
    int size;
    scanf("%d", &size);
    vector<int> sequence(size);
    for (auto element = sequence.begin(); element != sequence.end(); ++element)
    {
        scanf("%d", &(*element));
    }
    return sequence;
}

void ProcessQueries(const FixedSet& set, const vector<int>& queries)
{
    for (auto element = queries.begin(); element != queries.end(); ++element)
    { 
        if (set.Contains(*element))
            printf("Yes\n");
        else
            printf("No\n");
    }
}

int main()
{
    vector<int> set_elements = ReadSequence();

    FixedSet set;
    set.Initialize(set_elements);

    ProcessQueries(set, ReadSequence());

    return 0;
}
