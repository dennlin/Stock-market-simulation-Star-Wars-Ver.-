// Project identifier: AD48FB4835AF347EB0CA8009E24C3B13F8519882

#pragma once

#include <cinttypes>
#include <sstream>
#include <string>
#include <vector>

class P2random {
public:
    static inline void PR_init(std::stringstream &ss,
                               uint32_t seed,
                               uint32_t num_generals,
                               uint32_t num_planets,
                               uint32_t num_deployments,
                               uint32_t arrival_rate) {
        static Prng rng(seed);
        const uint32_t max_force_sensitivity = 100;
        const uint32_t max_quantity = 50;

        long double timestamp = 0;
        for (uint32_t i = 0; i < num_deployments; ++i) {
            uint32_t time_increase = rng() % arrival_rate + 1;
            timestamp += 1.0l / time_increase;
            const char *force_side = rng() % 2 == 0 ? "SITH" : "JEDI";
            uint32_t planet_num = rng() % num_planets;
            uint32_t general_num = rng() % num_generals;
            uint32_t force_sensitivity = rng() % max_force_sensitivity + 1;
            uint32_t quantity = rng() % max_quantity + 1;

            ss << int(timestamp)
               << " "  << force_side
               << " G" << general_num
               << " P" << planet_num
               << " F" << force_sensitivity
               << " #" << quantity
               << "\n";

        } // for
    } // PR_init()

private:
    // Adapted from http://www.pcg-random.org
    struct Prng {
        using result_type = uint32_t;
        explicit Prng() noexcept = default;
        explicit Prng(uint64_t seq) noexcept { reseed(seq); }
        explicit Prng(uint64_t state, uint64_t seq) noexcept { reseed(state, seq); }
        auto reseed(uint64_t seq) noexcept -> void { reseed(init_state, seq); }

        auto reseed(uint64_t state, uint64_t seq) noexcept -> void {
            state_ = 0U;
            inc_ = (seq << 1u) | 1u;
            operator()();
            state_ += state;
            operator()();
        } // reseed()

        auto operator()() noexcept -> result_type {
            auto const oldstate = state_;
            state_ = oldstate * 6364136223846793005ULL + (inc_ | 1);
            auto const xorshifted =
                static_cast<result_type>(((oldstate >> 18u) ^ oldstate) >> 27u);
            auto const rot = static_cast<result_type>(oldstate >> 59u);
            return (xorshifted >> rot) | (xorshifted << ((~rot + 1) & 31));
        } // operator()()

    private:
        constexpr static auto init_state = 0x853c49e6748fea9bULL;
        constexpr static auto init_seq = 0xda3e39cb94b95bdbULL;

        uint64_t state_ = init_state;
        uint64_t inc_ = init_seq;
    };
}; // P2random
