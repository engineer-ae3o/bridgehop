# Bridgehop Protocol Specification (v1 draft)

This document describes the Bridgehop protocol library. It does not depend on any particular hardware or radio. The Device Specification covers the firmware, the radio, Bluetooth and storage.

MUST means required. SHOULD means strongly recommended. MAY means optional.

## 1. Scope

This document covers:

- packet formats
- cryptography
- sessions and handshakes
- relaying and duplicate suppression
- delivery confirmation
- what the library needs from the platform

This document does not cover Bluetooth, flash storage, radio settings, duty-cycle enforcement or anything shown on screen.

Bridgehop is a multi-hop mesh messaging protocol with end-to-end encryption, forward secrecy and sender authentication. Every packet is flooded: each node that hears a packet sends it on, within a hop limit.

Delivery is best-effort. The protocol never retransmits a message on its own and does not guarantee order. When the target node receives, reassembles and verifies a whole message, it sends a confirmation back. If no confirmation arrives within the wait time, the sender reports the message as unconfirmed, and the application decides whether to send it again.

## 2. Node roles

**Passive node.** Relays only. It has no identity and no keys. When it hears a packet it has not seen before, it lowers the hop count, waits a random time and sends the packet again.

**Active node.** Has an identity. It reads packets addressed to it and relays all others in the same way as a passive node.

There is no routing table.

## 3. Cryptography

| Purpose | Algorithm |
|---|---|
| Signatures | Ed25519. Seed 32 bytes, public key 32 bytes, signature 64 bytes (R 32, S 32) |
| Key agreement | X25519, new key pair for every handshake |
| Key derivation | HKDF with SHA-256 |
| Encryption and authentication | AES-128-CCM, 12 byte nonce, 16 byte tag |
| Hashing | SHA-256 |

Numbers longer than one byte are stored little-endian. This applies to the message counter, the handshake counter and any other integer field, including when they are part of signed or authenticated data. Keys, signatures, IDs, tags and hashes are byte strings and are sent in the order the algorithm produces them.

## 4. Identity

- On first boot an active node creates a random 32 byte seed. The seed is the only secret that is saved.
- The Ed25519 private scalar and public key are derived from the seed. The public key is worked out at boot and kept in memory.
- The node ID is the first 16 bytes of SHA-256(public key), in the order the hash produces them. Users give each other this ID.
- A node MAY save its ID. If it does, it SHOULD recompute the ID at boot and compare.
- The full public key arrives in the first handshake and is checked against the ID by hashing.
- Contacts are mutual. An active node MUST ignore a handshake from any ID that is not in its contact list. The device sets the number of contacts (default 500).
- Changing an ID without a full reset is not supported in version 1.

## 5. Packet formats

The largest packet size is set by the transport. The sizes below assume 255 bytes (LoRa).

### 5.1 Header (41 bytes, always in the clear)

| Offset | Size | Field |
|---|---|---|
| 0 | 1 | type, version and hop count |
| 1 | 8 | packet ID |
| 9 | 16 | sender ID |
| 25 | 16 | target ID |

The first byte:

- bits 7 and 6: packet type
  - 00 = data
  - 01 = handshake
  - 10 = delivery confirmation
  - 11 = close
- bits 5 and 4: protocol version (this document is version 1)
- bits 3 to 0: hop count

Rules:

- The hop count is 0 to 10. The starting value is configurable (default 10). A received value above 10 is dropped as corrupted.
- A relay changes only the hop count bits.
- A packet with an unknown or unsupported version is dropped and not relayed.
- The packet ID is the first 8 bytes of SHA-256(byte-appended sender ID, target ID, payload).
- Every node recomputes the packet ID and drops the packet if it does not match.
- The packet length is not in the header. The transport reports it.

### 5.2 Handshake payload (136 bytes, 177 bytes in total)

| Size | Field |
|---|---|
| 8 | handshake counter |
| 32 | ephemeral X25519 public key |
| 32 | signature R |
| 32 | signature S |
| 32 | sender's Ed25519 public key |

