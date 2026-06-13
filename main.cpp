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

    explicit DenseMap(const unsigned int nBuckets = 4)
        : nBuckets { nBuckets }
        , loadFactor { .75f }
        , items { std::vector<Node>(vec_size(nBuckets, loadFactor)) }
        , bucketToStartIdx { std::vector<int>(nBuckets) }
    {
        for (size_t i { }; i < nBuckets; ++i) {
            bucketToStartIdx[i] = -1;
        }
    }

    void _set(const Key& k, const Value& v)
    {
        const int bucketIdx = hash(k) % nBuckets;
        int idx = bucketToStartIdx[bucketIdx];

        if (idx == -1) {
            idx = ++lastUsedIdx;
            bucketToStartIdx[bucketIdx] = idx;
            items[idx].key = k;
            items[idx].value = v;
            items[idx].used = true;
            storedItems++;
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
        storedItems++;
    }

    void set(const Key& k, const Value& v)
    {
        check_load_factor();
        _set(k, v);
    }

    void check_load_factor()
    {
        if (static_cast<float>(storedItems) / nBuckets >= loadFactor) {
            printf("here");
            this->nBuckets *= 2;
            rehash();
            // each bucket has, on average items.size()/nBuckets
            // if items.size()/nBuckets >= loadFactorThreshold, nBuckets *= 2
            // initial conditions : each buckets size is loadFactor so we initialize items with nBuckets*loadFactor size
        }
    }

    [[nodiscard]] Value get(const Key& k) const
    {
        const int bucketIdx = hash(k) % nBuckets;
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

    static size_t vec_size(const unsigned int nBuckets, const float loadFactor)
    {
        return static_cast<int>(static_cast<float>(nBuckets) * loadFactor);
    }

private:
    std::size_t hash(const Key& key) const { return std::hash<Key> { }(key); }

    void rehash()
    {
        std::vector<Node> itemsSnapshot;
        itemsSnapshot.reserve(storedItems);
        for (auto node : items) {
            if (node.used) {
                itemsSnapshot.push_back(node);
            }
        }

        items = std::vector<Node>(vec_size(nBuckets, loadFactor));
        bucketToStartIdx = std::vector<int>(nBuckets);
        lastUsedIdx = -1;
        storedItems = 0;

        for (size_t i { }; i < nBuckets; ++i) {
            bucketToStartIdx[i] = -1;
        }

        for (auto node : itemsSnapshot) {
            _set(node.key, node.value);
        }
    }

    unsigned int nBuckets;
    float loadFactor;
    std::vector<Node> items;
    std::vector<int> bucketToStartIdx;
    int lastUsedIdx = -1;
    unsigned int storedItems = 0;
};

void tests()
{
    auto h = DenseMap<int, int>();

    h.set(0, 1);
    h.set(1, 2);
    h.set(2, 4);
    h.set(3, 8);
    h.set(4, 16);
    h.set(9, 32);
    assert(h.get(0) == 1);
    assert(h.get(1) == 2);
    assert(h.get(2) == 4);
    h.set(0, 20);
    assert(h.get(0) == 20);
    try {
        h.get(42);
    } catch (std::exception& e) {
        assert(e.what() == std::string("Oops"));
    }
    printf("All tests passed!\n");
}

int main() { tests(); }
