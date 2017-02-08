# build .axf using the below commands, and use Ozone to flash/debug
cd bsp/stm32f40x/
scons --verbose --clean
scons --verbose

# After booting, use msh command to switch msh,
# then ifconfig to check the IP address
# After get IP, run tcpserv to start tcp server on 5000
# On Linux client, nc <ip> 5000 to connect to board. the uart2 is connect to TCP client
