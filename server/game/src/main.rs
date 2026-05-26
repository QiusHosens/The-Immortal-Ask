use bevy_ecs::prelude::*;
use immortal_ask_common::AppConfig;
use tracing_subscriber::{EnvFilter, fmt};

#[derive(Component)]
struct GameSession;

fn setup_world(mut commands: Commands) {
    commands.spawn(GameSession);
}

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    dotenvy::dotenv().ok();

    fmt()
        .with_env_filter(EnvFilter::from_default_env())
        .init();

    let config = AppConfig::from_env()?;
    let _pool = sqlx::postgres::PgPoolOptions::new()
        .max_connections(10)
        .connect(&config.database_url)
        .await?;

    let mut world = World::new();
    let mut schedule = Schedule::default();
    schedule.add_systems(setup_world);
    schedule.run(&mut world);

    tracing::info!("game service started (ECS world initialized, database connected)");
    tokio::signal::ctrl_c().await?;
    Ok(())
}
