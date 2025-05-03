
constexpr double days_in_year=365.0;


inline std::size_t safeIndex(int idx) {
    return static_cast<std::size_t>(std::max(0, idx));
}

// Print Function (will only print if veborse)  
// output wil be like -- label:value
template<typename T>
void Print(const std::string& label, const T& value) {

#if verbose    
    std::cout << " -- " << label << ": " << value << std::endl;
#endif

}