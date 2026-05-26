# 工具脚本

| 脚本 | 说明 |
|------|------|
| `migrate.sh` | 执行 PostgreSQL 初始 migration |

```bash
# Linux / macOS / Git Bash
./tools/migrate.sh

# Windows PowerShell
$env:DATABASE_URL = "postgres://immortal:immortal@localhost:5432/immortal_ask"
psql $env:DATABASE_URL -f deploy/database/migrations/001_initial_schema.sql
```
