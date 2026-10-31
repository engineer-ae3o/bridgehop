# Mesh Messaging Protocol Specification (v1 draft, updated)

This document describes the protocol library. It does not depend on any particular hardware or radio. A second document, the Device Specification, covers the firmware, the radio, Bluetooth and storage.

The words MUST, SHOULD and MAY are used in their usual specification sense. MUST means required. SHOULD means strongly recommended. MAY means optional.

## 1. Scope

This document covers:

- the packet formats
- the cryptography
- sessions and handshakes
- relaying and duplicate suppression
- delivery confirmation
- what the library needs from the platform it runs on

This document does not cover:

- Bluetooth
- flash storage layout
- radio settings such as spreading factor and bandwidth
- duty cycle enforcement
- anything a user sees on screen

The goals are:

- long range
- messages that hop through other nodes
- end-to-end encryption
- forward secrecy
- sender authentication
- a confirmation that a message arrived

Delivery is "fire and forget", like UDP. The protocol never retries a message on its own. If a message is not confirmed, the application decides whether to send it again.

## 2. Node roles

There are two kinds of nodes.

**Passive node.** It only relays. It has no identity and no keys. When it hears a packet it has not seen before, it lowers the hop count, waits a random time and sends the packet again.

**Active node.** It has an identity. It reads packets addressed to it and relays everything else, exactly like a passive node. An active node keeps relaying other people's packets even while it is busy with its own messages.

There is no routing table. Every packet is flooded: every node that hears it sends it on, and the hop count and duplicate cache make sure it dies out.

## 3. Cryptography used

| Purpose | Algorithm |
|---|---|
| Signatures | Ed25519. Seed 32 bytes, public key 32 bytes, signature 64 bytes (R is 32, S is 32) |
| Key agreement | X25519, with a new key pair for every handshake |
| Key derivation | HKDF with SHA-256 |
| Encryption and authentication | AES-128-CCM, 12 byte nonce, 16 byte tag |
| Hashing | SHA-256 |

Every number longer than one byte is stored big-endian (most significant byte first).

The authentication tag is 16 bytes in this draft.

## 4. Identity

- On first boot an active node creates a random 32 byte **seed**. The seed is the only secret that has to be saved.
- Ed25519 turns the seed into a private scalar and a public key. The public key is worked out once at boot and kept in memory. It does not need to be saved.
- The node **ID** is the first 16 bytes of SHA-256(public key), in the order the hash produces them. This 16 byte ID is what users give each other, like a phone number.
- A node MAY save its ID. If it does, it SHOULD recompute the ID at boot and compare, to catch damaged flash.
- An ID alone is not enough to check a signature. The full public key arrives inside the first handshake and is checked against the ID by hashing.
- Contacts are mutual. Both people must have added each other. An active node MUST ignore a handshake from any ID that is not in its contact list. The device decides how many contacts it holds (default 500).
- An ID of all zero bytes is reserved for a future broadcast feature. Version 1 nodes MUST NOT send to it.
- Changing an ID without a full reset is not supported in version 1.

## 5. Packet formats

The largest packet is set by the transport. For LoRa it is 255 bytes. The sizes below assume 255.

### 5.1 Header (41 bytes, always in the clear)

| Offset | Size | Field |
|---|---|---|
| 0 | 1 | type, version and hop count (see below) |
| 1 | 8 | packet ID |
| 9 | 16 | sender ID |
| 25 | 16 | target ID |

The first byte is split into three parts:

- bits 7 and 6: packet type
  - 00 = regular data message
  - 01 = handshake
  - 10 = delivery confirmation
  - 11 = reserved, drop it
- bits 5 and 4: protocol version (this document is version 1)
- bits 3 to 0: hop count (also called TTL)

Rules for the header:

