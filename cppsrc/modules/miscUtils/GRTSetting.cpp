#include "GRTSetting.h"
#include "json.hpp"
#include <fstream>

using json = nlohmann::json;

#define MJSON (*(static_cast<json*>(mJson)))

namespace GRTSetting {
    GRTConfigs::GRTConfigs() {
        std::ifstream f("/data/local/tmp/grt_config.json");
        if (!f.fail()) {
            mJson = new json();
            *(static_cast<json*>(mJson)) = json::parse(f);
        }
    }
    GRTConfigs::~GRTConfigs() {
        if (mJson != nullptr) {
            free(static_cast<json*>(mJson));
        }
    }

    const GRTConfigs& GRTConfigs::GetInstance() {
        static GRTConfigs instance;
        return instance;
    }

    bool GRTConfigs::timestampShaderEnabled() const {
        bool res = true; // default is enabled
        if (mJson != nullptr) {
            res = MJSON["GRTTimestampShaderEnabled"];
        }
        return res;
    }

    bool GRTConfigs::colorAAFTDumpEnabled() const {
        bool res = false; // default is enabled
        if (mJson != nullptr) {
            res = MJSON["GRTColorAAFTDumpEnabled"];
        }
        return res;
    }
}