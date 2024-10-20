import {
  zlib,
  type CompressionLevel,
  type FlushMode,
} from 'react-native-nitro-zlib'
import type { State } from './Testers'
import { it } from './Testers'
import { stringify } from './utils'

type TestResult =
  | {
      status: 'successful'
      result: string
    }
  | {
      status: 'failed'
      message: string
    }

export interface TestRunner {
  name: string
  run: () => Promise<TestResult>
}

function createTest<T>(
  name: string,
  run: () => State<T> | Promise<State<T>>
): TestRunner {
  return {
    name: name,
    run: async (): Promise<TestResult> => {
      try {
        console.log(`⏳ Test "${name}" started...`)
        const state = await run()
        console.log(`✅ Test "${name}" passed!`)
        return {
          status: 'successful',
          result: stringify(state.result ?? state.errorThrown ?? '(void)'),
        }
      } catch (e) {
        console.log(`❌ Test "${name}" failed! ${e}`)
        return {
          status: 'failed',
          message: stringify(e),
        }
      }
    },
  }
}

// Helper function to create ArrayBuffer from string
function stringToArrayBuffer(str: string): ArrayBuffer {
  return new TextEncoder().encode(str).buffer
}

// Helper function to convert ArrayBuffer to string
function arrayBufferToString(buffer: ArrayBuffer): string {
  return new TextDecoder().decode(buffer)
}

