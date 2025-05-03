

inline std::size_t safeIndex(int idx) {
    return static_cast<std::size_t>(std::max(0, idx));
}
