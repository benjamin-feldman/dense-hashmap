#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <functional>
#include <limits>
#include <stdexcept>
#include <vector>

template <typename Key, typename Value>
struct DenseMap {
    static constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();

    struct Node {
        Key key { };
        Value value { };
        std::size_t nextIdx = npos;
        bool used = false;
    };

    explicit DenseMap(const std::size_t bucket_count = 4)
        : bucket_count_ { bucket_count }
        , items { std::vector<Node>(vec_size(bucket_count, max_load_factor_)) }
        , bucket_heads_ { std::vector<std::size_t>(bucket_count, npos) }
    {
        if (bucket_count < 1) {
            throw std::invalid_argument("Cannot initialize the DenseMap with no buckets.");
        }
    }

    void set(const Key& k, const Value& v)
    {
        check_load_factor();
        set_impl(k, v);
    }

    [[nodiscard]] const Value& get(const Key& k) const
    {
        const std::size_t bucketIdx = hash(k) % bucket_count_;
        std::size_t idx = bucket_heads_[bucketIdx];

        while (idx != npos) {
            const Node& node = items[idx];
            if (node.key == k && node.used) {
                return node.value;
            }
            idx = node.nextIdx;
        }
        throw std::out_of_range("Key not present in DenseMap.");
    }

private:
    static constexpr double max_load_factor_ = 0.75;

    std::size_t hash(const Key& key) const { return std::hash<Key> { }(key); }

    static std::size_t vec_size(const std::size_t bucket_count, const double load_factor)
    {
        return static_cast<std::size_t>(
            std::ceil(static_cast<double>(bucket_count) * load_factor));
    }

    void check_load_factor()
    {
        if (static_cast<double>(size_) / static_cast<double>(bucket_count_) >= max_load_factor_) {
            bucket_count_ *= 2;
            rehash();
        }
    }

    void set_impl(const Key& k, const Value& v)
    {
        const std::size_t bucketIdx = hash(k) % bucket_count_;
        std::size_t idx = bucket_heads_[bucketIdx];

        if (idx == npos) {
            idx = size_++;
            bucket_heads_[bucketIdx] = idx;
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

            if (node.nextIdx == npos) {
                break;
            }

            idx = node.nextIdx;
        }
        Node& tail = items[idx];
        const std::size_t newIdx = size_++;

        tail.nextIdx = newIdx;
        items[newIdx].key = k;
        items[newIdx].value = v;
        items[newIdx].used = true;
    }

    void rehash()
    {
        std::vector<Node> itemsSnapshot;
        itemsSnapshot.reserve(size_);
        for (const auto& node : items) {
            if (node.used) {
                itemsSnapshot.push_back(node);
            }
        }

        items = std::vector<Node>(vec_size(bucket_count_, max_load_factor_));
        bucket_heads_ = std::vector<std::size_t>(bucket_count_, npos);
        size_ = 0;

        for (const auto& node : itemsSnapshot) {
            set_impl(node.key, node.value);
        }
    }

    std::size_t bucket_count_;
    std::vector<Node> items;
    std::vector<std::size_t> bucket_heads_;
    std::size_t size_ = 0;
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

    bool threw = false;

    try {
        [[maybe_unused]] const int& value = h.get(42);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    assert(threw);

    auto small = DenseMap<int, int>(2);
    small.set(0, 1);
    small.set(1, 2);
    assert(small.get(0) == 1);
    assert(small.get(1) == 2);

    threw = false;
    try {
        [[maybe_unused]] auto bad = DenseMap<int, int>(0);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);

    std::puts("All tests passed!");
}

int main() { tests(); }
