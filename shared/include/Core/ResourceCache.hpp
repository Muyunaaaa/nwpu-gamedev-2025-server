#pragma once

#include "Core/Common.hpp"
#include "Core/Meta/TypeTraits.hpp"

// common resource cache

namespace moe {
    template<typename ResT>
    using SharedResource = SharedPtr<ResT>;

    template<
            typename ResIdT,
            typename ResT>
    using ResHashMap = UnorderedMap<ResIdT, SharedResource<ResT>>;

    template<
            typename ResIdT,
            typename ResT,
            typename LoaderFunctorT,
            typename DeleterT = void>
    struct ResourceCache {
    public:
        Optional<SharedResource<ResT>> get(ResIdT id) {
            auto it = resources.find(id);
            if (it != resources.end()) {
                return it->second;
            }
            return std::nullopt;
        }

        Optional<ResT*> getRaw(ResIdT id) {
            auto it = resources.find(id);
            if (it != resources.end()) {
                return it->second.get();
            }
            return std::nullopt;
        }

        template<typename... Args>
        using EnableIfInvocable = Meta::EnableIf<
                Meta::IsInvocableV<LoaderFunctorT, Args...>>;

        template<typename... Args, typename = EnableIfInvocable<Args...>>
        Pair<ResIdT, SharedResource<ResT>> load(Args&&... args) {
            ResIdT id;
            if (!recycledIds.empty()) {
                id = recycledIds.front();
                recycledIds.pop_front();
            } else {
                id = idCounter++;
            }

            if constexpr (Meta::IsVoidV<DeleterT>) {
                return *resources.emplace(id, SharedResource<ResT>{LoaderFunctorT{}(std::forward<Args>(args)...)}).first;
            } else {
                auto loadedResource = LoaderFunctorT{}(std::forward<Args>(args)...);
                auto* allocatedResource = new ResT(std::move(*loadedResource));
                return *resources.emplace(id, SharedResource<ResT>{allocatedResource, DeleterT{}}).first;
            }
        }

        void leak(ResIdT id) {
            resources.erase(id);
            recycledIds.push_back(id);
        }

        void destroy() {
            resources.clear();
            recycledIds.clear();
            idCounter = 0;
        }

    private:
        ResIdT idCounter{0};
        Deque<ResIdT> recycledIds;
        ResHashMap<ResIdT, ResT> resources;
    };
}// namespace moe