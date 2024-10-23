import type { State } from './Testers'
import { it } from './Testers'
import { stringify } from './utils'
import {
  ZlibCompressionLevel,
  ZlibFlush,
  ZlibStrategy,
  type Zlib,
  type ZlibOptions,
  type ZlibStream,
} from 'react-native-nitro-zlib'

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

// Helper functions
function stringToArrayBuffer(str: string): ArrayBuffer {
  return new TextEncoder().encode(str).buffer
}

function arrayBufferToString(buffer: ArrayBuffer): string {
  return new TextDecoder().decode(buffer)
}

function generateTestData(size: number = 1000): string {
  return 'Hello, this is test data! '.repeat(size)
}

async function testStream(
  stream: ZlibStream,
  input: ArrayBuffer
): Promise<ArrayBuffer> {
  return new Promise((resolve, reject) => {
    const chunks: ArrayBuffer[] = []

    stream.onData((chunk) => chunks.push(chunk))
    stream.onError(reject)
    stream.onEnd(() => {
      const totalLength = chunks.reduce(
        (acc, chunk) => acc + chunk.byteLength,
        0
      )
      const result = new Uint8Array(totalLength)
      let offset = 0
      chunks.forEach((chunk) => {
        result.set(new Uint8Array(chunk), offset)
        offset += chunk.byteLength
      })
      resolve(result.buffer)
    })

    stream.write(input)
    stream.end()
  })
}

async function testStreamPair(
  compressStream: ZlibStream,
  decompressStream: ZlibStream,
  input: ArrayBuffer
): Promise<boolean> {
  const compressed = await testStream(compressStream, input)
  const decompressed = await testStream(decompressStream, compressed)
  const original = arrayBufferToString(input)
  const result = arrayBufferToString(decompressed)
  return original === result
}

