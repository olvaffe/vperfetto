#include "vperfetto.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#include <string.h>
#include <stdlib.h>

static bool sValidFilename(const char* fn) {
    if (!fn) {
        fprintf(stderr, "ERROR: Invalid filename (is null)\n");
        return false;
    }

    if (!strcmp("", fn)) {
        fprintf(stderr, "ERROR: Invalid filename (is empty string)\n");
        return false;
    }

    if (!std::filesystem::exists(fn)) {
        fprintf(stderr, "ERROR: Filename [%s] does not refer to a filesystem object. As absolute: [%s]\n", fn, std::filesystem::absolute(fn).c_str());
        return false;
    }

    if (!std::filesystem::is_regular_file(fn)) {
        fprintf(stderr, "ERROR: Filename [%s] does not refer to a regular file. As absolute: [%s]\n", fn, std::filesystem::absolute(fn).c_str());
        return false;
    }

    if (std::filesystem::is_empty(fn)) {
        fprintf(stderr, "ERROR: Filename [%s] refers to an empty file. As absolute: [%s]\n", fn, std::filesystem::absolute(fn).c_str());
        return false;
    }

    return true;
}

int main(int argc, char** argv) {
    vperfetto::TraceCombineConfig config;

    if ((argc != 4) && (argc != 5)) {
        fprintf(stderr, "%s: error: invalid usage of vperfetto_merge. Usage: vperfetto_merge <guestTraceFile> <hostTraceFile> <combinedTraceFile> [guestClockBootTimeNsWhenHostTracingStarted]>\n", __func__);
        return 1;
    } else {
        const char* guestFile = argv[1];
        const char* hostFile = argv[2];
        const char* combinedFile = argv[3];
        fprintf(stderr, "vperfetto_merge start. Configuration:\n", __func__);
        fprintf(stderr, "guest trace file: %s\n", guestFile);
        fprintf(stderr, "host trace file: %s\n", hostFile);
        fprintf(stderr, "combined trace file: %s\n", combinedFile);

        if (!sValidFilename(guestFile)) return 1;
        if (!sValidFilename(hostFile)) return 1;

        config.guestFile = guestFile;
        config.hostFile = hostFile;
        config.combinedFile = combinedFile;

        config.useGuestAbsoluteTime = false;
        config.useGuestTimeDiff = false;

        if (argc == 5) {
            // User specified guest boottime
            uint64_t guestClockBootTimeNs;
            std::istringstream ss(argv[4]);
            if (!(ss >> guestClockBootTimeNs)) {
                fprintf(stderr, "ERROR: Failed to parse guest clock boot time ns. Provided: [%s]\n", argv[4]);
                return 1;
            } else {
                fprintf(stderr, "using specified guest time diff: %llu\n", (unsigned long long)guestClockBootTimeNs);
                config.guestClockBootTimeNs = guestClockBootTimeNs;
                config.useGuestAbsoluteTime = true;
            }
        } else { // TODO: an arg for guest time diff
            // Derived guest boottime
            fprintf(stderr, "Will derive guest clock boot time and time diff.\n");
        }
    }

    vperfetto::combineTraces(&config);
    return 0;
}
