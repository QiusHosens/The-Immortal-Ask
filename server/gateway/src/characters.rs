use std::sync::Arc;

use axum::{
    Json, Router,
    extract::{Path, Query, State},
    routing::{get, post},
};
use serde::{Deserialize, Serialize};
use sqlx::Row;
use uuid::Uuid;

use crate::{
    AppState,
    auth::AuthUser,
    error::{ApiError, bad_request, conflict, internal, not_found},
};

#[derive(Serialize)]
struct RealmItem {
    id: i16,
    code: String,
    name: String,
    max_characters: i16,
}

#[derive(Serialize)]
struct RealmsResponse {
    realms: Vec<RealmItem>,
}

#[derive(Serialize)]
struct SectOption {
    id: i16,
    name: String,
    element: Option<String>,
}

#[derive(Serialize)]
struct SpiritRootOption {
    id: i16,
    name: String,
    rarity: i16,
}

#[derive(Serialize)]
struct CreationOptionsResponse {
    sects: Vec<SectOption>,
    spirit_roots: Vec<SpiritRootOption>,
}

#[derive(Deserialize)]
struct ListCharactersQuery {
    realm_id: i16,
}

#[derive(Serialize)]
struct CharacterSummary {
    public_id: Uuid,
    name: String,
    level: i16,
    combat_power: i32,
    realm_stage_name: Option<String>,
    sect_name: Option<String>,
}

#[derive(Serialize)]
struct ListCharactersResponse {
    characters: Vec<CharacterSummary>,
}

#[derive(Deserialize)]
struct CreateCharacterRequest {
    realm_id: i16,
    name: String,
    gender: i16,
    sect_id: i16,
    spirit_root_id: i16,
}

#[derive(Serialize)]
struct CreateCharacterResponse {
    public_id: Uuid,
    name: String,
}

#[derive(Serialize)]
struct EnterWorldResponse {
    public_id: Uuid,
    name: String,
    level: i16,
    map_id: i32,
    pos_x: f32,
    pos_y: f32,
    pos_z: f32,
    rotation_yaw: f32,
    realm_stage_name: Option<String>,
    sect_name: Option<String>,
}

fn validate_character_name(name: &str) -> Result<(), ApiError> {
    let len = name.chars().count();
    if !(2..=16).contains(&len) {
        return Err(bad_request("道号长度需为 2-16 个字符"));
    }
    Ok(())
}

fn validate_gender(gender: i16) -> Result<(), ApiError> {
    if !(1..=3).contains(&gender) {
        return Err(bad_request("性别参数无效"));
    }
    Ok(())
}

async fn list_realms(State(state): State<Arc<AppState>>) -> Result<Json<RealmsResponse>, ApiError> {
    let rows = sqlx::query(
        r#"
        SELECT id, code, name, max_characters
        FROM realms
        WHERE is_open = TRUE
        ORDER BY id
        "#,
    )
    .fetch_all(&state.db)
    .await
    .map_err(|_| internal("获取区服列表失败"))?;

    let realms = rows
        .into_iter()
        .map(|row| RealmItem {
            id: row.get("id"),
            code: row.get("code"),
            name: row.get("name"),
            max_characters: row.get("max_characters"),
        })
        .collect();

    Ok(Json(RealmsResponse { realms }))
}

async fn creation_options(
    State(state): State<Arc<AppState>>,
) -> Result<Json<CreationOptionsResponse>, ApiError> {
    let sect_rows = sqlx::query("SELECT id, name, element FROM sects ORDER BY id")
        .fetch_all(&state.db)
        .await
        .map_err(|_| internal("获取门派列表失败"))?;

    let root_rows = sqlx::query("SELECT id, name, rarity FROM spirit_roots ORDER BY id")
        .fetch_all(&state.db)
        .await
        .map_err(|_| internal("获取灵根列表失败"))?;

    Ok(Json(CreationOptionsResponse {
        sects: sect_rows
            .into_iter()
            .map(|row| SectOption {
                id: row.get("id"),
                name: row.get("name"),
                element: row.get("element"),
            })
            .collect(),
        spirit_roots: root_rows
            .into_iter()
            .map(|row| SpiritRootOption {
                id: row.get("id"),
                name: row.get("name"),
                rarity: row.get("rarity"),
            })
            .collect(),
    }))
}

async fn list_characters(
    AuthUser { account_id }: AuthUser,
    State(state): State<Arc<AppState>>,
    Query(query): Query<ListCharactersQuery>,
) -> Result<Json<ListCharactersResponse>, ApiError> {
    let rows = sqlx::query(
        r#"
        SELECT c.public_id, c.name, c.level, c.combat_power,
               rs.name AS realm_stage_name, s.name AS sect_name
        FROM characters c
        LEFT JOIN realm_stages rs ON rs.id = c.realm_stage_id
        LEFT JOIN sects s ON s.id = c.sect_id
        WHERE c.account_id = $1
          AND c.realm_id = $2
          AND c.deleted_at IS NULL
        ORDER BY c.created_at
        "#,
    )
    .bind(account_id)
    .bind(query.realm_id)
    .fetch_all(&state.db)
    .await
    .map_err(|_| internal("获取角色列表失败"))?;

    let characters = rows
        .into_iter()
        .map(|row| CharacterSummary {
            public_id: row.get("public_id"),
            name: row.get("name"),
            level: row.get("level"),
            combat_power: row.get("combat_power"),
            realm_stage_name: row.get("realm_stage_name"),
            sect_name: row.get("sect_name"),
        })
        .collect();

    Ok(Json(ListCharactersResponse { characters }))
}

