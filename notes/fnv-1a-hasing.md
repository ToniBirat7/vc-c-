To see exactly how the FNV-1a algorithm processes data, let's step through it at the bit level using a very short string: **`"Ab"`**.

We will trace this step-by-step using a 64-bit register.

---

### Step 0: The Starting State (The Offset Basis)

Before we read any characters, our `hash` variable is initialized to the 64-bit FNV Offset Basis:

- **Decimal:** `14695981039346656037`
- **Hexadecimal:** `0xcbf29ce484222325`
- **Binary (64 bits):**
  ```text
  11001011 11110010 10011100 11100100 10000100 00100010 00100011 00100101
  ```

---

### Step 1: Processing the First Character `'A'`

The ASCII value for `'A'` is `65` (or `0x41` in hex).
In binary, a single character is 8 bits (1 byte): `01000001`.

When we cast it to a 64-bit unsigned integer (`uint64_t`), it is padded with leading zeros:

```text
00000000 00000000 00000000 00000000 00000000 00000000 00000000 01000001
```

#### Step 1a: The XOR Operation (`hash ^= c`)

We align the 64-bit hash with the 64-bit version of `'A'` and perform a bitwise XOR.
_(Recall that XOR yields `1` if the bits are different, and `0` if they are the same)._

```text
  11001011 11110010 10011100 11100100 10000100 00100010 00100011 00100101  (Current Hash)
XOR
  00000000 00000000 00000000 00000000 00000000 00000000 00000000 01000001  (Character 'A')
--------------------------------------------------------------------------
  11001011 11110010 10011100 11100100 10000100 00100010 00100011 01100100  (New Hash)
                                                                 ^      ^
                                                             (Only these bits flipped)
```

At this point, only the last byte of our hash has changed. The other 56 bits remain completely identical to the starting state.

#### Step 1b: The Multiplication (`hash *= fnv_prime`)

Now we multiply this new hash value by the 64-bit FNV prime: `1099511628211` (`0x100000001b3` in hex).

In binary, multiplication is a series of shifts and additions. Because the prime is a massive number with many active bits scattered throughout, multiplying by it does two things:

1. **Shifts:** The bits we just modified at the bottom of the register are shifted (copied) to multiple different positions higher up in the 64-bit space.
2. **Overflow (Modulo $2^{64}$):** The multiplication will exceed the maximum value of a 64-bit register ($2^{64}-1$). In C++, unsigned integers automatically wrap around (overflow) without throwing an error [3]. This means any bits pushed past the 64th position are truncated (discarded).

This combination of shifting, adding, and truncating acts like a digital blender. After the multiplication, the simple change we made in the last byte is now scrambled across the entire 64-bit register.

Let's assume our new hash value after this multiplication becomes:

```text
01011010 11100001 00111100 ... [scrambled bits] ... 10011011 11001110
```

---

### Step 2: Processing the Second Character `'b'`

The ASCII value for `'b'` is `98` (or `0x62` in hex).
In binary (padded to 64-bit):

```text
00000000 00000000 00000000 00000000 00000000 00000000 00000000 01100010
```

#### Step 2a: The XOR Operation (`hash ^= c`)

We XOR this new character with our already-scrambled hash from the end of Step 1:

```text
  01011010 11100001 00111100 ... [scrambled] ... 10011011 11001110  (Hash from Step 1)
XOR
  00000000 00000000 00000000 ... [zeros]     ... 00000000 01100010  (Character 'b')
------------------------------------------------------------------
  01011010 11100001 00111100 ... [scrambled] ... 10011011 10101100  (New Hash)
                                                          ^ ^ ^
                                                    (Bits changed here)
```

#### Step 2b: The Multiplication (`hash *= fnv_prime`)

We multiply by the prime again. The new changes introduced by `'b'` are blended together with the existing scrambled pattern from `'A'`.

The bits cascade, wrap around, and we get our final, highly unique 64-bit hash.

---

### Why does FNV-1a XOR _before_ Multiplying?

There are two versions of this algorithm:

- **FNV-1:** Multiply first, then XOR.
- **FNV-1a:** XOR first, then multiply.

Your code uses **FNV-1a**. This is widely preferred because of how it handles the last byte.

If we multiplied first and then XORed (FNV-1), the very last character of our string would be introduced at the very end of the process via XOR. Because no multiplication happens after the final XOR, that last character's bits would only affect the lower bits of the final hash.

By XORing first and then multiplying (FNV-1a), we ensure that **every single character**—including the very last one—goes through the multiplication blender before the hash is finalized. This gives us a much better **avalanche effect** (where changing a single character at the end of a file completely changes the entire hash output).

