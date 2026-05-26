-- 宿命劫：问仙 — 初始数据库 Schema（纯 SQL，兼容 Navicat / DBeaver / DataGrip）
-- PostgreSQL 18.x
--
-- 【执行前】请先连接到 immortal_ask 数据库
-- 【建库建用户】见同目录 000_create_database.sql（Docker Compose 可跳过）
--
-- 命令行示例:
--   psql -U immortal -d immortal_ask -f deploy/database/migrations/001_initial_schema.sql
--
-- ---------------------------------------------------------------------------
-- SMALLINT 枚举约定（应用层常量，非 PG ENUM）
-- ---------------------------------------------------------------------------
-- account_status      : 1=active  2=banned  3=pending
-- character_gender    : 1=male    2=female  3=unknown
-- guild_role          : 1=leader  2=elder   3=member
-- quest_status        : 1=locked  2=active  3=completed  4=failed  5=abandoned
-- mail_status         : 1=unread  2=read    3=claimed    4=deleted
-- item_bind_type      : 1=none    2=pickup  3=equip
-- friendship_status   : 1=pending 2=accepted 3=blocked
-- ---------------------------------------------------------------------------

BEGIN;

GRANT ALL ON SCHEMA public TO immortal;
ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON TABLES TO immortal;
ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON SEQUENCES TO immortal;

CREATE EXTENSION IF NOT EXISTS pgcrypto;
CREATE EXTENSION IF NOT EXISTS citext;

COMMENT ON EXTENSION pgcrypto IS '提供 gen_random_uuid() 等加密函数';
COMMENT ON EXTENSION citext IS '大小写不敏感文本类型，用于用户名/邮箱';

-- ---------------------------------------------------------------------------
-- 区服 & 静态配置
-- ---------------------------------------------------------------------------

CREATE TABLE realms (
    id              SMALLINT PRIMARY KEY GENERATED ALWAYS AS IDENTITY,
    code            VARCHAR(16) NOT NULL UNIQUE,
    name            VARCHAR(64) NOT NULL,
    region          VARCHAR(32) NOT NULL DEFAULT 'cn',
    max_characters  SMALLINT NOT NULL DEFAULT 3 CHECK (max_characters BETWEEN 1 AND 10),
    is_open         BOOLEAN NOT NULL DEFAULT TRUE,
    opened_at       TIMESTAMPTZ,
    created_at      TIMESTAMPTZ NOT NULL DEFAULT now()
);

COMMENT ON TABLE realms IS '游戏区服';
COMMENT ON COLUMN realms.id IS '区服主键 ID';
COMMENT ON COLUMN realms.code IS '区服唯一编码，如 s1';
COMMENT ON COLUMN realms.name IS '区服显示名称';
COMMENT ON COLUMN realms.region IS '所属区域/发行区，如 cn';
COMMENT ON COLUMN realms.max_characters IS '单账号在该区服可创建的最大角色数';
COMMENT ON COLUMN realms.is_open IS '是否开放注册/登录';
COMMENT ON COLUMN realms.opened_at IS '开服时间';
COMMENT ON COLUMN realms.created_at IS '记录创建时间';

CREATE TABLE realm_stages (
    id              SMALLINT PRIMARY KEY,
    name            VARCHAR(32) NOT NULL,
    major_realm     VARCHAR(16) NOT NULL,
    minor_stage     SMALLINT NOT NULL,
    required_exp    BIGINT NOT NULL DEFAULT 0,
    max_level       SMALLINT NOT NULL DEFAULT 10
);

COMMENT ON TABLE realm_stages IS '修炼境界配置';
COMMENT ON COLUMN realm_stages.id IS '境界阶段 ID';
COMMENT ON COLUMN realm_stages.name IS '境界名称，如炼气一层';
COMMENT ON COLUMN realm_stages.major_realm IS '大境界：炼气、筑基、金丹等';
COMMENT ON COLUMN realm_stages.minor_stage IS '小阶段序号：初期、中期、后期等';
COMMENT ON COLUMN realm_stages.required_exp IS '突破至该境界所需累计经验';
COMMENT ON COLUMN realm_stages.max_level IS '该境界内允许的最高角色等级';

CREATE TABLE sects (
    id              SMALLINT PRIMARY KEY,
    code            VARCHAR(32) NOT NULL UNIQUE,
    name            VARCHAR(64) NOT NULL,
    element         VARCHAR(16),
    description     TEXT
);

