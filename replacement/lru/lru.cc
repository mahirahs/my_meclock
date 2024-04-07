#include <iostream>
#include <vector>
#include <memory>
#include <list>
#include <bitset>

class MeClockCache {
private:
    struct Page {
        int pageNumber;
        Page(int num) : pageNumber(num) {}
    };

    // Assuming a small size for simplicity; this should be sized based on the cache size and desired false positive rate.
    std::bitset<128> bloomFilter;
    std::list<std::unique_ptr<Page>> fifoQueue;
    size_t capacity;

    // Hash functions for the Bloom filter
    size_t hash1(int pageNumber) {
        return pageNumber % bloomFilter.size();
    }

    size_t hash2(int pageNumber) {
        return (pageNumber * pageNumber) % bloomFilter.size();
    }

    void addToBloomFilter(int pageNumber) {
        bloomFilter.set(hash1(pageNumber));
        bloomFilter.set(hash2(pageNumber));
    }

    bool isInBloomFilter(int pageNumber) {
        return bloomFilter.test(hash1(pageNumber)) && bloomFilter.test(hash2(pageNumber));
    }

    void removeFromBloomFilter(int pageNumber) {
        bloomFilter.reset(hash1(pageNumber));
        bloomFilter.reset(hash2(pageNumber));
    }

public:
    MeClockCache(size_t cap) : capacity(cap) {}

    void accessPage(int pageNumber) {
        auto it = std::find_if(fifoQueue.begin(), fifoQueue.end(), [pageNumber](const std::unique_ptr<Page>& page) {
            return page->pageNumber == pageNumber;
        });

        if (it != fifoQueue.end()) {
            // Page hit: move to the end if not in the bloom filter; otherwise, just update the bloom filter
            if (!isInBloomFilter(pageNumber)) {
                fifoQueue.splice(fifoQueue.end(), fifoQueue, it);
            }
            addToBloomFilter(pageNumber);
        } else {
            // Page miss: add new page, possibly evicting an old one
            if (fifoQueue.size() >= capacity) {
                // Evict the least recently used page not in the bloom filter
                for (auto it = fifoQueue.begin(); it != fifoQueue.end(); ++it) {
                    if (!isInBloomFilter((*it)->pageNumber)) {
                        removeFromBloomFilter((*it)->pageNumber);
                        fifoQueue.erase(it);
                        break;
                    }
                }
            }
            fifoQueue.push_back(std::make_unique<Page>(pageNumber));
            addToBloomFilter(pageNumber);
        }
    }

    void displayCache() {
        std::cout << "Cache Pages: ";
        for (const auto& page : fifoQueue) {
            std::cout << page->pageNumber << " ";
        }
        std::cout << std::endl;
    }
};