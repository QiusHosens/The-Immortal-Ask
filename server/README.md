# 后端服务 (Rust Workspace)

## 模块

| Crate | 说明 | 启动 |
|-------|------|------|
| `gateway` | Axum HTTP 网关 | `cargo run -p immortal-ask-gateway` |
| `auth` | 认证服务 | `cargo run -p immortal-ask-auth` |
| `game` | 游戏逻辑（Bevy ECS） | `cargo run -p immortal-ask-game` |
| `common` | 公共库 | — |

## 本地开发

```bash
cp ../.env.example ../.env
cargo run -p immortal-ask-gateway
# 健康检查
curl http://localhost:8080/health
```

## 协议

Protobuf 定义位于 `proto/`，后续通过 Volo CLI 生成 RPC 代码。
