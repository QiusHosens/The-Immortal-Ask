use std::sync::Arc;

use axum::Router;
use sqlx::PgPool;
use tower_http::cors::{Any, CorsLayer};

mod auth;
mod characters;
mod error;
mod routes;

pub struct AppState {
    pub db: PgPool,
    pub jwt_secret: String,
}

pub fn build_router(state: Arc<AppState>) -> Router {
    let cors = CorsLayer::new()
        .allow_origin(Any)
        .allow_methods(Any)
        .allow_headers(Any);

    routes::router()
        .merge(characters::router())
        .layer(cors)
        .with_state(state)
}