export function getTests(zlib: Zlib): TestRunner[] {
  const testOptions: ZlibOptions[] = [
    {},
    { level: 9 },
    { chunkSize: 16384 },
    { level: 9, strategy: 0, windowBits: 15 },
  ]

  // console.log(zlib.version)

  return [
    // Version test
    createTest('Zlib version is available', () => {
      return it(() => zlib.version)
        .didNotThrow()
        .didReturn('string')
    }),

    // Sync method tests
    ...testOptions.map((options, index) =>
      createTest(
        `inflateSync/deflateSync with options set ${index}`,
        async () => {
          const original = generateTestData()
          const originalBuffer = stringToArrayBuffer(original)

          return it(async () => {
            const compressed = zlib.deflateSync(originalBuffer, options)
            const decompressed = zlib.inflateSync(compressed, options)
            return arrayBufferToString(decompressed) === original
          })
        }
      )
    ),

    // Async method tests
    ...testOptions.map((options, index) =>
      createTest(
        `async inflate/deflate with options set ${index}`,
        async () => {
          const original = generateTestData()
          const originalBuffer = stringToArrayBuffer(original)

          return it(async () => {
            const compressed = await zlib.deflate(originalBuffer, {
              ...options,
            })
            const decompressed = await zlib.inflate(compressed, options)
            return arrayBufferToString(decompressed) === original
          })
        }
      )
    ),

    // Stream tests
    createTest('deflate stream basic functionality', async () => {
      const original = generateTestData()
      const originalBuffer = stringToArrayBuffer(original)

      return it(async () => {
        const stream = zlib.createDeflateStream()
        const compressed = await testStream(stream, originalBuffer)
        const decompressed = await zlib.inflate(compressed)
        return arrayBufferToString(decompressed) === original
      })
    }),

    // Error handling tests
    createTest('handles empty input correctly', async () => {
      const emptyBuffer = new ArrayBuffer(0)

      return it(async () => {
        const compressed = await zlib.compress(emptyBuffer)
        const decompressed = await zlib.inflate(compressed)
        return decompressed.byteLength === 0
      })
    }),

    createTest('handles invalid input', async () => {
      const invalidBuffer = new ArrayBuffer(10)

      return it(async () => {
        try {
          await zlib.inflate(invalidBuffer)
          return false
        } catch (error) {
          return error instanceof Error
        }
      })
    }),

    // Compression level tests
    ...[0, 3, 6, 9].map((level) =>
      createTest(`compression level ${level} works correctly`, async () => {
        const original = generateTestData()
        const originalBuffer = stringToArrayBuffer(original)

        return it(async () => {
          const compressed = await zlib.compress(originalBuffer, { level })
          const decompressed = await zlib.inflate(compressed)
          return arrayBufferToString(decompressed) === original
        })
      })
    ),

    // Stream types test
    createTest('all stream types initialize correctly', async () => {
      return it(() => {
        const streams = [
          zlib.createDeflateStream(),
          zlib.createInflateStream(),
          zlib.createGzipStream(),
          zlib.createGunzipStream(),
          zlib.createDeflateRawStream(),
          zlib.createInflateRawStream(),
          zlib.createUnzipStream(),
        ]

        return streams.every(
          (stream) =>
            stream &&
            typeof stream.write === 'function' &&
            typeof stream.end === 'function'
        )
      })
    }),

    // Performance test
    createTest('compression performance', async () => {
      const testSize = 100000
      const data = generateTestData(testSize)
      const buffer = stringToArrayBuffer(data)

      return it(async () => {
        const start = performance.now()
        const compressed = await zlib.compress(buffer)
        const end = performance.now()

        return JSON.stringify({
          originalSize: buffer.byteLength,
          compressedSize: compressed.byteLength,
          ratio: compressed.byteLength / buffer.byteLength,
          timeMs: end - start,
        })
      })
    }),

    // Comprehensive stream tests
    createTest('deflate/inflate stream pair works correctly', async () => {
      const original = generateTestData()
      const originalBuffer = stringToArrayBuffer(original)

      return it(async () => {
        const deflateStream = zlib.createDeflateStream()
        const inflateStream = zlib.createInflateStream()
        return testStreamPair(deflateStream, inflateStream, originalBuffer)
      })
    }),

    createTest('gzip/gunzip stream pair works correctly', async () => {
      const original = generateTestData()
      const originalBuffer = stringToArrayBuffer(original)

      return it(async () => {
        const gzipStream = zlib.createGzipStream()
        const gunzipStream = zlib.createGunzipStream()
        return testStreamPair(gzipStream, gunzipStream, originalBuffer)
      })
    }),

    createTest(
      'deflateRaw/inflateRaw stream pair works correctly',
      async () => {
        const original = generateTestData()
        const originalBuffer = stringToArrayBuffer(original)

        return it(async () => {
          const deflateRawStream = zlib.createDeflateRawStream()
          const inflateRawStream = zlib.createInflateRawStream()
          return testStreamPair(
            deflateRawStream,
            inflateRawStream,
            originalBuffer
          )
        })
      }
    ),

    createTest('stream with different chunk sizes', async () => {
      const original = generateTestData(10000) // Larger test data
      const originalBuffer = stringToArrayBuffer(original)

      return it(async () => {
        const results = await Promise.all(
          [1024, 4096, 16384].map(async (chunkSize) => {
            const deflateStream = zlib.createDeflateStream({ chunkSize })
            const inflateStream = zlib.createInflateStream({ chunkSize })
            return testStreamPair(deflateStream, inflateStream, originalBuffer)
          })
        )
        return results.every((result) => result === true)
      })
    }),

    // Test different windowBits values
    ...[8, 9, 15].map((windowBits) =>
      createTest(`windowBits=${windowBits} works correctly`, async () => {
        const original = generateTestData()
        const originalBuffer = stringToArrayBuffer(original)

        return it(async () => {
          const compressed = await zlib.deflate(originalBuffer, { windowBits })
          const decompressed = await zlib.inflate(compressed, { windowBits })
          return arrayBufferToString(decompressed) === original
        })
      })
    ),

    // Test different memLevel values
    ...[1, 4, 8, 9].map((memLevel) =>
      createTest(`memLevel=${memLevel} works correctly`, async () => {
        const original = generateTestData()
        const originalBuffer = stringToArrayBuffer(original)

        return it(async () => {
          const compressed = await zlib.deflate(originalBuffer, { memLevel })
          const decompressed = await zlib.inflate(compressed)
          return arrayBufferToString(decompressed) === original
        })
      })
    ),

    // Test all strategy types
    ...Object.values(ZlibStrategy).map((strategy) =>
      createTest(`strategy=${strategy} works correctly`, async () => {
        const original = generateTestData()
        const originalBuffer = stringToArrayBuffer(original)

        return it(async () => {
          const compressed = await zlib.deflate(originalBuffer, { strategy })
          const decompressed = await zlib.inflate(compressed)
          return arrayBufferToString(decompressed) === original
        })
      })
    ),

    // Test different flush modes
    ...Object.values(ZlibFlush).map((flush) =>
      createTest(`flush mode ${flush} works correctly`, async () => {
        const original = generateTestData()
        const originalBuffer = stringToArrayBuffer(original)

        return it(async () => {
          const stream = zlib.createDeflateStream({ flush })
          const compressed = await testStream(stream, originalBuffer)
          const decompressed = await zlib.inflate(compressed)
          return arrayBufferToString(decompressed) === original
        })
      })
    ),

    // Test stream error handling
    createTest('stream handles write after end', async () => {
      const stream = zlib.createDeflateStream()

      return it(async () => {
        stream.end()
        try {
          stream.write(new ArrayBuffer(10))
          return false
        } catch (error) {
          return error instanceof Error
        }
      })
    }),

    // Test dictionary support
    createTest('custom dictionary support', async () => {
      const dictionary = stringToArrayBuffer('common dictionary text')
      const original = generateTestData()
      const originalBuffer = stringToArrayBuffer(original)

      return it(async () => {
        const compressed = await zlib.deflate(originalBuffer, { dictionary })
        const decompressed = await zlib.inflate(compressed, { dictionary })
        return arrayBufferToString(decompressed) === original
      })
    }),

    // Test maxOutputLength option
    createTest('maxOutputLength limit handling', async () => {
      const original = generateTestData(1000000) // Large data
      const originalBuffer = stringToArrayBuffer(original)

      return it(async () => {
        try {
          await zlib.inflate(originalBuffer, { maxOutputLength: 100 })
          return false
        } catch (error) {
          return error instanceof Error
        }
      })
    }),

    // Test stream concatenation
    createTest('handle concatenated streams', async () => {
      const data1 = generateTestData(100)
      const data2 = generateTestData(100)
      const buffer1 = stringToArrayBuffer(data1)
      const buffer2 = stringToArrayBuffer(data2)

      return it(async () => {
        const stream = zlib.createDeflateStream()
        const chunks: ArrayBuffer[] = []

        await new Promise<void>((resolve) => {
          stream.onData((chunk) => chunks.push(chunk))
          stream.onEnd(resolve)
          stream.write(buffer1)
          stream.write(buffer2)
          stream.end()
        })

        const compressed = new Uint8Array(
          chunks.reduce((acc, chunk) => acc + chunk.byteLength, 0)
        )
        let offset = 0
        chunks.forEach((chunk) => {
          compressed.set(new Uint8Array(chunk), offset)
          offset += chunk.byteLength
        }) 
        const decompressed = await zlib.inflate(compressed.buffer)
        return arrayBufferToString(decompressed) === data1 + data2
      })
    }),

    // Test params modification mid-stream
    createTest('stream params modification', async () => {
      const stream = zlib.createDeflateStream()

      return it(() => {
        stream.params(
          ZlibCompressionLevel.BEST_COMPRESSION,
          ZlibStrategy.HUFFMAN_ONLY
        )
        return true // If no error thrown
      })
    }),

    // Test reset functionality
    createTest('stream reset functionality', async () => {
      const original = generateTestData()
      const originalBuffer = stringToArrayBuffer(original)
      const stream = zlib.createDeflateStream()

      return it(async () => {
        await testStream(stream, originalBuffer)
        stream.reset()
        const secondResult = await testStream(stream, originalBuffer)
        const decompressed = await zlib.inflate(secondResult)
        return arrayBufferToString(decompressed) === original
      })
    }),
  ]
}
