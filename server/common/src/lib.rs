//! 宿命劫：问仙 — 后端公共库

pub mod config;
pub mod error;

pub use config::AppConfig;
pub use error::{AppError, AppResult};
