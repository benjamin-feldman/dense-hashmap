#include <cassert>
#include <cstddef>
#include <functional>
#include <string>
#include <vector>

template <typename Key, typename Value>
struct DenseMap {
    struct Node {
        Key key { };
        Value value { };
        int nextIdx = -1;
        bool used = false;
    };

    DenseMap(unsigned int nBuckets = 4, unsigned int bucket_size = 64)
        : nBuckets { nBuckets }
        , loadFactor { 0.0f }
        , items { std::vector<Node>(nBuckets * bucket_size) }
        , bucketToStartIdx { std::vector<int>(nBuckets) }
    {
        for (size_t i { }; i < nBuckets; ++i) {
            bucketToStartIdx[i] = -1;
        }
    }

    void set(const Key& k, const Value& v)
    {
        int bucketIdx = hash(k) % nBuckets;
        int idx = bucketToStartIdx[bucketIdx];

        if (idx == -1) {
            idx = ++lastUsedIdx;
            bucketToStartIdx[bucketIdx] = idx;
            items[idx].key = k;
            items[idx].value = v;
            items[idx].used = true;
            return;
        }
        while (true) {
            Node& node = items[idx];

            if (node.used && node.key == k) {
                node.value = v;
                return;
            }

            if (node.nextIdx == -1) {
                break;
            }

            idx = node.nextIdx;
        }
        Node& tail = items[idx];
        int newIdx = ++lastUsedIdx;

        tail.nextIdx = newIdx;
        items[newIdx].key = k;
        items[newIdx].value = v;
        items[newIdx].used = true;
    }

    Value get(const Key& k) const
    {
        int bucketIdx = hash(k) % nBuckets;
        int idx = bucketToStartIdx[bucketIdx];

        while (idx != -1) {
            const Node& node = items[idx];
            if (node.key == k && node.used) {
                return node.value;
            }
            idx = node.nextIdx;
        }
        throw std::runtime_error("Oops");
    }

    std::size_t hash(const Key& key) const { return std::hash<Key> { }(key); }

private:
    std::vector<Node> items;
    std::vector<int> bucketToStartIdx;
    unsigned int nBuckets;
    float loadFactor;
    int lastUsedIdx = -1;
};

void tests()
{
    auto h = DenseMap<int, int>();

    h.set(0, 1);
    h.set(1, 2);
    h.set(2, 4);
    h.set(3, 8);
    h.set(4, 16);
    assert(h.get(0) == 1);
    assert(h.get(1) == 2);
    assert(h.get(2) == 4);
    h.set(0, 20);
    assert(h.get(0) == 2` ` 0);
    try {
        h.get(42);
    } catch (std::exception& e) {
        assert(e.what() == std::string("Oops"));
    }
    printf("All tests passed!\n");
}

int main() { tests(); }