export function getTests(): TestRunner[] {
  return [
    createTest('Zlib version is available', () =>
      it(() => zlib.version)
        .didNotThrow()
        .didReturn('string')
    ),

    createTest('inflate works correctly', () => {
      const original = 'Hello, World!'
      const compressed = zlib.deflate(stringToArrayBuffer(original))
      return it(() => zlib.inflate(compressed))
        .didNotThrow()
        .didReturn('object')
        .custom((result) => arrayBufferToString(result) === original)
    }),

    createTest('inflateRaw works correctly', () => {
      const original = 'Raw inflated data'
      const compressed = zlib.deflateRaw(stringToArrayBuffer(original))
      return it(() => zlib.inflateRaw(compressed))
        .didNotThrow()
        .didReturn('object')
        .custom((result) => arrayBufferToString(result) === original)
    }),

    createTest('compress works correctly', () => {
      const original = 'Compress me!'
      const compressed = zlib.compress(stringToArrayBuffer(original))
      return it(() => zlib.inflate(compressed))
        .didNotThrow()
        .didReturn('object')
        .custom((result) => arrayBufferToString(result) === original)
    }),

    createTest('deflate works correctly', () => {
      const original = 'Deflate this data'
      const level: CompressionLevel = 6
      const flush: FlushMode = 'SYNC_FLUSH'
      return it(() => zlib.deflate(stringToArrayBuffer(original), level, flush))
        .didNotThrow()
        .didReturn('object')
        .custom(
          (result) => arrayBufferToString(zlib.inflate(result)) === original
        )
    }),

    createTest('deflateRaw works correctly', () => {
      const original = 'Raw deflate data'
      const level: CompressionLevel = 4
      const flush: FlushMode = 'FULL_FLUSH'
      return it(() =>
        zlib.deflateRaw(stringToArrayBuffer(original), level, flush)
      )
        .didNotThrow()
        .didReturn('object')
        .custom(
          (result) => arrayBufferToString(zlib.inflateRaw(result)) === original
        )
    }),

    createTest('gzip works correctly', () => {
      const original = 'Gzip compression test'
      const level: CompressionLevel = 9
      return it(() => zlib.gzip(stringToArrayBuffer(original), level))
        .didNotThrow()
        .didReturn('object')
        .custom(
          (result) => arrayBufferToString(zlib.gunzip(result)) === original
        )
    }),

    createTest('gunzip works correctly', () => {
      const original = 'Gunzip test data'
      const gzipped = zlib.gzip(stringToArrayBuffer(original))
      return it(() => zlib.gunzip(gzipped))
        .didNotThrow()
        .didReturn('object')
        .custom((result) => arrayBufferToString(result) === original)
    }),

    // New tests for every compression level
    ...[0, 1, 2, 3, 4, 5, 6, 7, 8, 9].map((level) =>
      createTest(
        `deflate works correctly with compression level ${level}`,
        () => {
          const original = `Deflate test data for level ${level}`
          return it(() =>
            zlib.deflate(
              stringToArrayBuffer(original),
              level as CompressionLevel
            )
          )
            .didNotThrow()
            .didReturn('object')
            .custom(
              (result) => arrayBufferToString(zlib.inflate(result)) === original
            )
        }
      )
    ),

    ...[0, 1, 2, 3, 4, 5, 6, 7, 8, 9].map((level) =>
      createTest(`gzip works correctly with compression level ${level}`, () => {
        const original = `Gzip test data for level ${level}`
        return it(() =>
          zlib.gzip(stringToArrayBuffer(original), level as CompressionLevel)
        )
          .didNotThrow()
          .didReturn('object')
          .custom(
            (result) => arrayBufferToString(zlib.gunzip(result)) === original
          )
      })
    ),

    // Test different flush modes for deflate
    ...(
      [
        'NO_FLUSH',
        'PARTIAL_FLUSH',
        'SYNC_FLUSH',
        'FULL_FLUSH',
        'FINISH',
        'BLOCK',
        'TREES',
      ] as FlushMode[]
    ).map((flushMode) =>
      createTest(`deflate works correctly with flush mode ${flushMode}`, () => {
        const original = `Deflate test data for flush mode ${flushMode}`
        return it(() =>
          zlib.deflate(stringToArrayBuffer(original), 6, flushMode)
        )
          .didNotThrow()
          .didReturn('object')
          .custom(
            (result) => arrayBufferToString(zlib.inflate(result)) === original
          )
      })
    ),

    ...[100, 1000, 10000, 100000, 1000000].map((size) => {
      return createTest(`compress and decompress ${size} bytes`, () => {
        const original = 'A'.repeat(size)
        return it(() => {
          const compressed = zlib.compress(stringToArrayBuffer(original))
          const decompressed = zlib.inflate(compressed)
          const result = arrayBufferToString(decompressed) === original
          return result
        })
          .didNotThrow()
          .didReturn('boolean')
      })
    }),
    // Test with random data
    createTest('compress and decompress random data', () => {
      const size = 100000 // 100KB of random data
      const original = Array.from({ length: size }, () =>
        String.fromCharCode(Math.floor(Math.random() * 256))
      ).join('')
      return it(() => {
        const compressed = zlib.compress(stringToArrayBuffer(original))
        const decompressed = zlib.inflate(compressed)
        const result = arrayBufferToString(decompressed) === original
        return result
      })
        .didNotThrow()
        .didReturn('boolean')
    }),

    // Modified test for compression ratio
    createTest('compression reduces data size for compressible content', () => {
      const original = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'.repeat(1000) // 26KB of repeating data
      return it(() => {
        const originalBuffer = stringToArrayBuffer(original)
        const compressed = zlib.compress(originalBuffer)
        return compressed.byteLength < originalBuffer.byteLength
      })
        .didNotThrow()
        .didReturn('boolean')
    }),

    // Test to find the maximum compressible data size
    createTest('find maximum compressible data size', () => {
      const testCompression = (size: number): boolean => {
        try {
          const data = 'A'.repeat(size)
          const compressed = zlib.compress(stringToArrayBuffer(data))
          const decompressed = zlib.inflate(compressed)
          return arrayBufferToString(decompressed) === data
        } catch {
          return false
        }
      }

      return it(() => {
        let low = 1000 // 1KB
        let high = 1000000 // 1MB
        while (low <= high) {
          const mid = Math.floor((low + high) / 2)
          if (testCompression(mid)) {
            low = mid + 1
          } else {
            high = mid - 1
          }
        }
        console.log(`Maximum compressible data size: ${high} bytes`)
        return high
      })
        .didNotThrow()
        .didReturn('number')
    }),

    // Test compression with random data
    createTest('compress works correctly with random data', () => {
      const generateRandomString = (length: number) => {
        return Array.from({ length }, () =>
          String.fromCharCode(Math.floor(Math.random() * 256))
        ).join('')
      }
      const original = generateRandomString(10000) // 10KB of random data
      return it(() => {
        const compressed = zlib.compress(stringToArrayBuffer(original))
        const decompressed = zlib.inflate(compressed)
        return arrayBufferToString(decompressed) === original
      })
        .didNotThrow()
        .didReturn('boolean')
    }),

    // Test compression efficiency for different types of data
    createTest('compare compression efficiency', () => {
      const testEfficiency = (data: string) => {
        const originalBuffer = stringToArrayBuffer(data)
        const compressed = zlib.compress(originalBuffer)
        return (compressed.byteLength / originalBuffer.byteLength) * 100
      }

      return it(() => {
        const repeatingData = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'.repeat(1000)
        const randomData = Array.from({ length: 26000 }, () =>
          String.fromCharCode(Math.floor(Math.random() * 256))
        ).join('')

        const repeatingEfficiency = testEfficiency(repeatingData)
        const randomEfficiency = testEfficiency(randomData)

        return { repeatingEfficiency, randomEfficiency }
      })
        .didNotThrow()
        .didReturn('object')
    }),
  ]
}
