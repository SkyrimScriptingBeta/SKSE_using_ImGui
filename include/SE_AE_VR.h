#pragma once

#ifdef ENABLE_SKYRIM_VR
    #define SE_AE_VR(se, ae, vr) vr
    #define SE_AE(se, ae) 0
#elifdef ENABLE_SKYRIM_SE
    #define SE_AE_VR(se, ae, vr) se
    #define SE_AE(se, ae) se
#else
    #define SE_AE_VR(se, ae, vr) ae
    #define SE_AE(se, ae) ae
#endif
