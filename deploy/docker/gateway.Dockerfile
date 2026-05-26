# syntax=docker/dockerfile:1

FROM rust:1-bookworm AS builder
WORKDIR /app
COPY server/Cargo.toml server/Cargo.lock* ./
COPY server/common ./common
COPY server/gateway ./gateway
RUN cargo build --release -p immortal-ask-gateway

FROM debian:bookworm-slim
RUN apt-get update && apt-get install -y --no-install-recommends ca-certificates \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY --from=builder /app/target/release/gateway /app/gateway
EXPOSE 8080
ENTRYPOINT ["/app/gateway"]
