#ifndef ENUMERATE_HPP
#define ENUMERATE_HPP

template <typename Iterable>
class EnumerateObject {
    Iterable enumerationIterable;
    std::size_t index;
    decltype(std::begin(enumerationIterable)) enumerationBegin;
    decltype(std::end(enumerationIterable)) enumerationEnd;
    bool isReversed;
    bool isFinished = false;

public:
    EnumerateObject(Iterable iter, const bool reversed = false)
        : enumerationIterable(iter),
          enumerationBegin(std::begin(iter)),
          enumerationEnd(std::end(iter)),
          isReversed(reversed) {
        if (reversed) {
            index = std::distance(enumerationBegin, enumerationEnd) - 1;
            enumerationBegin = enumerationEnd - 1;
        } else {
            index = 0;
        }
    }
    ~EnumerateObject() = default;
    EnumerateObject(const EnumerateObject&) = delete;
    EnumerateObject& operator=(const EnumerateObject&) = delete;
    EnumerateObject(EnumerateObject&&) = delete;
    EnumerateObject& operator=(EnumerateObject&&) = delete;

    const EnumerateObject& begin() const { return *this; }
    const EnumerateObject& end() const { return *this; }

    bool operator!=(const EnumerateObject&) const {
        return isReversed ? isFinished != true : enumerationBegin != enumerationEnd;
    }

    void operator++() {
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

    EnumerateObject reverse() const {
        return enumerate_object(enumerationIterable, !isReversed);
    }
};

template <typename Iterable>
auto enumerate(Iterable&& iter) -> EnumerateObject<Iterable> {
    return {std::forward<Iterable>(iter)};
}

#endif
