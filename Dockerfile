FROM ubuntu:22.04 AS cpp-builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY simulator/ /build/
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release . \
    && cmake --build build --parallel

FROM python:3.12-slim AS final

WORKDIR /app

COPY scanner/ ./scanner/
COPY data/     ./data/

RUN pip install --no-cache-dir -r scanner/requirements.txt

COPY --from=cpp-builder /build/build/handover_sim ./handover_sim

COPY --from=cpp-builder /build/build/handover_tests ./handover_tests

VOLUME ["/app/data"]

ENTRYPOINT ["./handover_sim"]

CMD ["synthetic"]
