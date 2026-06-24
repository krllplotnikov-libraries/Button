FROM gcc:15

RUN apt-get update && apt-get install -y \
    cmake \
    build-essential \
    libgtest-dev \
    libssl-dev \
    lcov \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /button

COPY . /button

RUN chmod +x /button/tests/run_test

ENTRYPOINT [ "bash", "-c" ]

CMD ["cd tests && ./run_test"]