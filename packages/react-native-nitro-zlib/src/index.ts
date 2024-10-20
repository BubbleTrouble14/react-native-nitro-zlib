// TODO: Export all HybridObjects here
import { NitroModules } from 'react-native-nitro-modules'
import type { Zlib } from './specs/Zlib.nitro'

export * from './specs/Zlib.nitro'

export const zlib = NitroModules.createHybridObject<Zlib>('Zlib')
