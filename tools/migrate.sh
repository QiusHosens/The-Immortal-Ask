#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SCHEMA="${ROOT}/deploy/database/migrations/001_initial_schema.sql"
DATABASE_URL="${DATABASE_URL:-postgres://immortal:immortal@localhost:5432/immortal_ask}"

echo "Applying schema to ${DATABASE_URL}"
echo "Tip: create database/user first with 000_create_database.sql if needed."
psql "${DATABASE_URL}" -f "${SCHEMA}"
echo "Done."
