FROM --platform=linux/amd64 ubuntu:20.04 as builder

RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential

ADD . /aqcc
WORKDIR /aqcc
RUN make

RUN mkdir -p /deps
RUN ldd /aqcc/as/as | tr -s '[:blank:]' '\n' | grep '^/' | xargs -I % sh -c 'cp % /deps;'
RUN ldd /aqcc/cc/cc | tr -s '[:blank:]' '\n' | grep '^/' | xargs -I % sh -c 'cp % /deps;'

FROM ubuntu:20.04 as package

COPY --from=builder /deps /deps
COPY --from=builder /aqcc/as/as /aqcc/as/as
COPY --from=builder /aqcc/cc/cc /aqcc/cc/cc
ENV LD_LIBRARY_PATH=/deps
