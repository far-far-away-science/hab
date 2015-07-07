#pragma once

#define WPP_CONTROL_GUIDS                                            \
    WPP_DEFINE_CONTROL_GUID(                                         \
        uartdriverTraceGuid, (37f79f75,a9ac,46ad,b69c,61d2e9aa5bd5), \
                                                                     \
        WPP_DEFINE_BIT(MYDRIVER_ALL_INFO)                            \
        WPP_DEFINE_BIT(TRACE_DRIVER)                                 \
        WPP_DEFINE_BIT(TRACE_DEVICE)                                 \
        WPP_DEFINE_BIT(TRACE_SERCX2)                                 \
        WPP_DEFINE_BIT(TRACE_INTERRUPT)                              \
        WPP_DEFINE_BIT(TRACE_TRANSMIT)                               \
        WPP_DEFINE_BIT(TRACE_RECEIVE)                                \
        WPP_DEFINE_BIT(TRACE_BCM_2836_CONTROLLER)                    \
        WPP_DEFINE_BIT(TRACE_BCM_2836_REGISTERS)                     \
        )

#define WPP_FLAG_LEVEL_LOGGER(flag, level) \
    WPP_LEVEL_LOGGER(flag)

#define WPP_FLAG_LEVEL_ENABLED(flag, level)        \
    (WPP_LEVEL_ENABLED(flag) &&                    \
     WPP_CONTROL(WPP_BIT_ ## flag).Level >= level)

#define WPP_LEVEL_FLAGS_LOGGER(lvl,flags) \
           WPP_LEVEL_LOGGER(flags)
               
#define WPP_LEVEL_FLAGS_ENABLED(lvl, flags)                                          \
           (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level >= lvl)

//
// This comment block is scanned by the trace preprocessor to define our
// Trace function.
//
// begin_wpp config
// FUNC Trace{FLAG=MYDRIVER_ALL_INFO}(LEVEL, MSG, ...);
// FUNC TraceEvents(LEVEL, FLAGS, MSG, ...);
// end_wpp
//
