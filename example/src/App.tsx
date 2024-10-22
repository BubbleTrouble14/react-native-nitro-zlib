import * as React from 'react'
import { SafeAreaProvider } from 'react-native-safe-area-context'
import { HybridObjectTestsScreen } from './screens/HybridObjectTestsScreen'
import { zlib, type ZlibOptions } from 'react-native-nitro-zlib'

const options: ZlibOptions = {
  chunkSize: 1024,
  level: 3,
  windowBits: 15,
}

export default function App() {
  React.useEffect(() => {
    async function testInflate() {
      try {
        // First create some actual data
        const testData = new TextEncoder().encode('Hello World!') // Creates a Uint8Array
        const originalBuffer = testData.buffer

        // First deflate it
        const compressed = await zlib.deflate(originalBuffer)
        console.log(
          'Deflate successful, compressed size:',
          compressed.byteLength
        )

        // Then try to inflate it back
        const inflated = await zlib.inflate(compressed)
        console.log(
          'Inflate successful, original data:',
          new TextDecoder().decode(inflated)
        )
      } catch (error) {
        console.error('Error:', error)
      }
    }

    const test = async () => {
      await testInflate()
    }
    test()
  }, [])

  return (
    <SafeAreaProvider>
      <HybridObjectTestsScreen />
    </SafeAreaProvider>
  )
}
