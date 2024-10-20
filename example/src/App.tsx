import * as React from 'react'
import { SafeAreaProvider } from 'react-native-safe-area-context'
import { HybridObjectTestsScreen } from './screens/HybridObjectTestsScreen'
import { zlib } from 'react-native-nitro-zlib'

export default function App() {
  React.useEffect(() => {
    console.log(zlib.version)
  }, [])
  return (
    <SafeAreaProvider>
      <HybridObjectTestsScreen />
    </SafeAreaProvider>
  )
}
