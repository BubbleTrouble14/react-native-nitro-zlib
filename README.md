# React Native Nitro Zlib

React Native implementation of Zlib compression library. It's written in TypeScript and has direct bindings to a Zlib C++ library. The primary function of this implementation is to provide fast and efficient compression and decompression capabilities.

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

// Compression
const data = new TextEncoder().encode("Hello, World!");
const compressed = zlib.deflate(data.buffer);
console.log("Compressed data:", compressed);

// Decompression
const decompressed = zlib.inflate(compressed);
console.log("Decompressed data:", new TextDecoder().decode(decompressed));

// Using streams
const deflateStream = zlib.createDeflateStream();
deflateStream.onData((chunk) => console.log("Received chunk:", chunk));
deflateStream.onEnd(() => console.log("Stream ended"));
deflateStream.write(data.buffer);
deflateStream.end();
```

## Documentation

### Zlib Interface

```typescript
interface Zlib {
  readonly version: string;
  inflate(data: ArrayBuffer, flush?: FlushMode): ArrayBuffer;
  inflateRaw(data: ArrayBuffer, flush?: FlushMode): ArrayBuffer;
  compress(data: ArrayBuffer, level?: CompressionLevel): ArrayBuffer;
  deflate(data: ArrayBuffer, level?: CompressionLevel, flush?: FlushMode): ArrayBuffer;
  deflateRaw(data: ArrayBuffer, level?: CompressionLevel, flush?: FlushMode): ArrayBuffer;
  gzip(data: ArrayBuffer, level?: CompressionLevel): ArrayBuffer;
  gunzip(data: ArrayBuffer): ArrayBuffer;
  createDeflateStream(level?: CompressionLevel, strategy?: number): ZlibStream;
  createInflateStream(): ZlibStream;
}
```

### ZlibStream Interface

```typescript
interface ZlibStream {
  write(chunk: ArrayBuffer): boolean;
  end(): void;
  flush(kind?: number): void;
  onData(callback: (chunk: ArrayBuffer) => void): void;
  onEnd(callback: () => void): void;
  onError(callback: (error: Error) => void): void;
  params(level: number, strategy: number): void;
  reset(): void;
  getMemorySize(): number;
}
```

### Types

```typescript
type CompressionLevel = 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9;

type FlushMode =
  | 'NO_FLUSH'
  | 'PARTIAL_FLUSH'
  | 'SYNC_FLUSH'
  | 'FULL_FLUSH'
  | 'FINISH'
  | 'BLOCK'
  | 'TREES';
```

## Methods

### inflate(data: ArrayBuffer, flush?: FlushMode): ArrayBuffer

Decompresses the given data.

### inflateRaw(data: ArrayBuffer, flush?: FlushMode): ArrayBuffer

Decompresses raw deflate data.

### compress(data: ArrayBuffer, level?: CompressionLevel): ArrayBuffer

Compresses the given data.

### deflate(data: ArrayBuffer, level?: CompressionLevel, flush?: FlushMode): ArrayBuffer

Compresses the given data using the deflate algorithm.

### deflateRaw(data: ArrayBuffer, level?: CompressionLevel, flush?: FlushMode): ArrayBuffer

Compresses the given data using the raw deflate algorithm.

### gzip(data: ArrayBuffer, level?: CompressionLevel): ArrayBuffer

Compresses the given data using the gzip format.

### gunzip(data: ArrayBuffer): ArrayBuffer

Decompresses gzip data.

### createDeflateStream(level?: CompressionLevel, strategy?: number): ZlibStream

Creates a new deflate stream.

### createInflateStream(): ZlibStream

Creates a new inflate stream.

## ZlibStream Methods

### write(chunk: ArrayBuffer): boolean

Writes a chunk of data to the stream.

### end(): void

Ends the stream.

### flush(kind?: number): void

Flushes the stream.

### onData(callback: (chunk: ArrayBuffer) => void): void

Sets a callback to be called when data is available.

### onEnd(callback: () => void): void

Sets a callback to be called when the stream ends.

### onError(callback: (error: Error) => void): void

Sets a callback to be called when an error occurs.

### params(level: number, strategy: number): void

Updates the compression parameters.

### reset(): void

Resets the stream.

### getMemorySize(): number

Returns the current memory usage of the stream.

## Resources

- [mrousavy/nitro](https://nitro.margelo.com/) Nitro Modules
- [zlib](https://zlib.net/) Zlib compression library
- [margelo/react-native-worklets-core](https://github.com/margelo/react-native-worklets-core) Helpful for how JSI works