use std::sync::Arc;

use axum::{
    Json, Router,
    extract::State,
    http::StatusCode,
    routing::{get, post},
};
use serde::{Deserialize, Serialize};
use sqlx::Row;

use crate::{AppState, auth as auth_util};

#[derive(Serialize)]
struct HealthResponse {
    status: &'static str,
    service: &'static str,
}

#[derive(Deserialize)]
struct RegisterRequest {
    username: String,
    password: String,
    email: Option<String>,
}

#[derive(Serialize)]
struct RegisterResponse {
    account_id: i64,
}

#[derive(Deserialize)]
struct LoginRequest {
    username: String,
    password: String,
}

#[derive(Serialize)]
struct LoginResponse {
    access_token: String,
    refresh_token: String,
    expires_in: u64,
    account_id: i64,
}

#[derive(Serialize)]
struct ErrorBody {
    error: String,
    code: &'static str,
}

type ApiError = (StatusCode, Json<ErrorBody>);

async fn health() -> Json<HealthResponse> {
    Json(HealthResponse {
        status: "ok",
        service: "gateway",
    })
}

async fn ready() -> Json<HealthResponse> {
    Json(HealthResponse {
        status: "ready",
        service: "gateway",
    })
}

fn validate_username(username: &str) -> Result<(), ApiError> {
    let len = username.chars().count();
    if !(3..=32).contains(&len) {
        return Err((
            StatusCode::BAD_REQUEST,
            Json(ErrorBody {
                error: "用户名长度需为 3-32 个字符".into(),
                code: "INVALID_ARGUMENT",
            }),
        ));
    }
    Ok(())
}

fn validate_password(password: &str) -> Result<(), ApiError> {
    if password.chars().count() < 6 {
        return Err((
            StatusCode::BAD_REQUEST,
            Json(ErrorBody {
                error: "密码长度至少 6 个字符".into(),
                code: "INVALID_ARGUMENT",
            }),
        ));
    }
    Ok(())
}

async fn register(
    State(state): State<Arc<AppState>>,
    Json(body): Json<RegisterRequest>,
) -> Result<Json<RegisterResponse>, ApiError> {
    validate_username(&body.username)?;
    validate_password(&body.password)?;

    let password_hash = auth_util::hash_password(&body.password).map_err(|_| {
        (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(ErrorBody {
                error: "密码处理失败".into(),
                code: "INTERNAL",
            }),
        )
    })?;

    let row = sqlx::query(
        r#"
        INSERT INTO accounts (username, email, password_hash)
        VALUES ($1, $2, $3)
        RETURNING id
        "#,
    )
    .bind(&body.username)
    .bind(body.email.as_deref())
    .bind(&password_hash)
    .fetch_one(&state.db)
    .await
    .map_err(|err| match err {
        sqlx::Error::Database(db_err) if db_err.code().as_deref() == Some("23505") => (
            StatusCode::CONFLICT,
            Json(ErrorBody {
                error: "用户名或邮箱已被注册".into(),
                code: "CONFLICT",
            }),
        ),
        _ => (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(ErrorBody {
                error: "注册失败，请稍后重试".into(),
                code: "INTERNAL",
            }),
        ),
    })?;

    let account_id: i64 = row.get("id");

    Ok(Json(RegisterResponse { account_id }))
}

async fn login(
    State(state): State<Arc<AppState>>,
    Json(body): Json<LoginRequest>,
) -> Result<Json<LoginResponse>, ApiError> {
    validate_username(&body.username)?;
    validate_password(&body.password)?;

    let row = sqlx::query(
        r#"
        SELECT id, password_hash, status
        FROM accounts
        WHERE username = $1
        "#,
    )
    .bind(&body.username)
    .fetch_optional(&state.db)
    .await
    .map_err(|_| {
        (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(ErrorBody {
                error: "登录失败，请稍后重试".into(),
                code: "INTERNAL",
            }),
        )
    })?;

    let Some(row) = row else {
        return Err((
            StatusCode::UNAUTHORIZED,
            Json(ErrorBody {
                error: "用户名或密码错误".into(),
                code: "UNAUTHORIZED",
            }),
        ));
    };

    let account_id: i64 = row.get("id");
    let password_hash: String = row.get("password_hash");
    let status: i16 = row.get("status");

    if status != 1 {
        return Err((
            StatusCode::FORBIDDEN,
            Json(ErrorBody {
                error: "账号已被禁用".into(),
                code: "FORBIDDEN",
            }),
        ));
    }

    let valid = auth_util::verify_password(&body.password, &password_hash).map_err(|_| {
        (
            StatusCode::INTERNAL_SERVER_ERROR,
            Json(ErrorBody {
                error: "登录失败，请稍后重试".into(),
                code: "INTERNAL",
            }),
        )
    })?;

    if !valid {
        return Err((
            StatusCode::UNAUTHORIZED,
            Json(ErrorBody {
                error: "用户名或密码错误".into(),
                code: "UNAUTHORIZED",
            }),
        ));
    }

    let _ = sqlx::query("UPDATE accounts SET last_login_at = now() WHERE id = $1")
        .bind(account_id)
        .execute(&state.db)
        .await;

    const EXPIRES_IN: u64 = 86_400;
    let access_token = auth_util::issue_token(account_id, &state.jwt_secret, EXPIRES_IN).map_err(
        |_| {
            (
                StatusCode::INTERNAL_SERVER_ERROR,
                Json(ErrorBody {
                    error: "令牌签发失败".into(),
                    code: "INTERNAL",
                }),
            )
        },
    )?;

    Ok(Json(LoginResponse {
        access_token: access_token.clone(),
        refresh_token: access_token,
        expires_in: EXPIRES_IN,
        account_id,
    }))
}

pub fn router() -> Router<Arc<AppState>> {
    Router::new()
        .route("/health", get(health))
        .route("/ready", get(ready))
        .route("/api/v1/auth/register", post(register))
        .route("/api/v1/auth/login", post(login))
}