COMMENT ON TABLE sects IS '门派/宗派配置';
COMMENT ON COLUMN sects.id IS '门派 ID';
COMMENT ON COLUMN sects.code IS '门派唯一编码';
COMMENT ON COLUMN sects.name IS '门派显示名称';
COMMENT ON COLUMN sects.element IS '门派主属性/五行属性';
COMMENT ON COLUMN sects.description IS '门派背景描述';

CREATE TABLE spirit_roots (
    id              SMALLINT PRIMARY KEY,
    name            VARCHAR(32) NOT NULL,
    elements        JSONB NOT NULL DEFAULT '[]',
    rarity          SMALLINT NOT NULL DEFAULT 1 CHECK (rarity BETWEEN 1 AND 5)
);

COMMENT ON TABLE spirit_roots IS '灵根配置';
COMMENT ON COLUMN spirit_roots.id IS '灵根 ID';
COMMENT ON COLUMN spirit_roots.name IS '灵根名称';
COMMENT ON COLUMN spirit_roots.elements IS '灵根元素列表 JSON，如 ["fire","wood"]';
COMMENT ON COLUMN spirit_roots.rarity IS '稀有度 1-5，数值越大越稀有';

CREATE TABLE item_templates (
    id              INTEGER PRIMARY KEY,
    code            VARCHAR(64) NOT NULL UNIQUE,
    name            VARCHAR(64) NOT NULL,
    category        VARCHAR(32) NOT NULL,
    quality         SMALLINT NOT NULL DEFAULT 1 CHECK (quality BETWEEN 1 AND 7),
    max_stack       INTEGER NOT NULL DEFAULT 1 CHECK (max_stack >= 1),
    bind_type       SMALLINT NOT NULL DEFAULT 1 CHECK (bind_type BETWEEN 1 AND 3),
    sell_price      BIGINT NOT NULL DEFAULT 0,
    meta            JSONB NOT NULL DEFAULT '{}',
    created_at      TIMESTAMPTZ NOT NULL DEFAULT now()
);

COMMENT ON TABLE item_templates IS '物品模板（配置表）';
COMMENT ON COLUMN item_templates.id IS '物品模板 ID';
COMMENT ON COLUMN item_templates.code IS '物品唯一编码';
COMMENT ON COLUMN item_templates.name IS '物品显示名称';
COMMENT ON COLUMN item_templates.category IS '物品分类：material/consumable/equipment/quest 等';
COMMENT ON COLUMN item_templates.quality IS '品质等级 1-7';
COMMENT ON COLUMN item_templates.max_stack IS '最大堆叠数量';
COMMENT ON COLUMN item_templates.bind_type IS '绑定类型：1=none 2=pickup 3=equip';
COMMENT ON COLUMN item_templates.sell_price IS '出售价（基准货币单位）';
COMMENT ON COLUMN item_templates.meta IS '扩展配置 JSON';
COMMENT ON COLUMN item_templates.created_at IS '模板创建时间';

CREATE TABLE skill_templates (
    id              INTEGER PRIMARY KEY,
    code            VARCHAR(64) NOT NULL UNIQUE,
    name            VARCHAR(64) NOT NULL,
    skill_type      VARCHAR(32) NOT NULL,
    element         VARCHAR(16),
    cooldown_ms     INTEGER NOT NULL DEFAULT 0,
    gas_tag         VARCHAR(64),
    meta            JSONB NOT NULL DEFAULT '{}'
);

COMMENT ON TABLE skill_templates IS '技能模板（与 GAS 映射）';
COMMENT ON COLUMN skill_templates.id IS '技能模板 ID';
COMMENT ON COLUMN skill_templates.code IS '技能唯一编码';
COMMENT ON COLUMN skill_templates.name IS '技能显示名称';
COMMENT ON COLUMN skill_templates.skill_type IS '技能类型：active/passive/ultimate 等';
COMMENT ON COLUMN skill_templates.element IS '技能元素属性';
COMMENT ON COLUMN skill_templates.cooldown_ms IS '冷却时间（毫秒）';
COMMENT ON COLUMN skill_templates.gas_tag IS '对应 UE GAS Ability 标识';
COMMENT ON COLUMN skill_templates.meta IS '扩展配置 JSON';

