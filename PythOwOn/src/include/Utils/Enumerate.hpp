#ifndef ENUMERATE_HPP
#define ENUMERATE_HPP

template <typename Iterable>
class EnumerateIterable {
    Iterable enumerationIterable;
    size_t index;
    decltype(std::begin(enumerationIterable)) enumerationBegin;
    decltype(std::end(enumerationIterable)) enumerationEnd;
    bool isReversed;
    bool isFinished = false;

public:
    EnumerateIterable(Iterable iter, const bool reversed = false)
        : enumerationIterable(iter),
          enumerationBegin(std::begin(iter)),
          enumerationEnd(std::end(iter)),
          isReversed(reversed) {
        if (enumerationIterable.empty()) {
            isFinished = true;
            return;
        }

        if (reversed) {
            index = std::distance(enumerationBegin, enumerationEnd) - 1;
            enumerationBegin = enumerationEnd - 1;
        } else {
            index = 0;
        }
    }

    const EnumerateIterable& begin() const {  // a
        return *this;
    }
    const EnumerateIterable& end() const { return *this; }

    bool operator!=(const EnumerateIterable&) const {
        if (isFinished) return false;
        return isReversed ? !isFinished : enumerationBegin != enumerationEnd;
    }

    void operator++() {
        if (isFinished) return;
        if (isReversed) {
            if (enumerationBegin == std::begin(enumerationIterable)) {
                isFinished = true;
                return;
            }

            --enumerationBegin;
            --index;
        } else {
            ++enumerationBegin;
            ++index;
        }
    }


    auto operator*() const -> std::pair<std::size_t, decltype(*enumerationBegin)> {
        return {index, *enumerationBegin};
    }

    EnumerateIterable reverse() const {
        return EnumerateIterable(enumerationIterable, !isReversed);
    }
};

template <typename Iterable>
auto enumerate(Iterable&& iter) -> EnumerateIterable<Iterable> {
    return {std::forward<Iterable>(iter)};
}

#endif
