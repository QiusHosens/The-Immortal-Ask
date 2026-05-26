# 客户端说明

UE5.7 工程，含 **账号登录 / 注册** 界面（C++ UMG，纯代码构建，无需 Widget Blueprint 资产）。

## 目录

| 目录 | 说明 |
|------|------|
| `Source/TheImmortalAsk/` | C++ 游戏模块 |
| `Source/TheImmortalAsk/Public/UI/` | 登录 / 注册界面 |
| `Source/TheImmortalAsk/Public/Auth/` | HTTP 认证客户端 |
| `Config/` | 网关地址等配置 |
| `Content/` | 资产、蓝图、关卡 |
| `Plugins/` | GAS 扩展、UnLua / Puerts 等插件 |
| `Scripts/` | 脚本层（UI、任务、配置驱动逻辑） |

## 首次打开

1. 用 **Unreal Engine 5.7** 打开 `TheImmortalAsk.uproject`
2. 提示生成 Visual Studio 工程时选择 **Yes**
3. 编译 `TheImmortalAsk` 模块（Development Editor）
4. Play 后会自动显示登录界面

## 联调后端

```bash
# 终端 1：数据依赖
docker compose -f deploy/docker/docker-compose.yml up -d

# 终端 2：网关
cd server && cargo run -p immortal-ask-gateway
```

网关默认地址 `http://127.0.0.1:8080`，可在 `Config/DefaultGame.ini` 修改：

```ini
[/Script/TheImmortalAsk.AuthSettings]
GatewayBaseUrl=http://127.0.0.1:8080
```

## 界面说明

- **登录页**：道号 + 口令 → `POST /api/v1/auth/login`
- **注册页**：道号 + 邮箱（可选）+ 口令 → `POST /api/v1/auth/register`
- 登录成功后 Token 存入 `UAuthSession`（GameInstance 子系统）

## 插件

- **GAS**：战斗与技能系统（已启用）
- **UnLua / Puerts**：后续可用于 UI 热更新，当前登录页为 C++ 实现
