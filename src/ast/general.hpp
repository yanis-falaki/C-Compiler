#include <unordered_map>
#include <variant>
#include <stdint.h>
#include <string>

namespace compiler::ast {
    
// ------------------------------> Type <------------------------------

struct Int {};

struct FuncType {
    uint32_t mParamCount;
    FuncType(int paramCount) : mParamCount(paramCount) {}
    bool operator==(const FuncType& other) const {
        return mParamCount == other.mParamCount;
    }
};

using Type = std::variant<Int, FuncType>;

// ------------------------------> Symbol Info <------------------------------

struct SymbolInfo {
    Type mType;
    bool mDefined; // used for functions
    bool mHasExternalLinkage;
    int32_t mStackSize; // used for functions
    SymbolInfo() = default;
    SymbolInfo(Type type, bool defined, bool hasExternalLinkage)
        : mType(std::move(type)), mDefined(defined), mHasExternalLinkage(hasExternalLinkage) {}
};

using SymbolMapType = std::unordered_map<std::string, SymbolInfo>;

}