# UDP Protocol for Distributed Neural Network

## Collaborators

- GINO SEBASTIAN DIAZ NEYRA
- RODRIGO GOMEZ SAN ROMAN
- MATIAS PAVEL SANCHEZ CUNO
- NAYALEM KARIM ARUNE CHAHUA

---

## Datagram Structure

Each UDP datagram has a fixed size of *500 bytes*.  
The protocol uses the following structure:

# Datagram Structure

```text
+------+-----+---------+------+
| TYPE | DATA_SIZE |   DATA   |
+------+-----+---------+------+
| 1 B  |    4 B    | Variable |
+------+-----+---------+------+
```

```cabezera
+------+-----+---------+-------------+------------+
| SEQ | NODE_ID | CHUNK_TOTAL | CHUNK_NUM | HASH |
+------+-----+---------+-------------+------------+
| 4 B |   2 B   |     4 B     |    4 B    | 32 B |
+------+-----+---------+-------------+------------+

---

## Packet Fields

| Field | Size | Description |
|:--|:--:|:--|
| **TYPE** | 1 byte | Identifies the datagram type. Allows the receiver to interpret the `DATA` field correctly. Example values: `MATRIX`, `DATA`, `RESULT`, `ACK`, `NACK`, `FINISH`. |
| **SEQ** | 4 byte | Datagram sequence number. Used to match packets with their corresponding `ACK` or `NACK`. |
| **NODE_ID** | 2 bytes | Logical identifier of the node associated with the datagram. Example: `M0` for master node, `S1`, `S2`, `S3` for slave nodes. |
| **CHUNK_TOTAL** | 4 bytes | Total number of chunks when the transmitted data is fragmented. |
| **CHUNK_NUM** | 4 bytes | Current chunk index. Helps reconstruct fragmented data in the correct order. |
| **DATA_SIZE** | 4 bytes | Actual size of the `DATA` field in bytes. Indicates how many bytes contain valid information. |
| **DATA** | Variable | Payload of the datagram. Can contain matrices, weights, input data, partial results, control messages, or errors. |
| **HASH** | 32 bytes | Integrity verification hash. The receiver recalculates the hash and compares it against this field to detect corruption. |

---
Jacobson/ Karels Algorithm
• Diff = sampleRTT - EstRTT 
• EstRTT = EstRTT + ( d x Diff) 
• Dev = Dev + d ( |Diff| - Dev)

TimeOut = µ x EstRTT + φ x Dev – 
where µ = 1 and φ = 4 

TCP uses an initial value of 3 seconds [RFC2988],
which is also RECOMMENDED as an initial value for UDP applications.
SIP [RFC3261] and GIST [GIST] use an initial value of 500 ms, and
initial timeouts that are shorter than this are likely problematic in
many cases.[RFC5405] (https://www.rfc-editor.org/rfc/rfc5405?)

## Notes

- The protocol supports **fragmented transmission** through `CHUNK_TOTAL` and `CHUNK_NUM`.
- `SEQ` enables reliability mechanisms such as acknowledgments (`ACK`) and retransmissions (`NACK`).
- `HASH` provides **datagram integrity verification**.
- `DATA_SIZE` allows variable-length payloads inside the datagram.
