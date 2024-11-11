# minecraft-lan-registration
Advertise any Minecraft server on a local area network.

This project is intended for MacOS, but likely also works on Linux.

## Usage
Build the project using the makefile:

```bash
make clean all install
```

Then, run the generated executable using root permissions:

```bash
# Usage: ./registerLanServer <host_address> <port> <motd>
sudo ./registerLanServer 123.123.123.123 25565 "A display message"
```