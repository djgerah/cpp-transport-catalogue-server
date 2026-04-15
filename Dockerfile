# ---------- Stage 1: build ----------
FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    g++ \
    make \
    cmake \
    libcpprest-dev \
    libboost-system-dev \
    libssl-dev \
    libgtest-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN make build

# ---------- Stage 2: runtime ----------
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libcpprest2.10 \
    libboost-system1.74.0 \
    libssl3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/transport_catalogue .

EXPOSE 8080

CMD ["./transport_catalogue"]