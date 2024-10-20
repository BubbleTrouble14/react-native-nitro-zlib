# React Native Zlib

React Native implementation of [Bitcoin BIP39](https://github.com/bitcoin/bips/blob/master/bip-0039.mediawiki). It's written in TypeScript and has direct bindings to a Zlib C++ library. The primary function of this implementation is to provide mnemonic code for generating deterministic keys.

- 🏎️ Up to 800x faster then js solutions (mnemonicToSeed)
- ⚡️ Lightning fast implementation with pure C++ and JSI
- 📚 Standalone library
- 🧪 Tested in JS and C++ (OpenSSL)
- 💳 Made for crypto wallets

## Installation

To use this library in your React Native project, run the following commands:

Start by installing Nitro Modules. You can find more information [here](https://nitro.margelo.com/).

```sh
bun i react-native-nitro-modules
cd ios && pod install
```

Now install react-native-nitro-zlib

```sh
bun install react-native-nitro-zlib
```

## Example

```typescript
import { zlib } from "react-native-nitro-zlib";

zlib.setDefaultWordlist("english");
console.log(zlib.getDefaultWordlist());

const mnemonic = zlib.generateMnemonic(12);
console.log(mnemonic); // Outputs a 12-word mnemonic
console.log(zlib.validateMnemonic(mnemonic));
console.log(zlib.mnemonicToSeed(mnemonic));
console.log(zlib.mnemonicToSeedHex(mnemonic));
const entropy = zlib.mnemonicToEntropy(mnemonic);
console.log(entropy);
console.log(zlib.entropyToMnemonic(entropy));
```

## Documentation

### setDefaultWordlist

```typescript
setDefaultWordlist(wordlist: WordLists): void
```

- **Parameters:**
  - `wordlist` The word list to use (e.g., 'chinese_simplified', 'english', etc.). Default is 'english'.

### getDefaultWordlist

```typescript
getDefaultWordlist(): WordLists
```

- **Returns:** The word list to use (e.g., 'chinese_simplified', 'english', etc.). Default is 'english'.

### generateMnemonic

```typescript
generateMnemonic(wordCount?: WordCount, rng?: ArrayBuffer, wordlist?: WordLists): string
```

- **Parameters:**
  - `wordCount` (optional): Number of words in the mnemonic (e.g., 12, 15, 18, 21, or 24). Default is 12.
  - `rng` (optional): A random number generator represented as a `ArrayBuffer`.
  - `wordlist` (optional): The word list to use (e.g., 'chinese_simplified', 'english', etc.). Default is 'english'.
- **Returns:** A string representing the generated mnemonic.

### validateMnemonic

```typescript
validateMnemonic(mnemonic: string, wordlist?: WordLists): boolean
```

- **Parameters:**
  - `mnemonic`: The mnemonic phrase to validate.
  - `wordlist` (optional): The word list to use (e.g., 'chinese_simplified', 'english', etc.). Default is 'english'.
- **Returns:** A boolean indicating whether the mnemonic is valid.

### mnemonicToSeed

```typescript
mnemonicToSeed(mnemonic: string, password?: string): ArrayBuffer
```

- **Parameters:**
  - `mnemonic`: The mnemonic phrase to convert.
  - `password` (optional): An optional passphrase for additional security.
- **Returns:** A `ArrayBuffer` representing the binary seed.

### mnemonicToSeedHex

```typescript
mnemonicToSeedHex(mnemonic: string, password?: string): string
```

- **Parameters:**
  - `mnemonic`: The mnemonic phrase to convert.
  - `password` (optional): An optional passphrase for additional security.
- **Returns:** A string representing the hexadecimal seed.

### mnemonicToEntropy

```typescript
mnemonicToEntropy(mnemonic: string, wordlist?: WordLists): string
```

- **Parameters:**
  - `mnemonic`: The mnemonic phrase to convert.
  - `wordlist` (optional): The word list to use (e.g., 'chinese_simplified', 'english', etc.). Default is 'english'.
- **Returns:** A string representing the binary entropy.

### entropyToMnemonic

```typescript
entropyToMnemonic(entropy: string | ArrayBuffer, wordlist?: WordLists): string
```

- **Parameters:**
  - `entropy`: The binary entropy to convert.
  - `wordlist` (optional): The word list to use (e.g., 'chinese_simplified', 'english', etc.). Default is 'english'.
- **Returns:** A string representing the mnemonic phrase.

### WordLists

A type representing different word lists that can be used for generating or validating mnemonics.

```typescript
type WordLists =
  | "chinese_simplified"
  | "chinese_traditional"
  | "czech"
  | "english"
  | "french"
  | "italian"
  | "japanese"
  | "korean"
  | "portuguese"
  | "spanish";
```

### WordCount

A type representing the number of words in a mnemonic.

```typescript
type WordCount = 12 | 15 | 18 | 21 | 24;
```

## Tests

All test cases have been sourced from the [Python](https://github.com/trezor/python-mnemonic/blob/master/vectors.json) and [JavaScript](https://github.com/bitcoinjs/zlib/blob/master/test/index.js) implementations of BIP39. They have been subsequently adapted to be compatible with React Native.

## Resources

- [mrousavy/nitro](https://nitro.margelo.coma/) Nitro Modules
- [mattxlee/zlib_cpp](https://github.com/mattxlee/zlib_cpp) C++ Zlib library
- [margelo/react-native-worklets-core](https://github.com/margelo/react-native-worklets-core) Helpful for how JSI works
- [bitcoinjs/zlib](https://github.com/bitcoinjs/zlib/tree/master) Mostly used as zlib reference