These are three fundamental questions that strike at the very heart of cryptography, hashing, and database design. Let's break down each one from first principles.

---

### Question 1: How does FNV-1a ensure no duplicate hashes exist for different inputs?

The short answer is: **It mathematically cannot guarantee absolute zero duplicates.**

This limitation is defined by a fundamental rule in mathematics called the **Pigeonhole Principle**:

> _If you have 10 pigeonholes, and 11 pigeons, at least one pigeonhole must contain more than one pigeon._

In our 64-bit hashing system:

- The number of "pigeonholes" (possible hash values) is $2^{64}$ (about 18.4 quintillion).
- The number of "pigeons" (possible files we could create) is infinite.
- Because the input space is infinite and the output space is finite, duplicates (known as **hash collisions**) are mathematically guaranteed to exist.

#### How FNV-1a minimizes collisions

While collisions exist in theory, FNV-1a prevents them from happening in practice through **Uniform Distribution**.

If a hashing algorithm is poorly designed, many different inputs will cluster around the same few hash values. FNV-1a uses its prime multiplication to scatter the output values as widely and randomly as possible across the entire 18.4 quintillion range.

Because the outputs are distributed so evenly, the chance of two random files randomly landing on the exact same hash value is astronomically small—so small that you could run your version control system for lifetimes without ever encountering a natural collision.

_(Note: This is why Git uses SHA-1, a 160-bit hash, and is migrating to SHA-256 [4]. Cryptographic hashes like SHA-1/SHA-256 are even larger than FNV-1a, making it virtually impossible for even a malicious attacker to intentionally forge a file that produces a duplicate hash.)_

---

### Question 2: Why use huge integers?

Why use 64-bit integers instead of smaller ones like 8-bit or 32-bit? It comes down to the size of our "search space" and a mathematical phenomenon called the **Birthday Paradox**.

The Birthday Paradox states that in a room of just 23 random people, there is a 50% chance that two of them share the exact same birthday (even though there are 365 days in a year). We don't need 365 people to get a collision; we only need a fraction of that.

Let's look at how integer size changes our risk of collision:

1. **If we used an 8-bit integer (256 possible hashes):**
   - If you have just **20 files** in your repository, the probability of a collision is already over 50%. This is useless for version control.
2. **If we used a 32-bit integer (~4.29 billion possible hashes):**
   - If you have about **77,000 files** in your repository, you have a 50% chance of a collision. For large codebases (like the Linux kernel, which has over 1 million files), a 32-bit hash is far too small.
3. **If we use a 64-bit integer (~18.4 quintillion possible hashes):**
   - You would need to store about **5 billion files** in your repository before reaching a 50% chance of a collision.

By using large integers, we expand the number of possible outcomes so drastically that we practically eliminate accidental collisions.

---

### Question 3: How does it give the entire content of "Ab" back from the final hash? (Is it one-way?)

Your intuition is entirely correct: **Hashing is strictly a one-way function.**

There is absolutely no math that can reconstruct a file's content from its hash. If you write a 10-megabyte file, its FNV-1a hash is still only 8 bytes (64 bits). You cannot pack 10 megabytes of unique information into 8 bytes without losing data. The information is discarded during the multiplication overflow and truncation steps.

So, how does our version control system get the content back?

We do not use math to turn the hash back into data. Instead, we use the hash as a **lookup key (an address) on your hard drive**.

#### The Content-Addressable Storage (CAS) Loop:

1. **Saving (Hashing & Storing):**
   - We have the content: `"Ab"`.
   - We calculate the hash: `fnv1a_hash("Ab")` $\rightarrow$ returns `"d3b07384d113edec"`.
   - We create a file on disk: `.myvc/objects/d3b07384d113edec`.
   - We write the actual string `"Ab"` inside that file.
   - _The hash now acts as the address of that specific content._

2. **Retrieving (Looking up):**
   - Later, our tool looks at its manifest and sees: _"At version 1, `sandbox/bill1.txt` had the hash `d3b07384d113edec`."_
   - Our program does not perform any math to decode `"d3b07384d113edec"`.
   - Instead, it simply asks the operating system to read the file: `.myvc/objects/d3b07384d113edec`.
   - The OS opens the file, reads the raw bytes inside it (`"Ab"`), and writes those bytes back to `sandbox/bill1.txt`.

This is the core paradigm of Content-Addressable Storage: **The hash is the key, and the file on disk is the value.** It is exactly how Git's database works under the hood [1, 2].

Does this distinction between _reconstructing_ a hash mathematically (impossible) versus _looking up_ the content from disk using the hash as a filename make sense? If so, are you ready to write the C++ code to implement this lookup database?
