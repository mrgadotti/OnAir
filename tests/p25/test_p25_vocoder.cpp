// The vocoder built into OnAir (third_party/mbelib): IMBE (P25) and AMBE (D-STAR, NXDN) must be available with no library installed,
// and decoding frames must give 160 samples of audio each. Skipped when OnAir was built without it (-DDECT2_BUNDLE_MBELIB=OFF).
#ifdef _WIN32
#include <stdlib.h>
#define setenv(k, v, o) _putenv_s(k, v)
#endif
#include "dect2/p25_imbe.h"
#include "dect2/dstar_ambe.h"
#include "dect2/nxdn_ambe.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
using namespace dect2;
static int fails = 0;
#define CHECK(c, ...) do { if (!(c)) { printf("FAIL line %d: ", __LINE__); printf(__VA_ARGS__); printf("\n"); fails++; } } while (0)

int main() {
#ifndef DECT2_BUILTIN_MBE_TEST
    setenv("DECT2_NATIVE_LIBDIR", "/nonexistent", 1);   // no installed library may be picked up: only the built-in one
#endif
    std::string p1, p2, p3;
    const bool a = p25ImbeAvailable(&p1), b = dstarAmbeAvailable(&p2), c = nxdnAmbeAvailable(&p3);
    if (!a && !b && !c) { printf("no built-in vocoder in this build: skipped\n"); return 0; }
    CHECK(a && b && c, "available: imbe %d dstar %d nxdn %d", a, b, c);
    CHECK(p1 == "built into OnAir" && p2 == p1 && p3 == p1, "path: %s / %s / %s", p1.c_str(), p2.c_str(), p3.c_str());

    P25Imbe imbe; DstarAmbe dstar; NxdnAmbe nxdn;
    CHECK(imbe.ok() && dstar.ok() && nxdn.ok(), "decoders");
    int16_t pcm[160];
    char fr[8][23] = {};
    uint8_t bits[72] = {};
    srand(1);
    int nonzero = 0;
    for (int n = 0; n < 40; n++) {
        for (auto& r : fr) for (char& x : r) x = rand() & 1;
        for (auto& x : bits) x = rand() & 1;
        memset(pcm, 0x55, sizeof pcm); imbe.decode(fr, pcm);
        for (int16_t s : pcm) nonzero += s != 0x5555;
        memset(pcm, 0x55, sizeof pcm); dstar.decode(bits, pcm);
        for (int16_t s : pcm) nonzero += s != 0x5555;
        memset(pcm, 0x55, sizeof pcm); nxdn.decode(bits, pcm);
        for (int16_t s : pcm) nonzero += s != 0x5555;
    }
    CHECK(nonzero == 40 * 3 * 160, "every decode fills its 160 samples (%d of %d)", nonzero, 40 * 3 * 160);
    printf(fails ? "%d failure(s)\n" : "all passed\n", fails);
    return fails ? 1 : 0;
}
