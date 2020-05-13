FROM ubuntu:18.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    libcurl4-openssl-dev \
    nano iproute2 iputils-ping

COPY --from=dpdk-build /usr /usr

# compile app
ADD . /app
WORKDIR /app/build
RUN rm -rf * && cmake .. && make
RUN echo "export LD_LIBRARY_PATH=/usr/local/lib/x86_64-linux-gnu" >> /root/.bashrc

LABEL RUN docker run -it --privileged -v /sys/bus/pci/drivers:/sys/bus/pci/drivers -v /sys/kernel/mm/hugepages:/sys/kernel/mm/hugepages -v /sys/devices/system/node:/sys/devices/system/node -v /dev:/dev -v /var/run/dpdk/rte:/var/run/dpdk/rte

CMD /bin/bash