CREATE TABLE quest_templates (
    id              INTEGER PRIMARY KEY,
    code            VARCHAR(64) NOT NULL UNIQUE,
    name            VARCHAR(128) NOT NULL,
    quest_type      VARCHAR(32) NOT NULL,
    min_level       SMALLINT NOT NULL DEFAULT 1,
    prerequisites   JSONB NOT NULL DEFAULT '[]',
    rewards         JSONB NOT NULL DEFAULT '{}',
    meta            JSONB NOT NULL DEFAULT '{}'
);

COMMENT ON TABLE quest_templates IS '任务模板';
COMMENT ON COLUMN quest_templates.id IS '任务模板 ID';
COMMENT ON COLUMN quest_templates.code IS '任务唯一编码';
COMMENT ON COLUMN quest_templates.name IS '任务显示名称';
COMMENT ON COLUMN quest_templates.quest_type IS '任务类型：main/side/daily/sect 等';
COMMENT ON COLUMN quest_templates.min_level IS '接取最低角色等级';
COMMENT ON COLUMN quest_templates.prerequisites IS '前置条件 JSON 数组';
COMMENT ON COLUMN quest_templates.rewards IS '奖励配置 JSON';
COMMENT ON COLUMN quest_templates.meta IS '扩展配置 JSON';

-- ---------------------------------------------------------------------------
-- 账号
-- ---------------------------------------------------------------------------

