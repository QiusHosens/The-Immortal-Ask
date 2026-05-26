-- 宿命劫：问仙 — 创建数据库与用户（纯 SQL）
-- PostgreSQL 18.x
--
-- 【执行方式】
-- 1. 在客户端中先连接到 postgres 系统库（非 immortal_ask）
-- 2. 以 postgres 超级用户执行本脚本
-- 3. 若角色/库已存在，对应语句会报错，可忽略后继续
-- 4. 完成后连接 immortal_ask，再执行 001_initial_schema.sql
--
-- Docker Compose 已通过环境变量创建用户与库，无需执行本脚本。

-- 创建应用角色
DO
$do$
BEGIN
    IF NOT EXISTS (SELECT FROM pg_catalog.pg_roles WHERE rolname = 'immortal') THEN
        CREATE ROLE immortal LOGIN PASSWORD 'immortal';
    END IF;
END
$do$;

-- 创建业务数据库（不可放在 DO/事务块内；已存在时会报错，可忽略）
CREATE DATABASE immortal_ask OWNER immortal ENCODING 'UTF8';
-- 授权（需连接 postgres 库执行）
GRANT ALL PRIVILEGES ON DATABASE immortal_ask TO immortal;

