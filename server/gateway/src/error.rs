use axum::{Json, http::StatusCode};
use serde::Serialize;

#[derive(Serialize)]
pub struct ErrorBody {
    pub error: String,
    pub code: &'static str,
}

pub type ApiError = (StatusCode, Json<ErrorBody>);

pub fn bad_request(message: impl Into<String>) -> ApiError {
    (
        StatusCode::BAD_REQUEST,
        Json(ErrorBody {
            error: message.into(),
            code: "INVALID_ARGUMENT",
        }),
    )
}

pub fn unauthorized(message: impl Into<String>) -> ApiError {
    (
        StatusCode::UNAUTHORIZED,
        Json(ErrorBody {
            error: message.into(),
            code: "UNAUTHORIZED",
        }),
    )
}

pub fn forbidden(message: impl Into<String>) -> ApiError {
    (
        StatusCode::FORBIDDEN,
        Json(ErrorBody {
            error: message.into(),
            code: "FORBIDDEN",
        }),
    )
}

pub fn not_found(message: impl Into<String>) -> ApiError {
    (
        StatusCode::NOT_FOUND,
        Json(ErrorBody {
            error: message.into(),
            code: "NOT_FOUND",
        }),
    )
}

pub fn conflict(message: impl Into<String>) -> ApiError {
    (
        StatusCode::CONFLICT,
        Json(ErrorBody {
            error: message.into(),
            code: "CONFLICT",
        }),
    )
}

pub fn internal(message: impl Into<String>) -> ApiError {
    (
        StatusCode::INTERNAL_SERVER_ERROR,
        Json(ErrorBody {
            error: message.into(),
            code: "INTERNAL",
        }),
    )
}