The handshake counter is a number saved in flash on each device. It only increases. Each node saves the highest handshake counter seen from each contact.

The signature covers, in order:

1. the text `BH1-HS`
2. sender ID
3. target ID
4. ephemeral public key
5. the sender's own handshake counter
6. the other side's handshake counter

In an initiation, item 6 is 8 zero bytes. In a response, item 6 is the handshake counter from the initiation.

### 5.3 Data payload (22 bytes plus ciphertext)

| Size | Field |
|---|---|
| 16 | tag |
| 4 | message counter |
| 1 | total number of chunks (1 to 255) |
| 1 | chunk index (starts at 0) |
| rest | ciphertext chunk |

Each data packet has 63 bytes of overhead (41 header plus 22). At 255 bytes, a chunk carries up to 192 bytes of ciphertext.

Chunk size is the packet size minus 63. Every chunk except the last is exactly that size. The last chunk is between 1 byte and a full chunk. A chunk goes at position `index × chunk size`.

The tag, message counter and total are the same in every chunk of one message.

### 5.4 Confirmation payload (24 bytes, 65 bytes in total)

| Size | Field |
|---|---|
| 16 | tag |
| 4 | message counter (the sender's own next counter) |
| 4 | acked counter (the message counter of the message being confirmed) |

There is no ciphertext. The tag is made by AES-CCM over an empty message.

### 5.5 Close payload (21 bytes, 62 bytes in total)

| Size | Field |
|---|---|
| 16 | tag |
| 4 | message counter (the sender's own next counter) |
| 1 | reason |

There is no ciphertext. The tag is made by AES-CCM over an empty message.

Reason codes:

- 0 = closed by user
- 1 = idle timeout
- 2 = evicted for space
- 3 = confirmation timeout

### 5.6 Authenticated fields

The authentication fields are, in this order:

- data: type value (one byte), sender ID, target ID, message counter, total chunks
- confirmation: type value, sender ID, target ID, message counter, acked counter
- close: type value, sender ID, target ID, message counter, reason
- handshake: covered by the signature (see 5.2)

The chunk index is not part of a data packet's authentication fields. A message is encrypted once and has one tag. A wrong index puts the chunks in the wrong order, and the tag check fails.

## 6. Relaying and duplicate suppression

Every node keeps a duplicate cache of packet IDs.

| Setting | Default |
|---|---|
| Cache size | 2048 entries (configurable) |
| Entry lifetime | 120 seconds since the packet was last heard |
| When full | remove the oldest entry |
| Delay before relaying | random, 0 to 250 ms (configurable) |

When a packet arrives, every node does this, in order:

1. Look the packet ID up in the cache. If it is there, drop the packet. This applies to every node, including the target.
2. Check the version and the hop count (not above 10). Drop the packet if any is wrong.
3. Add the packet ID to the cache.
4. If this is an active node and the target ID is its own ID, process the packet. Do not relay it.
5. If the hop count is 0, drop the packet. Otherwise lower the hop count by one, wait a random delay and send the packet again.

A node MAY cancel a pending relay if it hears the same packet from another node first.

## 7. Sessions

An active node can have several sessions at once, one per peer. The maximum number is a setting.

A session holds:

- the peer's ID and public key
- a send key and send IV
- a receive key and receive IV
- the next send message counter
- a window of the last 64 receive message counters
- the time of last activity
- the session state

### 7.1 States

Initiator: none, pending, established.

Responder: none, tentative, established. A tentative session becomes established when the first data packet from the initiator passes its tag check.

### 7.2 Handshake steps

1. Before starting a handshake, a node looks for an established session with the target. If one exists, it uses it.
2. The initiator makes a new X25519 key pair, increases its saved handshake counter, signs, and sends a handshake packet. The session is pending.
3. The responder checks the handshake (see below). If it passes, it makes its own key pair, derives the keys, increases its own handshake counter and sends a signed response. The session is tentative.
4. The initiator checks the response against its own handshake counter, derives the keys, and the session is established.

Checks on a received handshake, in order:

1. The sender is in the contact list and the target ID is mine.
2. The handshake counter is higher than the last one saved for that sender.
3. SHA-256(public key) starts with the sender ID.
4. The signature is valid.

The new handshake counter is saved to the contact's record only after all four checks pass. Failed checks drop the packet silently.

### 7.3 Key derivation

1. Shared secret = X25519(my ephemeral private key, their ephemeral public key). An all-zero result is rejected.
2. HKDF extract: salt is the initiator's handshake counter followed by the responder's handshake counter. Input key material is the shared secret.
3. HKDF expand: info is `BH1-KEYS`, then the initiator ID, the responder ID, the initiator's ephemeral public key and the responder's ephemeral public key. Output length is 56 bytes.
4. The 56 bytes are split into: initiator-to-responder key (16), initiator-to-responder IV (12), responder-to-initiator key (16), responder-to-initiator IV (12).
5. Each side sends with its own direction's key and IV and receives with the other.
6. The ephemeral private key is erased once the keys are derived.

Nonce: the 12 byte IV XOR the message counter (4 bytes, little-endian) placed in the last 4 bytes. The first 8 bytes of the value being XORed are zero.

Message counters:

- Each direction has its own message counter, starting at 0.
- A data message uses one message counter, however many chunks it has.
- Confirmation and close packets also use one message counter each, taken from the same send counter.
- A node MUST start a new handshake before its send counter reaches 4,294,967,295.
- A message counter MUST NEVER be used twice with the same key.

### 7.4 Replacing a session

A handshake from a contact that passes every check replaces any existing session with that contact.

1. The old session stays until the new handshake has passed all checks.
2. The swap then happens in one step: new keys and IVs, both message counters set to 0, receive window cleared.
3. Half-assembled messages from that peer are discarded and the application is told.
4. Messages waiting for confirmation on the old session are reported as failed.

### 7.5 Closing a session

A node closes a session by sending a close packet, then erasing the session keys.

A node sends a close when:

- the application closes the session
- the idle timeout ends
- the session is evicted to free a slot
- a message goes unconfirmed (section 9)

When a node receives a close packet, it checks the tag and the message counter window under the session keys. If they pass:

1. The session is erased.
2. Half-assembled messages from that peer are discarded and the application is told.
3. Messages waiting for confirmation are reported as failed.
4. The application is told the session was closed by the peer, with the reason.

A close packet is not confirmed and is not repeated. A close packet that fails its checks is dropped silently.

When the session table is full and a new session is needed, the session idle the longest is closed with reason 2.

### 7.6 Simultaneous handshakes

If a node has a pending handshake to X and a handshake from X arrives:

1. It first checks the handshake as a response to its own pending handshake. If that passes, the handshake completes normally.
2. Otherwise it is a competing initiation. The node with the lower ID stays the initiator. If my ID is lower, I ignore theirs. If my ID is higher, I drop my pending handshake and answer theirs.

### 7.7 Timeouts

| Timer | Default | Meaning |
|---|---|---|
| Handshake timeout | 60 seconds | A pending or tentative session that has not completed is dropped. The initiator reports the failure. |
| Session idle timeout | 1 hour | No valid data, confirmation or close in either direction. The session is closed. |
| Reassembly timeout | 45 seconds (30 to 60 allowed) | Starts when the first chunk arrives. An incomplete message is discarded and the application is told. |
| Confirmation wait | 120 seconds | See section 9. |

Handshake processing is rate-limited per sender and in total.

## 8. Sending and receiving data

**Sending.**

1. Encrypt the whole message once with AES-CCM, using the send key, the nonce from the next message counter and the data authentication fields.
2. Split the ciphertext into chunks.
3. Send each chunk as its own packet with the same tag, message counter and total, and its own index.

The largest message is the number of chunks allowed times the chunk size. Default is 16 chunks. The limit is 255 chunks.

**Receiving.**

1. Find the session by sender ID.
2. Reject a message counter that is older than the window or already marked in it.
3. Collect chunks by (peer, message counter). Check total, index and size against 5.3. A chunk whose tag or total differs from the first chunk is dropped.
4. When every chunk is present, join them, decrypt and check the tag.
5. After the tag passes: mark the message counter as used, refresh the session timer, give the message to the application and send a confirmation.

A failed tag changes no state.

When the reassembly timeout ends with chunks missing, the buffer is freed and the application is told the message was incomplete. The number of messages assembled at once is limited per peer and in total.

## 9. Delivery confirmation

A confirmation tells the sender that the target node received every chunk, put them together, decrypted the message and the tag was valid.

**Receiver.**

1. After step 5 of section 8 succeeds, send a confirmation to the sender.
2. Send 3 copies by default, with a random gap of 1 to 5 seconds between copies.
3. Each copy uses a new message counter. The acked counter is the same in every copy.
4. No confirmation is sent for a duplicate, a replay or a message that failed a check.
5. A confirmation is not confirmed.

**Sender.**

1. After the last chunk has been transmitted, start a timer of 120 seconds (configurable).
2. Track each message waiting for confirmation by (peer, message counter).
3. When a valid confirmation arrives with a matching acked counter, report the message as confirmed. A valid confirmation also refreshes the session timer.
4. Later copies of the same confirmation are ignored.
5. When the timer ends, report the message as unconfirmed, send a close packet with reason 3 and erase the session. Other messages waiting on that session are reported as failed. The application decides whether to send the message again.

The number of messages waiting for confirmation at once is limited.

## 10. Platform requirements

The library is written in C++23 and takes these as concepts or function-pointer tables.

| Piece | What it does |
|---|---|
| Transport | Sends a packet and returns true or false. Reports the largest packet size. Received packets are passed to the library with optional signal information. |
| Clock | Returns a steadily increasing time in milliseconds |
| Scheduler | Runs a callback after a delay and cancels it, or provides a regular tick |
| Random numbers | Fills a buffer with secure random bytes |
| Crypto | SHA-256, HKDF-SHA256, Ed25519 (public key from seed, sign, verify), X25519 (make key, agree), AES-128-CCM (seal, open), memory wipe |
| Identity and contacts | Loads and saves the seed. Looks up a contact by ID. Reads and writes the last handshake counter seen from a contact. Reads and writes this device's handshake counter. |
| Event sink | Receives: message received, message incomplete, session opened, session closed (with reason), send result (confirmed, unconfirmed, failed) |

Settings chosen at build or start: node role, starting hop count, cache size, relay delay, confirmation copies and gap, all timeouts, maximum sessions, maximum chunks, maximum messages waiting for confirmation.

A passive-only build needs the transport, clock, scheduler, random numbers and SHA-256.

The library is single-threaded. The platform serialises calls into it. It SHOULD NOT allocate memory after startup.

## 11. Errors and drops

Dropped silently, with no reply: unknown version, hop count above 10, wrong packet ID, unknown contact, bad signature, bad tag, replayed counter. Each increments a debug counter.

Reported to the application: no session could be set up, message incomplete, message unconfirmed, session closed, message too large.

## 12. Security summary and limits

- Messages are encrypted and authenticated. Keys are new for every session.
- Senders are authenticated by signature over the handshake.
- Old handshakes are rejected by the handshake counter. Old data is rejected by the message counter window.
- The ID is tied to the public key by a hash.
- Sender ID, target ID, packet size and timing are visible to anyone listening.
- The hop count and packet ID are not authenticated, so an attacker can change the hop count of a copy of a packet.
- Jamming, handshake flooding and battery drain are limited by rate limits only.
- If a device's flash is wiped, its handshake counter restarts and contacts reject it until they add it again.
- A confirmation shows the node received the message. It does not show that a person read it.

## 13. Undecided

1. Maximum message size and memory limits per peer.
2. Whether relays cancel their send when they overhear the same packet.
3. Handshake retry rules, in the library or the application.
4. Number of confirmation copies and the gap between them, once airtime is known.
5. Test vectors for the handshake, key derivation and AES-CCM, and a test suite against a simulated lossy mesh.
