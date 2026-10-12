#include "ue_storage.h"

#include "config.h"  // kUeNet

UeStorage::UeStorage() {
    for (uint32_t i = 1; i <= 4; ++i) {
        ues_.push_back(Ue{kUeNet | i, i * 2, i * 2 + 1});
    }
    for (std::size_t i = 0; i < ues_.size(); ++i) {
        byIp_.emplace(ues_[i].ip, i);
        byTeidUl_.emplace(ues_[i].teidUl, i);
    }
}

const Ue* UeStorage::findByIp(uint32_t ip) const {
    auto it = byIp_.find(ip);
    return it == byIp_.end() ? nullptr : &ues_[it->second];
}

const Ue* UeStorage::findByTeidUl(uint32_t teid) const {
    auto it = byTeidUl_.find(teid);
    return it == byTeidUl_.end() ? nullptr : &ues_[it->second];
}
