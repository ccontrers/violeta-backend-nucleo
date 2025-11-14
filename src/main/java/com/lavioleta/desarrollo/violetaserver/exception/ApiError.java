package com.lavioleta.desarrollo.violetaserver.exception;

import java.time.Instant;
import java.util.List;

public record ApiError(
        Instant timestamp,
        int status,
        String error,
        String message,
        String path,
        String method,
        String errorCode,
        List<FieldErrorEntry> fieldErrors,
        String traceId
) {
    public record FieldErrorEntry(String field, String rejectedValue, String message) {}
}
