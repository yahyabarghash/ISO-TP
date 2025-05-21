## ISO-TP with TLS for Deeply Embedded Devices

Secure, non-blocking, full-duplex ISO-TP (ISO 15765-2) implementation for CAN-based ECUs, integrated with emb::TLS for end-to-end vehicle network security.

---

### Table of Contents

1. [Project Overview](#project-overview)  
2. [Goals](#goals)  
3. [Features](#features)  
4. [Architecture](#architecture)  
5. [Quick Start](#quick-start)    
   - [Dependencies](#dependencies)    
   - [Building](#building)    
   - [Initialization](#initialization)  
6. [API Reference](#api-reference)  
   - `isotp_init()`  
   - `isotp_send()`  
   - `isotp_receive()`  
   - Callback registration  
7. [Performance & Testing](#performance--testing)  
8. [Contributing](#contributing)  
9. [License](#license)

---

## Project Overview

Modern in-vehicle networks (IVNs) are exposed to eavesdropping, message injection, and DoS attacks. This project provides:

- **ISO-TP (ISO 15765-2)** transport layer over CAN for messages up to 4095 bytes.  

 Note: emb::TLS** integration "can" be made for mutual authentication, confidentiality, and integrity on deeply embedded ECUs.

All designed for resource-constrained microcontrollers (e.g., ARM Cortex-M) to secure ECU-to-gateway channels.

---

## Goals

- Understand ISO 15765-2 specification.  
- Provide a standalone, non-blocking ISO-TP implementation.  
- Integrate emb::TLS over ISO-TP for secure channel establishment.  
- Perform interoperability and performance evaluations.

---

## Features

1. **Independency**  
   Abstract data-link interface decouples ISO-TP from any specific CAN driver (`CAN_init`, `CAN_snd`, `CAN_recv`, `CAN_ioctl`).  
2. **Non-Blocking**  
   All send/receive operations are spread across periodic `isotp_tick()` calls; CAN frames are sent via interrupt-driven driver, avoiding application stalls.  
3. **Full-Duplex Mode**  
   Independent Tx/Rx state machines allow simultaneous transmission and reception of large messages.

---

## Architecture

```plaintext
┌───────────────────┐     CAN      ┌───────────────────┐
│  Application      │ <──────────> │  Other ECU / PC   │
│  + emb::TLS API   │             │  ISO-TP Socket    │
└─▲──────────▲──────┘             └───────┬───────────┘
  │          │                                  │
  │ TLS over │                                  │
  │ ISO-TP   │                                  │
  │          │                                  │
┌─┴──────────┴───────┐                   CAN    │
│    isoTP Layer     │ <──────────> ────────────┘
│  ┌───────────────┐ │
│  │ tpSnd (Tx FSM)│ │
│  │ tpRcv (Rx FSM)│ │
│  └───────────────┘ │
└────────────────────┘
```

- **tpSnd**: segments messages into SF/FF/CF, handles FlowControl replies.  
- **tpRcv**: reassembles frames, allocates buffers, signals completion or errors.  
- **isoTP**: top-level FSM that dispatches frames to `tpSnd` or `tpRcv`.

---

## Quick Start

### Dependencies

- ARM Cortex-M toolchain (GCC, CMSIS)  
- CAN driver that implements:
  - `CAN_init(baudrate)`  
  - `CAN_snd(id, data, len)`  
  - `CAN_recv(&id, buffer, &len)`  
  - `CAN_ioctl(...)`

### Building

1. Clone repository:  
   ```sh
   git clone https://github.com/yahyabarghash/ISO-TP.git
   cd ISO-TP
   ```
2. Configure your toolchain and CAN driver in `Makefile`.  
3. Build:  
   ```sh
   make all
   ```

### Initialization

```c
// Define callback for send/receive completion
void isotp_cb(uint32_t task_id, N_Result_t result);

// Init ISO-TP at 500 kbps and register callback
isotp_init(500000, &isotp_cb);
```

---

## API Reference

### `isotp_init(uint32_t baudrate, isotp_callback_t cb)`

- **baudrate**: CAN bit-rate (e.g. 500000)  
- **cb**: invoked on message completion or error

### `int isotp_send(uint8_t *data, uint16_t len)`

- Buffers and begins transmission of an `len`-byte message.  
- Returns a **task ID** or error if no buffer space.

### `int isotp_receive(uint8_t *buffer, uint16_t *len)`

- Attempts to fetch a complete message.  
- Returns **task ID**, sets `*len`, or 0 if none available.

### Callback Prototype

```c
typedef void (*isotp_callback_t)(uint32_t task_id, N_Result_t status);
```

- **status**: e.g. `en_nResultOk`, `en_nResult_timeoutAs`, `en_nResultRx_wrongSNRx`, etc.

---

## Performance & Testing

- **Interoperability** with Linux Socket CAN: successful SF/FF exchange.  
- **Overhead**: ≈10 µs per frame at 1 Mbps  
- **TLS Handshake**: tuning `BS` and `STmin` yields min latency at `BS=10, STmin=0 ms`.  
- **Secure Data Rate**: linear with payload; best at `STmin=0 ms` but watch bus occupancy.  
- **Memory Footprint**:
  - ISO-TP alone: ~9.8 KB SRAM  
  - ISO-TP + emb::TLS: ~110 KB SRAM

See [Measurements & Analysis](./docs/Measurements.md) for full test results.

---

## Contributing

1. Fork & clone.  
2. Create feature branch: `git checkout -b feat/my-feature`.  
3. Commit & push.  
4. Submit a PR.

Please follow the existing coding style and include tests for new functionality.

---

## License

This project is licensed under the MIT License. See [LICENSE](./LICENSE) for details.
