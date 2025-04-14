#pragma once

#ifdef ENABLE_SKYRIM_VR
    #define SE_AE_VR(se, ae, vr) vr
#elifdef ENABLE_SKYRIM_SE
    #define SE_AE_VR(se, ae, vr) se
#else
    #define SE_AE_VR(se, ae, vr) ae
#endif
