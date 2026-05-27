use std::sync::Arc;

use axum::{
    extract::FromRequestParts,
    http::{StatusCode, header::AUTHORIZATION, request::Parts},
};

use crate::{AppState, auth::jwt::verify_token, error::ErrorBody};

pub struct AuthUser {
    pub account_id: i64,
}

impl FromRequestParts<Arc<AppState>> for AuthUser {
    type Rejection = (StatusCode, axum::Json<ErrorBody>);

    fn from_request_parts(
        parts: &mut Parts,
        state: &Arc<AppState>,
    ) -> impl std::future::Future<Output = Result<Self, Self::Rejection>> + Send {
        let jwt_secret = state.jwt_secret.clone();
        let auth_header = parts
            .headers
            .get(AUTHORIZATION)
            .and_then(|value| value.to_str().ok())
            .map(str::to_owned);

        async move {
            let Some(header) = auth_header else {
                return Err((
                    StatusCode::UNAUTHORIZED,
                    axum::Json(ErrorBody {
                        error: "缺少 Authorization 头".into(),
                        code: "UNAUTHORIZED",
                    }),
                ));
            };

            let Some(token) = header.strip_prefix("Bearer ") else {
                return Err((
                    StatusCode::UNAUTHORIZED,
                    axum::Json(ErrorBody {
                        error: "Authorization 格式应为 Bearer <token>".into(),
                        code: "UNAUTHORIZED",
                    }),
                ));
            };

            let claims = verify_token(token, &jwt_secret).map_err(|_| {
                (
                    StatusCode::UNAUTHORIZED,
                    axum::Json(ErrorBody {
                        error: "令牌无效或已过期".into(),
                        code: "UNAUTHORIZED",
                    }),
                )
            })?;

            Ok(AuthUser {
                account_id: claims.sub,
            })
        }
    }
}
