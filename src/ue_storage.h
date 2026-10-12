#pragma once

#include <cstddef>        // size_t
#include <cstdint>        // uint32_t
#include <unordered_map>  // unordered_map
#include <vector>         // vector

struct Ue {
    uint32_t ip;
    uint32_t teidDl;
    uint32_t teidUl;
};

class UeStorage {
public:
    UeStorage();

    const Ue* findByIp(uint32_t ip) const;
    const Ue* findByTeidUl(uint32_t teid) const;

private:
    std::vector<Ue> ues_;
    std::unordered_map<uint32_t, std::size_t> byIp_;
    std::unordered_map<uint32_t, std::size_t> byTeidUl_;
};
