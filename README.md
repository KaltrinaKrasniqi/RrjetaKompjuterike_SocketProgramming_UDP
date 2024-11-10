UDP Server-Client Communication Project

Overview
This project implements a UDP-based server-client architecture in C, designed for real-time communication across multiple devices on a network. The server manages connections, enforces user permissions, logs requests for auditing, and maintains efficient resource usage. One client has full access to execute commands on the server, while other clients have read-only access.

Features

Server

Configurable Port and IP Address: The server’s IP and port number are customizable to allow flexible setup and network configuration.
Connection Management:
  - Listens for incoming connections from multiple clients.
  - Limits active connections; if the connection count exceeds a set threshold, new connections are either rejected or queued.
Request Logging:
  - Logs every request with a timestamp and sender IP address for later auditing.
Message Monitoring:
  - Reads and stores messages from clients for real-time monitoring.
  - Automatically disconnects inactive clients after a specified period and reconnects them if they return.
Full Access Control:
  - Grants one client full access to read, write, and execute commands on server files and directories.

Client
UDP Socket Connection:
  - Establishes a UDP connection to the server using the server's IP address and port number.
Permissions:
  -Full Access Client: Has read, write, and execute privileges on the server.
  -Read-Only Clients: Can only read data from the server.
Communication and Response:
  - Sends text-based messages to the server and receives responses.
  - Full-access clients have optimized response times compared to read-only clients.


Requirements
- Communication over UDP protocol.
- Connection of at least four devices on the same network.
- C programming language.

Configuration
- IP Address and Port: Ensure the IP and port number on each client matches those specified on the server.
- Permissions: Configure client permissions in the server code, assigning one client full access and others read-only access.

Logging and Error Handling
- Server logs all client requests with timestamps and IP addresses.
- The server disconnects inactive clients and restores their session if they reconnect.
