# syntax=docker/dockerfile:1

FROM rust:1-bookworm AS builder
WORKDIR /app
COPY server/Cargo.toml server/Cargo.lock* ./
COPY server/common ./common
COPY server/game ./game
RUN cargo build --release -p immortal-ask-game

FROM debian:bookworm-slim
RUN apt-get update && apt-get install -y --no-install-recommends ca-certificates \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY --from=builder /app/target/release/game /app/game
ENTRYPOINT ["/app/game"]
