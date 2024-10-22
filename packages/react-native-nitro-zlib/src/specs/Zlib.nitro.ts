import type { HybridObject } from 'react-native-nitro-modules'

export interface ZlibOptions {
  flush?: number
  finishFlush?: number
  chunkSize?: number
  windowBits?: number
  level?: number
  memLevel?: number
  strategy?: number
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
