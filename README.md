📖 Overview
This repository contains the implementation of ISO 15765-2 (ISO-TP), a transport/network layer protocol for CAN-based automotive networks, it can be integrated with emb::TLS (Transport Layer Security) to secure in-vehicle communication to address vulnerabilities in Controller Area Networks (CAN) by providing:

Segmentation/reassembly of large messages (up to 4095 bytes).

Secure data transmission via TLS 1.2 handshake and encryption.

Evaluation of handshake latency, secure data rate, and memory footprint.

Key Motivation: Modern vehicles are vulnerable to attacks due to CAN's lack of encryption, integrity checks, and authentication. This project secures ECU-to-gateway communication using standardized protocols.

🚀 Features
ISO 15765-2 Implementation
Segmentation & Reassembly:

Single Frame (SF), First Frame (FF), Consecutive Frame (CF), and Flow Control (FC) handling.

Configurable Block Size (BS) and Separation Time (STmin).

Non-Blocking Design: Asynchronous operation with state machines (Tx/Rx).

Full-Duplex Mode: Simultaneous transmission/reception.

Error Handling: Timeout management (N_As, N_Bs, N_Ar, N_Cr).

emb::TLS Integration
TLS 1.2 handshake (full/abbreviated) over ISO-TP.

Secure data encryption/decryption with configurable cipher suites.

Hardware Support
Tested on STM32F407 Discovery Kit (ARM Cortex-M4).

Interoperability with Linux SocketCAN ISO-TP.

⚙️ Installation & Usage
Prerequisites
ARM Cortex-M4 toolchain (GCC ARM Embedded).

CAN interface (e.g., PCAN-USB, SocketCAN).

STM32CubeMX (for HAL configuration).
