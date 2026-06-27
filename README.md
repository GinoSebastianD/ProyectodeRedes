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

### Data Section

<table>
  <thead>
    <tr>
      <th align="center">DATA</th>
      <th align="center">PADDING</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td align="center"><strong>Variable up to 485 bytes</strong></td>
      <td align="center"><strong>Variable</strong></td>
    </tr>
  </tbody>
</table>

### Header Section

<table>
  <thead>
    <tr>
      <th align="center">CHECKSUM</th>
      <th align="center">NODE_ID</th>
      <th align="center">FLAGS</th>
      <th align="center">SEQ</th>
      <th align="center">TYPE</th>
      <th align="center">DATA_SIZE</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td align="center"><strong>2 bytes</strong></td>
      <td align="center"><strong>2 bytes</strong></td>
      <td align="center"><strong>2 bytes</strong></td>
      <td align="center"><strong>4 bytes</strong></td>
      <td align="center"><strong>1 byte</strong></td>
      <td align="center"><strong>4 bytes</strong></td>
    </tr>
  </tbody>
</table>


### Header Fields

<table>
  <thead>
    <tr>
      <th align="center">Field</th>
      <th align="center">Size</th>
      <th align="center">Description</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td><strong>CHECKSUM</strong></td>
      <td align="center">2 bytes</td>
      <td>Integrity value used to verify whether the datagram was received correctly.</td>
    </tr>
    <tr>
      <td><strong>NODE_ID</strong></td>
      <td align="center">2 bytes</td>
      <td>Numeric logical identifier of the node involved in the communication. Example: Master = 0, Slave 1 = 1.</td>
    </tr>
    <tr>
      <td><strong>FLAGS</strong></td>
      <td align="center">2 bytes</td>
      <td>Indicates the datagram position in a fragmented message: start, body, or end.</td>
    </tr>
    <tr>
      <td><strong>SEQ</strong></td>
      <td align="center">4 bytes</td>
      <td>Datagram sequence number used for ordering and ACK/NACK matching.</td>
    </tr>
    <tr>
      <td><strong>TYPE</strong></td>
      <td align="center">1 byte</td>
      <td>Identifies the message type: dataset, master weights, slave weights, ACK, or NACK.</td>
    </tr>
    <tr>
      <td><strong>DATA_SIZE</strong></td>
      <td align="center">4 bytes</td>
      <td>Indicates the number of valid payload bytes stored in the data section.</td>
    </tr>
  </tbody>
</table>
---

## Jacobson/Karels Algorithm

```text
Diff    = sampleRTT - EstRTT
EstRTT  = EstRTT + (d × Diff)
Dev     = Dev + d × (|Diff| - Dev)

TimeOut = μ × EstRTT + φ × Dev
```

where:

```text
μ = 1
φ = 4
```

TCP uses an initial value of **3 seconds** [RFC2988], which is also recommended as an initial value for UDP applications.  
SIP [RFC3261] and GIST [GIST] use an initial value of **500 ms**, and initial timeouts shorter than this are likely problematic in many cases.  
Reference: [RFC5405](https://www.rfc-editor.org/rfc/rfc5405)

## Notes

- The protocol supports **fragmented transmission** through `FLAG`.
- `SEQ` enables reliability mechanisms such as acknowledgments (`ACK`), negative acknowledgments (`NACK`), and retransmissions.
- `PADDING` is used to complete the fixed datagram size when the payload does not fill all the available space.