CREATE TABLE accounts (
    id              BIGINT PRIMARY KEY GENERATED ALWAYS AS IDENTITY,
    username        CITEXT NOT NULL UNIQUE,
    email           CITEXT UNIQUE,
    password_hash   TEXT NOT NULL,
    status          SMALLINT NOT NULL DEFAULT 1 CHECK (status BETWEEN 1 AND 3),
    last_login_at   TIMESTAMPTZ,
    last_login_ip   INET,
    created_at      TIMESTAMPTZ NOT NULL DEFAULT now(),
    updated_at      TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX idx_accounts_status ON accounts (status);

COMMENT ON TABLE accounts IS '平台账号';
COMMENT ON COLUMN accounts.id IS '账号主键 ID';
COMMENT ON COLUMN accounts.username IS '登录用户名（大小写不敏感）';
COMMENT ON COLUMN accounts.email IS '绑定邮箱（可选，大小写不敏感）';
COMMENT ON COLUMN accounts.password_hash IS '密码哈希（Argon2 等）';
COMMENT ON COLUMN accounts.status IS '账号状态：1=active 2=banned 3=pending';
COMMENT ON COLUMN accounts.last_login_at IS '最近一次登录时间';
COMMENT ON COLUMN accounts.last_login_ip IS '最近一次登录 IP';
COMMENT ON COLUMN accounts.created_at IS '账号创建时间';
COMMENT ON COLUMN accounts.updated_at IS '账号最后更新时间';

-- ---------------------------------------------------------------------------
-- 角色
-- ---------------------------------------------------------------------------

CREATE TABLE characters (
    id              BIGINT PRIMARY KEY GENERATED ALWAYS AS IDENTITY,
    public_id       UUID NOT NULL DEFAULT gen_random_uuid() UNIQUE,
    account_id      BIGINT NOT NULL REFERENCES accounts (id),
    realm_id        SMALLINT NOT NULL REFERENCES realms (id),
    name            VARCHAR(32) NOT NULL,
    gender          SMALLINT NOT NULL DEFAULT 3 CHECK (gender BETWEEN 1 AND 3),
    sect_id         SMALLINT REFERENCES sects (id),
    spirit_root_id  SMALLINT REFERENCES spirit_roots (id),
    realm_stage_id  SMALLINT NOT NULL DEFAULT 1 REFERENCES realm_stages (id),
    level           SMALLINT NOT NULL DEFAULT 1 CHECK (level >= 1),
    exp             BIGINT NOT NULL DEFAULT 0 CHECK (exp >= 0),
    combat_power    INTEGER NOT NULL DEFAULT 0,
    map_id          INTEGER NOT NULL DEFAULT 1001,
    pos_x           REAL NOT NULL DEFAULT 0,
    pos_y           REAL NOT NULL DEFAULT 0,
    pos_z           REAL NOT NULL DEFAULT 0,
    rotation_yaw    REAL NOT NULL DEFAULT 0,
    attrs           JSONB NOT NULL DEFAULT '{}',
    last_online_at  TIMESTAMPTZ,
    created_at      TIMESTAMPTZ NOT NULL DEFAULT now(),
    updated_at      TIMESTAMPTZ NOT NULL DEFAULT now(),
    deleted_at      TIMESTAMPTZ,
    UNIQUE (realm_id, name)
);

CREATE INDEX idx_characters_account ON characters (account_id) WHERE deleted_at IS NULL;
CREATE INDEX idx_characters_realm ON characters (realm_id) WHERE deleted_at IS NULL;

COMMENT ON TABLE characters IS '玩家角色';
COMMENT ON COLUMN characters.id IS '角色主键 ID（内部）';
COMMENT ON COLUMN characters.public_id IS '对外公开 UUID，避免暴露自增 ID';
COMMENT ON COLUMN characters.account_id IS '所属平台账号 ID';
COMMENT ON COLUMN characters.realm_id IS '所属区服 ID';
COMMENT ON COLUMN characters.name IS '角色名（区服内唯一）';
COMMENT ON COLUMN characters.gender IS '性别：1=male 2=female 3=unknown';
COMMENT ON COLUMN characters.sect_id IS '所属门派 ID';
COMMENT ON COLUMN characters.spirit_root_id IS '灵根 ID';
COMMENT ON COLUMN characters.realm_stage_id IS '当前修炼境界阶段 ID';
COMMENT ON COLUMN characters.level IS '角色等级';
COMMENT ON COLUMN characters.exp IS '当前累计经验值';
COMMENT ON COLUMN characters.combat_power IS '战力评分（展示/排行用）';
COMMENT ON COLUMN characters.map_id IS '上次所在地图 ID';
COMMENT ON COLUMN characters.pos_x IS '上次下线 X 坐标';
COMMENT ON COLUMN characters.pos_y IS '上次下线 Y 坐标';
COMMENT ON COLUMN characters.pos_z IS '上次下线 Z 坐标';
COMMENT ON COLUMN characters.rotation_yaw IS '上次下线朝向（Yaw）';
COMMENT ON COLUMN characters.attrs IS '扩展属性 JSON：根骨、悟性、幸运等';
COMMENT ON COLUMN characters.last_online_at IS '最近上线时间';
COMMENT ON COLUMN characters.created_at IS '角色创建时间';
COMMENT ON COLUMN characters.updated_at IS '角色最后更新时间';
COMMENT ON COLUMN characters.deleted_at IS '软删除时间，NULL 表示未删除';

CREATE TABLE character_stats (
    character_id    BIGINT PRIMARY KEY REFERENCES characters (id) ON DELETE CASCADE,
    hp              INTEGER NOT NULL DEFAULT 100,
    mp              INTEGER NOT NULL DEFAULT 100,
    atk             INTEGER NOT NULL DEFAULT 10,
    def             INTEGER NOT NULL DEFAULT 5,
    crit_rate       REAL NOT NULL DEFAULT 0.05,
    crit_damage     REAL NOT NULL DEFAULT 1.5,
    move_speed      REAL NOT NULL DEFAULT 600,
    updated_at      TIMESTAMPTZ NOT NULL DEFAULT now()
);

COMMENT ON TABLE character_stats IS '角色战斗属性（持久化基准值，Buff 由 Redis 热数据维护）';
COMMENT ON COLUMN character_stats.character_id IS '关联角色 ID';
COMMENT ON COLUMN character_stats.hp IS '生命值上限基准';
COMMENT ON COLUMN character_stats.mp IS '法力值上限基准';
COMMENT ON COLUMN character_stats.atk IS '攻击力基准';
COMMENT ON COLUMN character_stats.def IS '防御力基准';
COMMENT ON COLUMN character_stats.crit_rate IS '暴击率 0-1';
COMMENT ON COLUMN character_stats.crit_damage IS '暴击伤害倍率';
COMMENT ON COLUMN character_stats.move_speed IS '移动速度';
COMMENT ON COLUMN character_stats.updated_at IS '属性最后更新时间';

-- ---------------------------------------------------------------------------
-- 背包 & 装备
-- ---------------------------------------------------------------------------

CREATE TABLE character_items (
    id              BIGINT PRIMARY KEY GENERATED ALWAYS AS IDENTITY,
    character_id    BIGINT NOT NULL REFERENCES characters (id) ON DELETE CASCADE,
    template_id     INTEGER NOT NULL REFERENCES item_templates (id),
    quantity        INTEGER NOT NULL DEFAULT 1 CHECK (quantity >= 1),
    slot_index      SMALLINT,
    bind_state      SMALLINT NOT NULL DEFAULT 1 CHECK (bind_state BETWEEN 1 AND 3),
    durability      SMALLINT,
    affixes         JSONB NOT NULL DEFAULT '{}',
    created_at      TIMESTAMPTZ NOT NULL DEFAULT now(),
    updated_at      TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX idx_character_items_char ON character_items (character_id);
CREATE UNIQUE INDEX idx_character_items_slot ON character_items (character_id, slot_index)
    WHERE slot_index IS NOT NULL;

COMMENT ON TABLE character_items IS '角色背包物品';
COMMENT ON COLUMN character_items.id IS '物品实例 ID';
COMMENT ON COLUMN character_items.character_id IS '所属角色 ID';
COMMENT ON COLUMN character_items.template_id IS '物品模板 ID';
COMMENT ON COLUMN character_items.quantity IS '堆叠数量';
COMMENT ON COLUMN character_items.slot_index IS '背包格位索引，NULL 表示溢出或待领取';
COMMENT ON COLUMN character_items.bind_state IS '绑定状态：1=none 2=pickup 3=equip';
COMMENT ON COLUMN character_items.durability IS '耐久度（可选）';
COMMENT ON COLUMN character_items.affixes IS '词缀/随机属性 JSON';
COMMENT ON COLUMN character_items.created_at IS '获得时间';
COMMENT ON COLUMN character_items.updated_at IS '最后更新时间';

CREATE TABLE character_equipment (
    character_id    BIGINT NOT NULL REFERENCES characters (id) ON DELETE CASCADE,
    slot            VARCHAR(16) NOT NULL,
    item_id         BIGINT NOT NULL REFERENCES character_items (id) ON DELETE CASCADE,
    equipped_at     TIMESTAMPTZ NOT NULL DEFAULT now(),
    PRIMARY KEY (character_id, slot),
    UNIQUE (item_id)
);

COMMENT ON TABLE character_equipment IS '已穿戴装备';
COMMENT ON COLUMN character_equipment.character_id IS '角色 ID';
COMMENT ON COLUMN character_equipment.slot IS '装备槽位：weapon/head/body/boots/accessory_1 等';
COMMENT ON COLUMN character_equipment.item_id IS '已装备的物品实例 ID';
COMMENT ON COLUMN character_equipment.equipped_at IS '穿戴时间';

-- ---------------------------------------------------------------------------
-- 技能
-- ---------------------------------------------------------------------------

CREATE TABLE character_skills (
    character_id    BIGINT NOT NULL REFERENCES characters (id) ON DELETE CASCADE,
    skill_id        INTEGER NOT NULL REFERENCES skill_templates (id),
    level           SMALLINT NOT NULL DEFAULT 1 CHECK (level >= 1),
    unlocked_at     TIMESTAMPTZ NOT NULL DEFAULT now(),
    PRIMARY KEY (character_id, skill_id)
);

COMMENT ON TABLE character_skills IS '角色已解锁技能';
COMMENT ON COLUMN character_skills.character_id IS '角色 ID';
COMMENT ON COLUMN character_skills.skill_id IS '技能模板 ID';
COMMENT ON COLUMN character_skills.level IS '技能等级';
COMMENT ON COLUMN character_skills.unlocked_at IS '解锁时间';

-- ---------------------------------------------------------------------------
-- 任务
-- ---------------------------------------------------------------------------

CREATE TABLE character_quests (
    id              BIGINT PRIMARY KEY GENERATED ALWAYS AS IDENTITY,
    character_id    BIGINT NOT NULL REFERENCES characters (id) ON DELETE CASCADE,
    quest_id        INTEGER NOT NULL REFERENCES quest_templates (id),
    status          SMALLINT NOT NULL DEFAULT 2 CHECK (status BETWEEN 1 AND 5),
    progress        JSONB NOT NULL DEFAULT '{}',
    accepted_at     TIMESTAMPTZ NOT NULL DEFAULT now(),
    completed_at    TIMESTAMPTZ,
    UNIQUE (character_id, quest_id)
);

CREATE INDEX idx_character_quests_status ON character_quests (character_id, status);

COMMENT ON TABLE character_quests IS '角色任务进度';
COMMENT ON COLUMN character_quests.id IS '任务进度记录 ID';
COMMENT ON COLUMN character_quests.character_id IS '角色 ID';
COMMENT ON COLUMN character_quests.quest_id IS '任务模板 ID';
COMMENT ON COLUMN character_quests.status IS '任务状态：1=locked 2=active 3=completed 4=failed 5=abandoned';
COMMENT ON COLUMN character_quests.progress IS '任务进度 JSON';
COMMENT ON COLUMN character_quests.accepted_at IS '接取时间';
COMMENT ON COLUMN character_quests.completed_at IS '完成时间';

-- ---------------------------------------------------------------------------
-- 货币
-- ---------------------------------------------------------------------------

CREATE TABLE character_wallets (
    character_id    BIGINT NOT NULL REFERENCES characters (id) ON DELETE CASCADE,
    currency_code   VARCHAR(16) NOT NULL,
    amount          BIGINT NOT NULL DEFAULT 0 CHECK (amount >= 0),
    updated_at      TIMESTAMPTZ NOT NULL DEFAULT now(),
    PRIMARY KEY (character_id, currency_code)
);

COMMENT ON TABLE character_wallets IS '角色货币钱包';
COMMENT ON COLUMN character_wallets.character_id IS '角色 ID';
COMMENT ON COLUMN character_wallets.currency_code IS '货币代码：spirit_stone/merit/sect_contribution 等';
COMMENT ON COLUMN character_wallets.amount IS '持有数量';
COMMENT ON COLUMN character_wallets.updated_at IS '最后变动时间';

-- ---------------------------------------------------------------------------
-- 宗门（公会）
-- ---------------------------------------------------------------------------

CREATE TABLE guilds (
    id              BIGINT PRIMARY KEY GENERATED ALWAYS AS IDENTITY,
    realm_id        SMALLINT NOT NULL REFERENCES realms (id),
    name            VARCHAR(32) NOT NULL,
    leader_id       BIGINT REFERENCES characters (id),
    level           SMALLINT NOT NULL DEFAULT 1,
    notice          TEXT,
    created_at      TIMESTAMPTZ NOT NULL DEFAULT now(),
    disbanded_at    TIMESTAMPTZ,
    UNIQUE (realm_id, name)
);

COMMENT ON TABLE guilds IS '宗门/公会';
COMMENT ON COLUMN guilds.id IS '宗门 ID';
COMMENT ON COLUMN guilds.realm_id IS '所属区服 ID';
COMMENT ON COLUMN guilds.name IS '宗门名称（区服内唯一）';
COMMENT ON COLUMN guilds.leader_id IS '掌门角色 ID';
COMMENT ON COLUMN guilds.level IS '宗门等级';
COMMENT ON COLUMN guilds.notice IS '宗门公告';
COMMENT ON COLUMN guilds.created_at IS '创建时间';
COMMENT ON COLUMN guilds.disbanded_at IS '解散时间，NULL 表示存续中';

CREATE TABLE guild_members (
    guild_id        BIGINT NOT NULL REFERENCES guilds (id) ON DELETE CASCADE,
    character_id    BIGINT NOT NULL REFERENCES characters (id) ON DELETE CASCADE,
    role            SMALLINT NOT NULL DEFAULT 3 CHECK (role BETWEEN 1 AND 3),
    contribution    BIGINT NOT NULL DEFAULT 0,
    joined_at       TIMESTAMPTZ NOT NULL DEFAULT now(),
    PRIMARY KEY (guild_id, character_id),
    UNIQUE (character_id)
);

COMMENT ON TABLE guild_members IS '宗门成员';
COMMENT ON COLUMN guild_members.guild_id IS '宗门 ID';
COMMENT ON COLUMN guild_members.character_id IS '成员角色 ID';
COMMENT ON COLUMN guild_members.role IS '职位：1=leader 2=elder 3=member';
COMMENT ON COLUMN guild_members.contribution IS '宗门贡献值';
COMMENT ON COLUMN guild_members.joined_at IS '加入时间';

-- ---------------------------------------------------------------------------
-- 社交
-- ---------------------------------------------------------------------------

CREATE TABLE friendships (
    id              BIGINT PRIMARY KEY GENERATED ALWAYS AS IDENTITY,
    requester_id    BIGINT NOT NULL REFERENCES characters (id) ON DELETE CASCADE,
    addressee_id    BIGINT NOT NULL REFERENCES characters (id) ON DELETE CASCADE,
    status          SMALLINT NOT NULL DEFAULT 1 CHECK (status BETWEEN 1 AND 3),
    created_at      TIMESTAMPTZ NOT NULL DEFAULT now(),
    updated_at      TIMESTAMPTZ NOT NULL DEFAULT now(),
    CHECK (requester_id <> addressee_id),
    UNIQUE (requester_id, addressee_id)
);

CREATE INDEX idx_friendships_addressee ON friendships (addressee_id, status);

COMMENT ON TABLE friendships IS '好友关系';
COMMENT ON COLUMN friendships.id IS '关系记录 ID';
COMMENT ON COLUMN friendships.requester_id IS '发起方角色 ID';
COMMENT ON COLUMN friendships.addressee_id IS '接收方角色 ID';
COMMENT ON COLUMN friendships.status IS '关系状态：1=pending 2=accepted 3=blocked';
COMMENT ON COLUMN friendships.created_at IS '创建时间';
COMMENT ON COLUMN friendships.updated_at IS '最后更新时间';

CREATE TABLE mails (
    id              BIGINT PRIMARY KEY GENERATED ALWAYS AS IDENTITY,
    receiver_id     BIGINT NOT NULL REFERENCES characters (id) ON DELETE CASCADE,
    sender_name     VARCHAR(32) NOT NULL DEFAULT '系统',
    title           VARCHAR(128) NOT NULL,
    body            TEXT,
    attachments     JSONB NOT NULL DEFAULT '[]',
    status          SMALLINT NOT NULL DEFAULT 1 CHECK (status BETWEEN 1 AND 4),
    expire_at       TIMESTAMPTZ,
    created_at      TIMESTAMPTZ NOT NULL DEFAULT now(),
    read_at         TIMESTAMPTZ,
    claimed_at      TIMESTAMPTZ
);

CREATE INDEX idx_mails_receiver ON mails (receiver_id, status);

COMMENT ON TABLE mails IS '邮件（含系统奖励）';
COMMENT ON COLUMN mails.id IS '邮件 ID';
COMMENT ON COLUMN mails.receiver_id IS '收件人角色 ID';
COMMENT ON COLUMN mails.sender_name IS '发件人显示名';
COMMENT ON COLUMN mails.title IS '邮件标题';
COMMENT ON COLUMN mails.body IS '邮件正文';
COMMENT ON COLUMN mails.attachments IS '附件列表 JSON';
COMMENT ON COLUMN mails.status IS '邮件状态：1=unread 2=read 3=claimed 4=deleted';
COMMENT ON COLUMN mails.expire_at IS '过期时间';
COMMENT ON COLUMN mails.created_at IS '发送时间';
COMMENT ON COLUMN mails.read_at IS '阅读时间';
COMMENT ON COLUMN mails.claimed_at IS '附件领取时间';

-- ---------------------------------------------------------------------------
-- 审计 & 事件 Outbox（Kafka 投递）
-- ---------------------------------------------------------------------------

CREATE TABLE audit_logs (
    id              BIGINT PRIMARY KEY GENERATED ALWAYS AS IDENTITY,
    account_id      BIGINT REFERENCES accounts (id),
    character_id    BIGINT REFERENCES characters (id),
    action          VARCHAR(64) NOT NULL,
    detail          JSONB NOT NULL DEFAULT '{}',
    ip              INET,
    created_at      TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX idx_audit_logs_created ON audit_logs (created_at DESC);

COMMENT ON TABLE audit_logs IS '操作审计日志';
COMMENT ON COLUMN audit_logs.id IS '审计记录 ID';
COMMENT ON COLUMN audit_logs.account_id IS '关联账号 ID（可选）';
COMMENT ON COLUMN audit_logs.character_id IS '关联角色 ID（可选）';
COMMENT ON COLUMN audit_logs.action IS '操作类型标识';
COMMENT ON COLUMN audit_logs.detail IS '操作详情 JSON';
COMMENT ON COLUMN audit_logs.ip IS '操作来源 IP';
COMMENT ON COLUMN audit_logs.created_at IS '记录时间';

CREATE TABLE outbox_events (
    id              BIGINT PRIMARY KEY GENERATED ALWAYS AS IDENTITY,
    aggregate_type  VARCHAR(64) NOT NULL,
    aggregate_id    BIGINT NOT NULL,
    event_type      VARCHAR(64) NOT NULL,
    payload         JSONB NOT NULL,
    created_at      TIMESTAMPTZ NOT NULL DEFAULT now(),
    published_at    TIMESTAMPTZ
);

CREATE INDEX idx_outbox_unpublished ON outbox_events (created_at)
    WHERE published_at IS NULL;

COMMENT ON TABLE outbox_events IS '事务性 Outbox，供 Kafka 异步投递';
COMMENT ON COLUMN outbox_events.id IS '事件 ID';
COMMENT ON COLUMN outbox_events.aggregate_type IS '聚合根类型，如 account/character';
COMMENT ON COLUMN outbox_events.aggregate_id IS '聚合根 ID';
COMMENT ON COLUMN outbox_events.event_type IS '事件类型';
COMMENT ON COLUMN outbox_events.payload IS '事件载荷 JSON';
COMMENT ON COLUMN outbox_events.created_at IS '事件创建时间';
COMMENT ON COLUMN outbox_events.published_at IS '投递至 Kafka 的时间，NULL 表示未发布';

-- ---------------------------------------------------------------------------
-- 触发器：自动更新 updated_at
-- ---------------------------------------------------------------------------

CREATE OR REPLACE FUNCTION set_updated_at()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = now();
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

COMMENT ON FUNCTION set_updated_at() IS 'BEFORE UPDATE 触发器：自动刷新 updated_at 字段';

CREATE TRIGGER trg_accounts_updated
    BEFORE UPDATE ON accounts
    FOR EACH ROW EXECUTE FUNCTION set_updated_at();

CREATE TRIGGER trg_characters_updated
    BEFORE UPDATE ON characters
    FOR EACH ROW EXECUTE FUNCTION set_updated_at();

CREATE TRIGGER trg_character_items_updated
    BEFORE UPDATE ON character_items
    FOR EACH ROW EXECUTE FUNCTION set_updated_at();

-- ---------------------------------------------------------------------------
-- 种子数据
-- ---------------------------------------------------------------------------

INSERT INTO realms (code, name, region, is_open, opened_at) VALUES
    ('s1', '太虚一区', 'cn', TRUE, now());

INSERT INTO realm_stages (id, name, major_realm, minor_stage, required_exp, max_level) VALUES
    (1, '炼气一层', '炼气', 1, 0, 10),
    (2, '炼气二层', '炼气', 2, 1000, 10),
    (3, '筑基初期', '筑基', 1, 10000, 20),
    (4, '金丹初期', '金丹', 1, 100000, 30);

INSERT INTO sects (id, code, name, element) VALUES
    (1, 'qingyun', '青云宗', 'wind'),
    (2, 'xuanming', '玄冥阁', 'water'),
    (3, 'lieyan', '烈阳谷', 'fire');

INSERT INTO spirit_roots (id, name, elements, rarity) VALUES
    (1, '杂灵根', '["mixed"]', 1),
    (2, '火灵根', '["fire"]', 3),
    (3, '天灵根·金', '["metal"]', 5);

INSERT INTO item_templates (id, code, name, category, quality, max_stack) VALUES
    (10001, 'spirit_stone_low', '下品灵石', 'material', 1, 9999),
    (20001, 'hp_pill', '回春丹', 'consumable', 2, 99),
    (30001, 'sword_iron', '玄铁剑', 'equipment', 3, 1);

INSERT INTO skill_templates (id, code, name, skill_type, element, cooldown_ms, gas_tag) VALUES
    (1, 'sword_qi', '御剑诀', 'active', 'metal', 3000, 'GA_SwordQi'),
    (2, 'fire_ball', '火球术', 'active', 'fire', 5000, 'GA_FireBall');

INSERT INTO quest_templates (id, code, name, quest_type, min_level, rewards) VALUES
    (1001, 'main_001', '拜入仙门', 'main', 1, '{"exp":500,"items":[{"id":20001,"qty":5}]}'),
    (2001, 'daily_gather', '采集灵草', 'daily', 1, '{"currency":{"spirit_stone":100}}');

COMMIT;
