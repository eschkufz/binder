#ifndef BINDER_INCLUDE_CLOCK_H
#define BINDER_INCLUDE_CLOCK_H

#include <map>

namespace binder {

template <typename T, typename S>
class Clock {
  public:
    Clock() : hand_(clock_.begin()) { }

    void touch(typename S::const_iterator line) {
      clock_[line->first] = {line, true};
      if (hand_ == clock_.end()) {
        hand_ = clock_.begin();
      }
    }
    typename S::const_iterator evict() {
      while (hand_->second.second) {
        hand_->second.second = false;
        if (++hand_ == clock_.end()) {
          hand_ = clock_.begin();
        }
      } 
      const auto res = hand_->second.first;
      clock_.erase(hand_++);
      return res;
    }

  private:
    typedef typename T::ckey_type key_type;
    typedef std::pair<typename S::const_iterator, bool> val_type;

    std::map<key_type, val_type> clock_;
    typename std::map<key_type, val_type>::iterator hand_;
};

} // namespace binder

#endif
