echo 1024 | sudo tee /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
sudo mount -t hugetlbfs nodev /mnt/huge

sudo modprobe uio_pci_generic
python3 ~/dpdk-app-docker/deps/dpdk-code/usertools/dpdk-devbind.py --bind=uio_pci_generic enp1s0f1
python3 ~/dpdk-app-docker/deps/dpdk-code/usertools/dpdk-devbind.py --bind=uio_pci_generic enp1s0f0

