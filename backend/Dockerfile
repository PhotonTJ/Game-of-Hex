# Build context is the repo root (see docker-compose.yml) so this stage can
# see both the CMake/C++ engine and the FastAPI app.

FROM python:3.11-slim AS engine-builder
RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential cmake \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /src
RUN pip install --no-cache-dir pybind11
COPY CMakeLists.txt ./
COPY engine ./engine
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
      -DHEX_BUILD_TESTS=ON -DHEX_BUILD_TOOLS=OFF -DHEX_BUILD_PYTHON_BINDINGS=ON \
    && cmake --build build -j"$(nproc)" \
    && ./build/engine/hex_engine_tests

FROM python:3.11-slim AS runtime
WORKDIR /app
COPY backend/requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt
COPY backend/app ./app
COPY --from=engine-builder /src/build/engine/bindings/hex_engine*.so ./app/
EXPOSE 8000
CMD ["uvicorn", "app.main:app", "--host", "0.0.0.0", "--port", "8000"]
