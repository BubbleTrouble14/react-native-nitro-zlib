import type { State } from './Testers'
import { it } from './Testers'
import { stringify } from './utils'
import type { Zlib, ZlibOptions, ZlibStream } from 'react-native-nitro-zlib'

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
    { level: 6 },
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
            const compressed = await zlib.deflate(originalBuffer, options)
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

        return {
          originalSize: buffer.byteLength,
          compressedSize: compressed.byteLength,
          ratio: compressed.byteLength / buffer.byteLength,
          timeMs: end - start,
        }
      })
    }),

    // // Comprehensive stream tests
    // createTest('deflate/inflate stream pair works correctly', async () => {
    //   const original = generateTestData()
    //   const originalBuffer = stringToArrayBuffer(original)

    //   return it(async () => {
    //     const deflateStream = zlib.createDeflateStream()
    //     const inflateStream = zlib.createInflateStream()
    //     return testStreamPair(deflateStream, inflateStream, originalBuffer)
    //   })
    // }),

    // createTest('gzip/gunzip stream pair works correctly', async () => {
    //   const original = generateTestData()
    //   const originalBuffer = stringToArrayBuffer(original)

    //   return it(async () => {
    //     const gzipStream = zlib.createGzipStream()
    //     const gunzipStream = zlib.createGunzipStream()
    //     return testStreamPair(gzipStream, gunzipStream, originalBuffer)
    //   })
    // }),

    // createTest(
    //   'deflateRaw/inflateRaw stream pair works correctly',
    //   async () => {
    //     const original = generateTestData()
    //     const originalBuffer = stringToArrayBuffer(original)

    //     return it(async () => {
    //       const deflateRawStream = zlib.createDeflateRawStream()
    //       const inflateRawStream = zlib.createInflateRawStream()
    //       return testStreamPair(
    //         deflateRawStream,
    //         inflateRawStream,
    //         originalBuffer
    //       )
    //     })
    //   }
    // ),

    // createTest('stream with different chunk sizes', async () => {
    //   const original = generateTestData(10000) // Larger test data
    //   const originalBuffer = stringToArrayBuffer(original)

    //   return it(async () => {
    //     const results = await Promise.all(
    //       [1024, 4096, 16384].map(async (chunkSize) => {
    //         const deflateStream = zlib.createDeflateStream({ chunkSize })
    //         const inflateStream = zlib.createInflateStream({ chunkSize })
    //         return testStreamPair(deflateStream, inflateStream, originalBuffer)
    //       })
    //     )
    //     return results.every((result) => result === true)
    //   })
    // }),

    // createTest('stream with different compression levels', async () => {
    //   const original = generateTestData()
    //   const originalBuffer = stringToArrayBuffer(original)

    //   return it(async () => {
    //     const results = await Promise.all(
    //       [0, 3, 6, 9].map(async (level) => {
    //         const deflateStream = zlib.createDeflateStream({ level })
    //         const inflateStream = zlib.createInflateStream()
    //         return testStreamPair(deflateStream, inflateStream, originalBuffer)
    //       })
    //     )
    //     return results.every((result) => result === true)
    //   })
    // }),

    // createTest('stream error handling', async () => {
    //   const invalidData = new ArrayBuffer(100)

    //   return it(async () => {
    //     return new Promise<boolean>((resolve) => {
    //       const inflateStream = zlib.createInflateStream()

    //       inflateStream.onError((error) => {
    //         resolve(error instanceof Error)
    //       })

    //       inflateStream.write(invalidData)
    //       inflateStream.end()
    //     })
    //   })
    // }),

    // createTest('stream memory cleanup', async () => {
    //   const original = generateTestData(1000)
    //   const originalBuffer = stringToArrayBuffer(original)

    //   return it(async () => {
    //     const stream = zlib.createDeflateStream()
    //     const initialMemory = stream.getMemorySize()

    //     await testStream(stream, originalBuffer)
    //     stream.reset()

    //     const afterMemory = stream.getMemorySize()
    //     return afterMemory <= initialMemory
    //   })
    // }),

    // createTest('stream params modification', async () => {
    //   const original = generateTestData()
    //   const originalBuffer = stringToArrayBuffer(original)

    //   return it(async () => {
    //     const stream = zlib.createDeflateStream()
    //     const results: boolean[] = []

    //     // Test initial compression
    //     const compressed1 = await testStream(stream, originalBuffer)
    //     stream.reset()

    //     // Change params and test again
    //     stream.params(9, 2) // High compression, filtered strategy
    //     const compressed2 = await testStream(stream, originalBuffer)

    //     // Verify both compressions work and are different
    //     const decompressed1 = await zlib.inflate(compressed1)
    //     const decompressed2 = await zlib.inflate(compressed2)

    //     results.push(arrayBufferToString(decompressed1) === original)
    //     results.push(arrayBufferToString(decompressed2) === original)
    //     results.push(compressed1.byteLength !== compressed2.byteLength) // Params change should affect output

    //     return results.every((result) => result === true)
    //   })
    // }),

    // createTest('stream with large data chunks', async () => {
    //   const sizes = [100000, 500000, 1000000]

    //   return it(async () => {
    //     const results = await Promise.all(
    //       sizes.map(async (size) => {
    //         const original = generateTestData(size)
    //         const originalBuffer = stringToArrayBuffer(original)

    //         const deflateStream = zlib.createDeflateStream()
    //         const inflateStream = zlib.createInflateStream()

    //         return testStreamPair(deflateStream, inflateStream, originalBuffer)
    //       })
    //     )
    //     return results.every((result) => result === true)
    //   })
    // }),

    // createTest('unzip stream with multiple formats', async () => {
    //   const original = generateTestData()
    //   const originalBuffer = stringToArrayBuffer(original)

    //   return it(async () => {
    //     const unzipStream = zlib.createUnzipStream()
    //     const results: boolean[] = []

    //     // Test with gzip format
    //     const gzipped = zlib.gzipSync(originalBuffer)
    //     results.push(
    //       arrayBufferToString(await testStream(unzipStream, gzipped)) ===
    //         original
    //     )

    //     // Reset and test with deflate format
    //     unzipStream.reset()
    //     const deflated = zlib.deflateSync(originalBuffer)
    //     results.push(
    //       arrayBufferToString(await testStream(unzipStream, deflated)) ===
    //         original
    //     )

    //     return results.every((result) => result === true)
    //   })
    // }),
  ]
}
