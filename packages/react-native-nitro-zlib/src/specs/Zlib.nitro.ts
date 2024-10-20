import type { HybridObject } from 'react-native-nitro-modules'

export type CompressionLevel = 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9

export type FlushMode =
  | 'NO_FLUSH'
  | 'PARTIAL_FLUSH'
  | 'SYNC_FLUSH'
  | 'FULL_FLUSH'
  | 'FINISH'
  | 'BLOCK'
  | 'TREES'

export interface Zlib extends HybridObject<{ ios: 'c++'; android: 'c++' }> {
  readonly version: string
  inflate(data: ArrayBuffer, flush?: FlushMode): ArrayBuffer
  inflateRaw(data: ArrayBuffer, flush?: FlushMode): ArrayBuffer
  compress(data: ArrayBuffer, level?: CompressionLevel): ArrayBuffer
  deflate(
    data: ArrayBuffer,
    level?: CompressionLevel,
    flush?: FlushMode
  ): ArrayBuffer
  deflateRaw(
    data: ArrayBuffer,
    level?: CompressionLevel,
    flush?: FlushMode
  ): ArrayBuffer
  gzip(data: ArrayBuffer, level?: CompressionLevel): ArrayBuffer
  gunzip(data: ArrayBuffer): ArrayBuffer
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
