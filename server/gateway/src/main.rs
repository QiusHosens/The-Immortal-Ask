use std::sync::Arc;

use immortal_ask_common::AppConfig;
use immortal_ask_gateway::{AppState, build_router};
use tokio::net::TcpListener;
use tracing_subscriber::{EnvFilter, fmt};

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    dotenvy::dotenv().ok();

    fmt()
        .with_env_filter(EnvFilter::from_default_env())
        .init();

    let config = AppConfig::from_env()?;
    let jwt_secret = std::env::var("JWT_SECRET")
        .unwrap_or_else(|_| "dev-only-change-me-in-production".into());

    let db = sqlx::postgres::PgPoolOptions::new()
        .max_connections(10)
        .connect(&config.database_url)
        .await?;

    let state = Arc::new(AppState { db, jwt_secret });
    let app = build_router(state);

    let addr = format!("{}:{}", config.gateway_host, config.gateway_port);
    tracing::info!("gateway listening on {addr}");

    let listener = TcpListener::bind(&addr).await?;
    axum::serve(listener, app).await?;

    Ok(())
}
