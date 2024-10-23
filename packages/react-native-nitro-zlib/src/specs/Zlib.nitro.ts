import type { HybridObject } from 'react-native-nitro-modules'

/** Compression levels for zlib operations */
export const ZlibCompressionLevel = {
  DEFAULT: -1,
  NO_COMPRESSION: 0,
  BEST_SPEED: 1,
  BEST_COMPRESSION: 9,
} as const

/** Flush modes for zlib operations */
export const ZlibFlush = {
  NO_FLUSH: 0,
  PARTIAL_FLUSH: 1,
  SYNC_FLUSH: 2,
  FULL_FLUSH: 3,
  FINISH: 4,
  BLOCK: 5,
  TREES: 6,
} as const

export const ZlibStrategy = {
  DEFAULT_STRATEGY: 0,
  FILTERED: 1,
  HUFFMAN_ONLY: 2,
  RLE: 3,
  FIXED: 4,
} as const

// Type definitions for the constants
export type ZlibCompressionLevel =
  | (typeof ZlibCompressionLevel)[keyof typeof ZlibCompressionLevel]
  | 2
  | 3
  | 4
  | 5
  | 6
  | 7
  | 8
export type ZlibFlush = (typeof ZlibFlush)[keyof typeof ZlibFlush]
export type ZlibStrategy = (typeof ZlibStrategy)[keyof typeof ZlibStrategy]

export interface ZlibOptions {
  flush?: ZlibFlush
  finishFlush?: number
  chunkSize?: number
  windowBits?: number
  level?: ZlibCompressionLevel
  memLevel?: number
  strategy?: ZlibStrategy
  dictionary?: ArrayBuffer
  info?: boolean
  maxOutputLength?: number
}

export interface ZlibStream
  extends HybridObject<{ ios: 'c++'; android: 'c++' }> {
  write(chunk: ArrayBuffer): boolean
  end(): void
  flush(kind?: number): void

  onData(callback: (chunk: ArrayBuffer) => void): void
  onEnd(callback: () => void): void
  onError(callback: (error: Error) => void): void

  params(level: number, strategy: number): void
  reset(): void

  getMemorySize(): number
}

export interface Zlib extends HybridObject<{ ios: 'c++'; android: 'c++' }> {
  readonly version: string

  // Sync methods
  inflateSync(data: ArrayBuffer, options?: ZlibOptions): ArrayBuffer
  inflateRawSync(data: ArrayBuffer, options?: ZlibOptions): ArrayBuffer
  compressSync(data: ArrayBuffer, options?: ZlibOptions): ArrayBuffer
  deflateSync(data: ArrayBuffer, options?: ZlibOptions): ArrayBuffer
  deflateRawSync(data: ArrayBuffer, options?: ZlibOptions): ArrayBuffer
  gzipSync(data: ArrayBuffer, options?: ZlibOptions): ArrayBuffer
  gunzipSync(data: ArrayBuffer, options?: ZlibOptions): ArrayBuffer

  // Async methods
  inflate(data: ArrayBuffer, options?: ZlibOptions): Promise<ArrayBuffer>
  inflateRaw(data: ArrayBuffer, options?: ZlibOptions): Promise<ArrayBuffer>
  compress(data: ArrayBuffer, options?: ZlibOptions): Promise<ArrayBuffer>
  deflate(data: ArrayBuffer, options?: ZlibOptions): Promise<ArrayBuffer>
  deflateRaw(data: ArrayBuffer, options?: ZlibOptions): Promise<ArrayBuffer>
  gzip(data: ArrayBuffer, options?: ZlibOptions): Promise<ArrayBuffer>
  gunzip(data: ArrayBuffer, options?: ZlibOptions): Promise<ArrayBuffer>

  //Stream
  createDeflateStream(options?: ZlibOptions): ZlibStream
  createInflateStream(options?: ZlibOptions): ZlibStream
  createGzipStream(options?: ZlibOptions): ZlibStream
  createGunzipStream(options?: ZlibOptions): ZlibStream
  createDeflateRawStream(options?: ZlibOptions): ZlibStream
  createInflateRawStream(options?: ZlibOptions): ZlibStream
  createUnzipStream(options?: ZlibOptions): ZlibStream
}
