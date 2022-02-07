FROM ubuntu:18.04 as builder

LABEL maintainer="Filippo Sallemi <fsallemi@nomadnt.com>"

RUN apt update && apt install -y cmake build-essential ragel llvm-7-dev zlib1g-dev

COPY . /monicelli

RUN mkdir -p /monicelli/build \
    && cd /monicelli/build \
    && cmake .. -DCMAKE_INSTALL_PREFIX="/usr/local" \
    && make all install

FROM ubuntu:18.04

RUN apt update && apt install -y g++

COPY --from=builder /usr/local/bin/mcc /usr/local/bin/mcc

CMD ['mcc']