/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef XLATOR_H
#define XLATOR_H

#include <iostream>
#include <fstream>
#include <memory>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <shared_mutex>
#include <unordered_map>

namespace embers
{

class Xlator
{
  struct Translation {
    uintptr_t pa;
    bool ok;
  };

 private:
  const uintptr_t pagesize_;
  const pid_t pid_;
  const uid_t euid_;
  std::unordered_map<uintptr_t, uintptr_t> xlate_cache_;
  mutable std::shared_mutex mutex_;  // RW-lock to protect the xlate_cache_

  // The layout of each entry in a process' virtual pages as defined in
  // `/proc/[pid]/pagemap` - see `man 5 proc` or consult the online manpages:
  // https://man7.org/linux/man-pages/man5/proc.5.html. We only care about if
  // a particular entry is present and its page frame number (pfn).  More info
  // is available in the entry.
  union pagemap_entry {
    uint64_t val;
    struct {
      uint64_t pfn : 55;        // 54:0
      uint64_t other_bits : 8;  // 55:62
      uint64_t present : 1;     // 63
    };
  };

  /// Returns the pagemap entry for a virtual address by lookup in a process'
  /// pagemap file.
  ///
  /// @param ifstream: Input File Stream for `/proc/[pid]/pagemap`
  /// @param va: Virtual address to lookup in the pagemap.
  ///
  pagemap_entry get_pagemap_entry(std::ifstream &pagemap_file, const uintptr_t va) const
  {
    pagemap_entry pm_ent;
    pm_ent.present = false;
    pm_ent.pfn = 0xdead;

    // Calculate the vpn (virtual page number) from va (virtual address)
    uint64_t vpn = va / pagesize_;

    // Calculate the offset into pagemap file using vpn to find our entry
    std::streampos offset = vpn * sizeof(pm_ent);

    // Lookup and return pagemap_entry
    pagemap_file.seekg(offset);
    if (pagemap_file.tellg() != offset) {
      std::cerr << "WARN: pos after seek: " << std::to_string(pagemap_file.tellg()) << "\n";
    }

    pagemap_file.read(reinterpret_cast<char *>(&pm_ent), sizeof(pm_ent));

    // Ensure entry is marked "Present in RAM"
    if (!pm_ent.present) {
      std::cerr << "WARN: page not in ram!" << "\n";
      return pm_ent;
    }

    return pm_ent;
  }

  Translation translate(uintptr_t va)
  {
    std::ifstream pagemap_file;
    pagemap_file.rdbuf()->pubsetbuf(0, 0);  // Prevents reading more bytes than requested
    pagemap_file.open(std::string("/proc/") + std::to_string(pid_) + std::string("/pagemap"));

    const auto ent = get_pagemap_entry(pagemap_file, va);

    if (!ent.present) {
      return Translation{.pa = 0, .ok = false};
    }
    // Calculate the `pa` (physical address) using the pagemap entry's
    // pfn, our system's `pagesize_`, and the va (for the byte index into the
    // page in the resultant physical address).

    uint64_t pa = (ent.pfn * pagesize_) | (va % pagesize_);
    {
      std::unique_lock lock(mutex_);  // Wr-lock
      // Update translation cache
      xlate_cache_[va & ~(pagesize_ - 1)] = pa & ~(pagesize_ - 1);
    }
    return Translation{.pa = pa, .ok = true};
  }

 public:
  Xlator() : pagesize_(sysconf(_SC_PAGE_SIZE)), pid_(getpid()), euid_(geteuid()) {}
  ~Xlator() {}
  void ClearCache()
  {
    std::unique_lock lock(mutex_);  // Wr-lock
    xlate_cache_.clear();
  }
  // Translation via the pagemap requires 'root' permissions
  bool CanTranslate() { return euid_ == 0; }
  // @note
  // IMPORTANT: Page mappings can change arbitrarily at runtime due to swap
  //            and kernel mm mechanics (copy-on-write, etc). To ensure the
  //            most 'stable' (but no guarantees!) page mapping:
  //
  //            * Prefer using pinned pages
  //            * For READ-ONLY pages: read an address from the page first
  //            * For WRITE-ONLY pages: write an address in the page first
  //
  //            Caller can load the VA content into a volatile variable
  //            prior to calling VAtoPA to help ensure the page mapping
  //            will exist so that a valid PA can be returned.
  //
  Translation VAtoPA(const uintptr_t va, const bool bypass_xlate_cache = false)
  {
    if (!CanTranslate()) {
      return Translation{.pa = 0, .ok = false};
    }
    if (bypass_xlate_cache) {
      return translate(va);
    }
    {
      std::shared_lock lock(mutex_);  // Rd-lock
      const auto it = xlate_cache_.find(va & ~(pagesize_ - 1));
      if (it != xlate_cache_.end()) {
        return Translation{.pa = it->second | (va % pagesize_), .ok = true};
      }
    }
    return translate(va);
  }
};
}  // namespace embers
#endif  // XLATOR_H
