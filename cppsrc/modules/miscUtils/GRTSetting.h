#pragma once

namespace GRTSetting {
    class GRTConfigs {
    public:
        static const GRTConfigs& GetInstance();
        bool timestampShaderEnabled() const;
        bool colorAAFTDumpEnabled() const;
    private:
        GRTConfigs();
        ~GRTConfigs();
        GRTConfigs(const GRTConfigs&) = delete;
        GRTConfigs& operator=(const GRTConfigs&) = delete;
        void* mJson = nullptr;
    };
}