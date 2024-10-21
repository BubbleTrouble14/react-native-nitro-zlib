import * as React from 'react'
import { SafeAreaProvider } from 'react-native-safe-area-context'
import { HybridObjectTestsScreen } from './screens/HybridObjectTestsScreen'
import { zlib } from 'react-native-nitro-zlib'

export default function App() {
  console.log(zlib.version)
  React.useEffect(() => {
    function testZlibStreams() {
      // Create a larger, more compressible dataset
      const originalData = new TextEncoder().encode(
        'Hello, world! '.repeat(1000) +
          'This is a test of Zlib streams with a larger, more compressible dataset.'
      )
      console.log('Original data size:', originalData.length, 'bytes')

      let compressedData: Uint8Array | null = null
      let decompressedData: Uint8Array | null = null

      // Create a deflate stream
      console.log('Creating deflate stream...')
      const deflateStream = zlib.createDeflateStream(6) // Compression level 6

      // Set up deflate stream events
      deflateStream.onData((chunk: ArrayBuffer) => {
        console.log('Received compressed chunk of size:', chunk.byteLength)
        const chunkArray = new Uint8Array(chunk)
        compressedData = compressedData
          ? new Uint8Array([...compressedData, ...chunkArray])
          : chunkArray
      })

      deflateStream.onEnd(() => {
        console.log('Deflate stream ended')
        if (compressedData) {
          console.log('Compressed data size:', compressedData.length, 'bytes')
          console.log(
            'Compression ratio:',
            ((compressedData.length / originalData.length) * 100).toFixed(2) +
              '%'
          )
        } else {
          console.error('No compressed data produced')
        }

        console.log('Starting decompression process...')
        // decompressWithInflateStream()
      })

      deflateStream.onError((error: Error) => {
        console.error('Deflate stream error:', error)
      })

      console.log('Writing data to deflate stream...')
      deflateStream.write(originalData.buffer)
      console.log('Ending deflate stream...')
      deflateStream.end()

      // function decompressWithInflateStream() {
      //   if (!compressedData) {
      //     console.error('No compressed data to decompress')
      //     return
      //   }

      //   console.log('Creating inflate stream...')
      //   const inflateStream = zlib.createInflateStream()

      //   inflateStream.onData((chunk: ArrayBuffer) => {
      //     console.log('Received decompressed chunk of size:', chunk.byteLength)
      //     const chunkArray = new Uint8Array(chunk)
      //     decompressedData = decompressedData
      //       ? new Uint8Array([...decompressedData, ...chunkArray])
      //       : chunkArray
      //   })

      //   inflateStream.onEnd(() => {
      //     console.log('Inflate stream ended')
      //     if (decompressedData) {
      //       console.log(
      //         'Decompressed data size:',
      //         decompressedData.length,
      //         'bytes'
      //       )

      //       const isSuccessful =
      //         decompressedData.length === originalData.length &&
      //         decompressedData.every((byte, i) => byte === originalData[i])
      //       console.log('Decompression successful:', isSuccessful)

      //       if (!isSuccessful) {
      //         console.error('Decompressed data does not match original data')
      //       }
      //     } else {
      //       console.error('No decompressed data produced')
      //     }
      //   })

      //   inflateStream.onError((error: Error) => {
      //     console.error('Inflate stream error:', error)
      //   })

      //   console.log('Writing compressed data to inflate stream...')
      //   inflateStream.write(compressedData.buffer)
      //   console.log('Ending inflate stream...')
      //   inflateStream.end()
      // }
    }

    testZlibStreams()
  }, [])

  return (
    <SafeAreaProvider>
      <HybridObjectTestsScreen />
    </SafeAreaProvider>
  )
}
