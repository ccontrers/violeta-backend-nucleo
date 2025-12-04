package com.lavioleta.desarrollo.violetaserver.exception;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.slf4j.MDC;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.http.converter.HttpMessageNotReadableException;
import org.springframework.security.access.AccessDeniedException;
import org.springframework.security.authorization.AuthorizationDeniedException;
import org.springframework.web.HttpMediaTypeNotSupportedException;
import org.springframework.web.HttpRequestMethodNotSupportedException;
import org.springframework.web.bind.MethodArgumentNotValidException;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.RestControllerAdvice;
import org.springframework.web.servlet.resource.NoResourceFoundException;

import jakarta.servlet.http.HttpServletRequest;
import java.time.Instant;
import java.util.List;
import java.util.stream.Collectors;

@RestControllerAdvice
public class GlobalExceptionHandler {

    private static final Logger logger = LoggerFactory.getLogger(GlobalExceptionHandler.class);

    private ApiError build(HttpServletRequest req, HttpStatus status, String message, String errorCode, List<ApiError.FieldErrorEntry> fieldErrors) {
        return new ApiError(
                Instant.now(),
                status.value(),
                status.getReasonPhrase(),
                message,
                req != null ? req.getRequestURI() : null,
                req != null ? req.getMethod() : null,
                errorCode,
                fieldErrors,
                MDC.get("traceId")
        );
    }

    @ExceptionHandler(MethodArgumentNotValidException.class)
    public ResponseEntity<ApiError> handleValidation(MethodArgumentNotValidException ex, HttpServletRequest req) {
        logger.warn("Validation error: {} fields invalid", ex.getBindingResult().getErrorCount());
        List<ApiError.FieldErrorEntry> fieldErrors = ex.getBindingResult().getFieldErrors().stream()
                .map(fe -> new ApiError.FieldErrorEntry(
                        fe.getField(),
                        fe.getRejectedValue() != null ? String.valueOf(fe.getRejectedValue()) : null,
                        fe.getDefaultMessage()))
                .collect(Collectors.toList());
        ApiError body = build(req, HttpStatus.BAD_REQUEST, "Error de validación", "VALIDATION_ERROR", fieldErrors);
        return ResponseEntity.status(HttpStatus.BAD_REQUEST).body(body);
    }

    @ExceptionHandler(HttpMessageNotReadableException.class)
    public ResponseEntity<ApiError> handleMalformed(HttpMessageNotReadableException ex, HttpServletRequest req) {
        logger.warn("JSON parse error: {}", ex.getMostSpecificCause().getMessage());
        ApiError body = build(req, HttpStatus.BAD_REQUEST, "JSON malformado", "MALFORMED_JSON", null);
        return ResponseEntity.status(HttpStatus.BAD_REQUEST).body(body);
    }

    @ExceptionHandler(HttpMediaTypeNotSupportedException.class)
    public ResponseEntity<ApiError> handleUnsupported(HttpMediaTypeNotSupportedException ex, HttpServletRequest req) {
        logger.warn("Unsupported media type: {}", ex.getMessage());
        ApiError body = build(req, HttpStatus.UNSUPPORTED_MEDIA_TYPE, "Tipo de media no soportado", "UNSUPPORTED_MEDIA_TYPE", null);
        return ResponseEntity.status(HttpStatus.UNSUPPORTED_MEDIA_TYPE).body(body);
    }

    @ExceptionHandler(HttpRequestMethodNotSupportedException.class)
    public ResponseEntity<ApiError> handleMethodNotSupported(HttpRequestMethodNotSupportedException ex, HttpServletRequest req) {
        logger.warn("Method not supported: {}", ex.getMessage());
        ApiError body = build(req, HttpStatus.METHOD_NOT_ALLOWED, "Método HTTP no permitido", "METHOD_NOT_ALLOWED", null);
        return ResponseEntity.status(HttpStatus.METHOD_NOT_ALLOWED).body(body);
    }

    @ExceptionHandler(NoResourceFoundException.class)
    public ResponseEntity<ApiError> handleNotFound(NoResourceFoundException ex, HttpServletRequest req) {
        logger.info("Resource not found: {}", ex.getResourcePath());
        ApiError body = build(req, HttpStatus.NOT_FOUND, "Recurso no encontrado", "RESOURCE_NOT_FOUND", null);
        return ResponseEntity.status(HttpStatus.NOT_FOUND).body(body);
    }

    @ExceptionHandler({AccessDeniedException.class, AuthorizationDeniedException.class})
    public ResponseEntity<ApiError> handleAccessDenied(Exception ex, HttpServletRequest req) {
        logger.warn("Access denied: {}", ex.getMessage());
        ApiError body = build(req, HttpStatus.FORBIDDEN, "Acceso denegado", "ACCESS_DENIED", null);
        return ResponseEntity.status(HttpStatus.FORBIDDEN).body(body);
    }

    @ExceptionHandler(RuntimeException.class)
    public ResponseEntity<ApiError> handleRuntimeException(RuntimeException ex, HttpServletRequest req) {
        logger.error("Runtime exception", ex);
        ApiError body = build(req, HttpStatus.INTERNAL_SERVER_ERROR, "Error interno del servidor", "RUNTIME_EXCEPTION", null);
        return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).body(body);
    }

    @ExceptionHandler(Exception.class)
    public ResponseEntity<ApiError> handleGenericException(Exception ex, HttpServletRequest req) {
        logger.error("Unexpected exception", ex);
        ApiError body = build(req, HttpStatus.INTERNAL_SERVER_ERROR, "Ha ocurrido un error inesperado", "UNEXPECTED_ERROR", null);
        return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).body(body);
    }
}
