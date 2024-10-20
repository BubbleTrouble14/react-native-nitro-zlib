import { Buffer } from '@craftzdog/react-native-buffer'
global.Buffer = Buffer

import 'fast-text-encoding'

import { AppRegistry } from 'react-native'
import App from './src/App'
import { name as appName } from './app.json'

AppRegistry.registerComponent(appName, () => App)
