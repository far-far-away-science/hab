#pragma once

#define TRACE_AND_RETURN_STATUS_IF_FAILED(methodCall, trace, ...) \
    status = methodCall; \
    if (!NT_SUCCESS(status)) \
    { \
        TraceEvents((TRACE_LEVEL_ERROR), (trace), ##__VA_ARGS__); \
        return status; \
    }