- The hop count goes from 0 to 10. The starting value is configurable (default 10). A received value above 10 is treated as corrupted and the packet is dropped.
- A relay changes only the hop count bits. It MUST leave the type and version bits as they were.
- A packet with an unknown version or the reserved type is dropped and not relayed.
- The packet ID is the first 8 bytes of SHA-256(sender ID, then target ID, then the whole payload). It is based on the content, so the same packet always gets the same ID.
- Every node MUST recompute the packet ID and drop the packet if it does not match. This stops an attacker poisoning the duplicate cache with fake IDs.
- The packet length is not in the header. The transport reports how many bytes arrived. The radio CRC (or the UART adapter's own check) makes sure damaged packets never reach the protocol.
- The first byte and the packet ID are not protected by the session. The sender ID and target ID are protected (see 5.5).

### 5.2 Handshake payload (144 bytes, 185 bytes in total)

| Size | Field |
|---|---|
| 32 | ephemeral X25519 public key |
| 16 | freshness value: 8 byte handshake counter, then 8 random bytes |
| 32 | signature R |
| 32 | signature S |
| 32 | sender's Ed25519 public key |

The handshake counter is a number the device saves in flash and only ever increases. Each receiver remembers the highest value it has seen from each contact and rejects anything that is not higher. This blocks replay of old handshakes and needs no clock.

What gets signed, in order:

1. the text `BH1-HS`
2. sender ID
3. target ID
4. ephemeral public key
5. the sender's own freshness value
6. the other side's freshness value

For the first message of a handshake (the initiation), item 6 is 16 zero bytes. For the reply (the response), item 6 is the freshness value from the initiation. This ties a reply to one particular initiation.

The receiver checks all of these:

- SHA-256(public key) starts with the sender ID
- the sender is in the contact list
- the target ID is mine
- the handshake counter is higher than the last one seen from that sender
- the signature is valid

### 5.3 Data payload (22 bytes plus ciphertext)

| Size | Field |
|---|---|
| 16 | tag |
| 4 | message counter |
| 1 | total number of chunks (1 to 255) |
| 1 | chunk index (starts at 0; 0 when there is only one chunk) |
| rest | ciphertext chunk |

Every data packet costs 63 bytes of overhead (41 header plus 22). With a 255 byte packet, each chunk carries up to 192 bytes of ciphertext.

Chunk size is the packet size minus 63. Every chunk except the last MUST be exactly that size. The last chunk is between 1 byte and a full chunk. A receiver can therefore place a chunk at position `index × chunk size` without caring about arrival order.

The tag, counter and total are the same in every chunk of one message.

### 5.4 Confirmation payload (24 bytes, 65 bytes in total)

| Size | Field |
|---|---|
| 16 | tag |
| 4 | counter (the confirming node's own next send counter) |
| 4 | acked counter (the counter of the message being confirmed) |

There is no ciphertext. The tag is made by running AES-CCM over an empty message, so this packet proves who sent it and what it refers to, and nothing more.

### 5.5 What is authenticated

Data packets: the tag covers (in this order) the type value as one byte, sender ID, target ID, message counter and total chunks. The chunk index is deliberately not included. The whole message is encrypted once and has one tag, so the tag cannot depend on which chunk carries it. The index is still protected in practice. If someone changes an index, the chunks are put back together in the wrong order, and the tag check fails.

Confirmation packets: the tag covers the type value, sender ID, target ID, counter and acked counter.

Handshakes: the signature covers the sender ID and target ID (see 5.2).

The result is that the sender and target IDs are public but cannot be changed without detection. Nobody can redirect a packet to a different sender or target. Including the type value stops anyone turning one kind of packet into another by flipping the unprotected type bits.

## 6. Relaying and duplicate suppression

Every node keeps a duplicate cache of packet IDs.

| Setting | Default |
|---|---|
| Cache size | 100 entries (configurable; this is an implementation detail) |
| Entry lifetime | 120 seconds since the packet was last heard |
| When full | remove the oldest entry |
| Delay before relaying | random, 0 to 250 ms (configurable) |

When a packet arrives, every node does these steps in this order:

1. Check the version, the type, the hop count (not above 10) and that the packet ID matches the content. Drop it if anything is wrong.
2. Look the packet ID up in the cache. If it is there, drop the packet. This applies to every node, including the packet's target. (Optionally refresh the entry's time.)
3. Add the packet ID to the cache.
4. If this is an active node and the target ID is its own ID, process the packet and stop. Do not relay it.
5. Otherwise, if the hop count is 0, drop the packet. If not, lower the hop count by one, wait a random delay, and send the packet again.

Optional improvement: if a node hears the same packet from someone else while it is waiting to relay, it MAY cancel its own relay.

Notes:

- Packet IDs are 64 bits and based on content, so two different packets almost never share an ID.
- The cache does not protect against replays of packets addressed to me, because entries expire. Replays are stopped by the handshake counter and the message counter window (sections 7 and 8).
- The hop count is not protected. An attacker can send a copy of a packet with a low hop count near a relay, so the relay remembers the ID and then ignores the real packet. This is a known limitation. The cap of 10 limits other damage.

## 7. Sessions

An active node can have several sessions at once, one per peer. The maximum number is a setting.

A session holds:

- the peer's ID and public key
- a send key and send IV
- a receive key and receive IV
- the next send counter
- a window of recently seen receive counters
- the time of last activity
- the session state

### 7.1 States

Initiator: no session, then pending, then established.

Responder: no session, then tentative, then established when the first valid data packet from the initiator arrives and passes its tag check.

### 7.2 Handshake steps

1. Before starting a handshake, a node looks for an established session with the target. If one exists, it uses it and does not start a handshake.
2. The initiator makes a new X25519 key pair and a new freshness value (increasing the saved handshake counter first), signs, and sends a handshake packet. It is now pending.
3. The responder checks the handshake as in 5.2. If it is good, it makes its own key pair, works out the keys, and sends a signed handshake back. The signature includes the initiator's freshness value. The session is now tentative.
4. The initiator checks the reply (including that it matches its own freshness value), works out the keys, and the session is established.
5. The responder's session becomes established when the first data packet from the initiator decrypts and passes its tag check.

### 7.3 Key derivation

1. shared secret = X25519(my private key, their public key). Reject an all-zero result.
2. HKDF extract: salt is the initiator's freshness value followed by the responder's freshness value. The input key material is the shared secret.
3. HKDF expand: the info text is `BH1-KEYS`, then the initiator ID, the responder ID, the initiator's ephemeral public key and the responder's ephemeral public key. Ask for 56 bytes.
4. Split the 56 bytes into: initiator-to-responder key (16), initiator-to-responder IV (12), responder-to-initiator key (16), responder-to-initiator IV (12).
5. Each side sends with its own direction's key and receives with the other.
6. Erase the ephemeral private key as soon as the keys exist.

Nonce for every encrypted or authenticated packet: the 12 byte IV combined (XOR) with the counter placed in the last 4 bytes (the first 8 bytes of the counter side are zero). Nothing extra is sent for the nonce. The counter in the packet is all the receiver needs.

Counters:

- Each direction has its own counter, starting at 0.
- A data message uses one counter, however many chunks it has.
- A confirmation packet uses one counter too. Data messages and confirmations from one node share the same counter, so counters never repeat under one key.
- A node MUST start a new handshake before its counter reaches 4,294,967,295 (2 to the power 32, minus 1).
- A counter MUST NEVER be used twice with the same key.

### 7.4 Replacing a session

If a contact sends a handshake that passes every check, it replaces any existing session with that contact. This handles a peer that rebooted and lost its keys.

1. Keep the old session until the new handshake has passed all checks.
2. Then swap in one step: new keys and IVs, both counters back to 0, the receive window cleared.
3. Throw away any half-assembled messages from that peer and tell the application.
4. Throw away any confirmations still being waited for on the old session and report them as failed.

Messages sent under the old keys will fail their tag check. That is accepted.

### 7.5 Two sides starting at once

If I have a pending handshake to X and a handshake arrives from X:

1. First try it as a reply to my pending handshake. If it checks out, finish normally.
2. If not, it is a competing initiation. The side with the lower ID stays the initiator. If my ID is lower, I ignore theirs. If my ID is higher, I drop mine and answer theirs.

### 7.6 Timeouts

| Timer | Default | Meaning |
|---|---|---|
| Handshake timeout | 60 seconds | A pending or tentative session that has not completed is dropped. The initiator reports the failure upward. Retry is the caller's choice. |
| Session idle timeout | 1 hour | If no valid data or confirmation passes in either direction, the session is closed and its keys erased. The next message starts a new handshake. |
| Reassembly timeout | 45 seconds (allowed range 30 to 60) | Starts when the first chunk of a message arrives. If the message is not complete, it is thrown away and the application is told. |
| Confirmation wait | 120 seconds | See section 9. |

Handshakes cost a lot of processing. Nodes MUST limit how many handshakes they handle per sender and in total, because unsigned junk can be sent in handshake form.

## 8. Sending and receiving data

**Sending.**

1. Encrypt the whole message once with AES-CCM, using the send key, the nonce from the next counter, and the data authentication fields from 5.5.
2. Split the ciphertext into chunks of the chunk size.
3. Send each chunk as its own packet with the same tag, counter and total, and its own index.

The largest message is the number of chunks allowed times the chunk size. The default is 16 chunks (about 3 KB). The absolute limit is 255 chunks.

**Receiving.**

1. Find the session by sender ID.
2. Reject a counter that is older than the window or already marked in it (the window holds the last 64 counters).
3. Collect chunks by (peer, counter). Check that the total, index and size follow the rules in 5.3. If a later chunk has a different tag or total than the first one, drop that chunk.
4. When every chunk is present, join them, decrypt and check the tag.
5. Only after the tag passes: mark the counter as used, refresh the session timer, give the message to the application, and send a confirmation (section 9).

A failed tag never changes any state.

If chunks are missing when the reassembly timeout ends, the buffer is freed and the application is told the message was incomplete. Nothing is asked to be re-sent. Each peer and the whole node have a limit on how many messages can be assembled at once, to keep memory use bounded.

## 9. Delivery confirmation

A confirmation tells the sender one thing: the target node received every chunk, put them together, decrypted the message and the tag was valid. It does not mean a person has read it or that the phone app has it.

**What the receiver does.**

1. Only after step 5 of section 8 succeeds, send a confirmation to the sender.
2. Send it more than once, because the confirmation itself is not confirmed. Defaults: 3 copies in total, with a random gap of 1 to 5 seconds between copies.
3. Each copy MUST use a fresh send counter. This gives each copy a different tag and packet ID, so relays do not discard the later copies as duplicates. The acked counter field stays the same in every copy.
4. Do not send a confirmation for a duplicate, a replay or a message that failed any check.
5. Never confirm a confirmation.

**What the sender does.**

1. After the last chunk has actually been sent on the radio, start a timer of 120 seconds (configurable). The timer starts at transmission, so any delay for duty-cycle limits does not use it up.
2. Remember each message waiting for confirmation by (peer, counter).
3. When a valid confirmation arrives with a matching acked counter, report the message as confirmed. A confirmation also passes the normal counter window check and refreshes the session timer.
4. Later copies of the same confirmation are ignored. They are valid but change nothing.
5. If the timer ends, report the message as failed and unconfirmed. The application and a user decide whether to send again.

Things to know:

- A message can arrive even if it is reported as unconfirmed, if every confirmation copy was lost. The application should say "not confirmed", never "not delivered".
- Sending the message again uses a new counter. The receiver may see the same text twice. Removing duplicate text is the application's job.
- At slow radio settings, 120 seconds may be too short for a long message over several hops. Pick the value from the real airtime once the radio settings are chosen.
- Limit how many messages can wait for confirmation at once.

## 10. What the platform must provide

The library does no input or output on its own. It is written in C++23 and takes these as concepts (or tables of function pointers).

| Piece | What it must do |
|---|---|
| Transport | Send a packet and return true or false. Report the largest packet size. Received packets are passed to the library with optional signal information. |
| Clock | Return a steadily increasing time in milliseconds |
| Scheduler | Run a callback after a delay, and cancel it (or give the library a regular tick) |
| Random numbers | Fill a buffer with secure random bytes |
| Crypto | SHA-256, HKDF-SHA256, Ed25519 (public key from seed, sign, verify), X25519 (make key, agree), AES-128-CCM (seal and open), and a way to wipe memory |
| Identity and contacts | Load and save the seed. Look up a contact by ID. Read and write the last handshake counter seen from a contact. Provide the next handshake counter for this device. |
| Event sink | Receives: message received, message incomplete, session state change, send result (confirmed, unconfirmed or failed) |

Settings chosen when the library is built or started: node role, tag length, starting hop count, cache size, relay delay, confirmation copies and gap, all timeouts, maximum sessions, maximum chunks, maximum pending confirmations.

A passive-only build needs only the transport, clock, scheduler, random numbers and SHA-256.

The library is single-threaded. The platform must make sure calls into it do not overlap. Avoiding memory allocation after startup is recommended.

## 11. Errors and drops

These are dropped silently, with no reply, so nobody can use replies to probe a node: unknown version, reserved type, hop count above 10, wrong packet ID, unknown contact, bad signature, bad tag, replayed counter. Each one adds to a debug counter.

These are reported to the application: a message could not be sent because no session could be set up, a message was incomplete, a message went unconfirmed, a session closed, a message was too large.

## 12. Security summary and limits

What the protocol gives you:

- Messages are encrypted and tamper-evident. Keys are new for every session, so recorded traffic stays safe even if a long-term key leaks later (forward secrecy).
- Senders are authenticated by signature over the handshake.
- Old handshakes cannot be replayed (handshake counter). Old data cannot be replayed (counter window).
- The ID is tied to the public key by a hash.

What it does not give you:

- Metadata is public. Anyone listening sees who is talking to whom, packet sizes and timing.
- The hop count and packet ID are not protected, so an attacker can change the hop count.
- Jamming, flooding with handshakes and draining batteries can only be limited, not prevented.
- If a device's flash is wiped, its handshake counter restarts and contacts will reject it until they add it again. This is intended.
- Trust in an ID is only as good as the way it was shared.
- A confirmation proves the node got the message, not that a person read it.

## 13. Changes in this update

- Packet type is now 2 bits (data, handshake, confirmation, reserved). Version is 2 bits. Hop count stays 4 bits.
- Delivery confirmation packet added, with repeats and a 120 second wait.
- Every node checks the duplicate cache first, including the target.
- Duplicate cache default is now 100 entries.
- Session idle timeout is now 1 hour.
- The type value is now part of the data authentication fields.

Earlier changes still in effect: no length field, nonce built from IV and counter, one tag per message with the index left out of it, packet ID covers sender, target and payload, handshake freshness value, replace-on-new-handshake, separate 60 second handshake timeout.

## 14. Still undecided

1. Test vectors for the handshake, key derivation and AES-CCM, and a test suite that runs against a simulated lossy mesh.
2. Maximum message size and memory limits per peer.
3. Whether relays cancel their send when they overhear the same packet.
4. Broadcast (it would need a signed but unencrypted mode).
5. Handshake retry rules and whether they live in the library or the application.
6. How many confirmation copies and what gap, once airtime is known.