async fn create_character(
    AuthUser { account_id }: AuthUser,
    State(state): State<Arc<AppState>>,
    Json(body): Json<CreateCharacterRequest>,
) -> Result<Json<CreateCharacterResponse>, ApiError> {
    validate_character_name(&body.name)?;
    validate_gender(body.gender)?;

    let mut tx = state.db.begin().await.map_err(|_| internal("创建角色失败"))?;

    let realm_row = sqlx::query(
        "SELECT max_characters FROM realms WHERE id = $1 AND is_open = TRUE",
    )
    .bind(body.realm_id)
    .fetch_optional(&mut *tx)
    .await
    .map_err(|_| internal("创建角色失败"))?;

    let Some(realm_row) = realm_row else {
        return Err(bad_request("区服不存在或未开放"));
    };

    let max_characters: i16 = realm_row.get("max_characters");

    let count_row = sqlx::query(
        r#"
        SELECT COUNT(*) AS count
        FROM characters
        WHERE account_id = $1 AND realm_id = $2 AND deleted_at IS NULL
        "#,
    )
    .bind(account_id)
    .bind(body.realm_id)
    .fetch_one(&mut *tx)
    .await
    .map_err(|_| internal("创建角色失败"))?;

    let count: i64 = count_row.get("count");
    if count >= i64::from(max_characters) {
        return Err(conflict("该区服角色数量已达上限"));
    }

    let sect_exists = sqlx::query("SELECT 1 FROM sects WHERE id = $1")
        .bind(body.sect_id)
        .fetch_optional(&mut *tx)
        .await
        .map_err(|_| internal("创建角色失败"))?;
    if sect_exists.is_none() {
        return Err(bad_request("门派不存在"));
    }

    let root_exists = sqlx::query("SELECT 1 FROM spirit_roots WHERE id = $1")
        .bind(body.spirit_root_id)
        .fetch_optional(&mut *tx)
        .await
        .map_err(|_| internal("创建角色失败"))?;
    if root_exists.is_none() {
        return Err(bad_request("灵根不存在"));
    }

    let row = sqlx::query(
        r#"
        INSERT INTO characters (
            account_id, realm_id, name, gender, sect_id, spirit_root_id
        )
        VALUES ($1, $2, $3, $4, $5, $6)
        RETURNING id, public_id, name
        "#,
    )
    .bind(account_id)
    .bind(body.realm_id)
    .bind(&body.name)
    .bind(body.gender)
    .bind(body.sect_id)
    .bind(body.spirit_root_id)
    .fetch_one(&mut *tx)
    .await
    .map_err(|err| match err {
        sqlx::Error::Database(db_err) if db_err.code().as_deref() == Some("23505") => {
            conflict("该道号已被占用")
        }
        _ => internal("创建角色失败"),
    })?;

    let character_id: i64 = row.get("id");
    let public_id: Uuid = row.get("public_id");
    let name: String = row.get("name");

    sqlx::query("INSERT INTO character_stats (character_id) VALUES ($1)")
        .bind(character_id)
        .execute(&mut *tx)
        .await
        .map_err(|_| internal("创建角色失败"))?;

    tx.commit().await.map_err(|_| internal("创建角色失败"))?;

    Ok(Json(CreateCharacterResponse { public_id, name }))
}

async fn enter_world(
    AuthUser { account_id }: AuthUser,
    State(state): State<Arc<AppState>>,
    Path(public_id): Path<Uuid>,
) -> Result<Json<EnterWorldResponse>, ApiError> {
    let row = sqlx::query(
        r#"
        WITH updated AS (
            UPDATE characters
            SET last_online_at = now(), updated_at = now()
            WHERE public_id = $1
              AND account_id = $2
              AND deleted_at IS NULL
            RETURNING public_id, name, level, map_id, pos_x, pos_y, pos_z, rotation_yaw,
                      realm_stage_id, sect_id
        )
        SELECT u.public_id, u.name, u.level, u.map_id, u.pos_x, u.pos_y, u.pos_z, u.rotation_yaw,
               rs.name AS realm_stage_name, s.name AS sect_name
        FROM updated u
        LEFT JOIN realm_stages rs ON rs.id = u.realm_stage_id
        LEFT JOIN sects s ON s.id = u.sect_id
        "#,
    )
    .bind(public_id)
    .bind(account_id)
    .fetch_optional(&state.db)
    .await
    .map_err(|_| internal("进入世界失败"))?;

    let Some(row) = row else {
        return Err(not_found("角色不存在"));
    };

    Ok(Json(EnterWorldResponse {
        public_id: row.get("public_id"),
        name: row.get("name"),
        level: row.get("level"),
        map_id: row.get("map_id"),
        pos_x: row.get("pos_x"),
        pos_y: row.get("pos_y"),
        pos_z: row.get("pos_z"),
        rotation_yaw: row.get("rotation_yaw"),
        realm_stage_name: row.get("realm_stage_name"),
        sect_name: row.get("sect_name"),
    }))
}

pub fn router() -> Router<Arc<AppState>> {
    Router::new()
        .route("/api/v1/realms", get(list_realms))
        .route("/api/v1/creation-options", get(creation_options))
        .route("/api/v1/characters", get(list_characters).post(create_character))
        .route(
            "/api/v1/characters/{public_id}/enter",
            post(enter_world),
        )
}
