// import { zlib } from 'react-native-nitro-zlib'
// import type { State } from './Testers'
// import { it } from './Testers'
// import { stringify } from './utils'
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

// function createTest<T>(
//   name: string,
//   run: () => State<T> | Promise<State<T>>
// ): TestRunner {
//   return {
//     name: name,
//     run: async (): Promise<TestResult> => {
//       try {
//         console.log(`⏳ Test "${name}" started...`)
//         const state = await run()
//         console.log(`✅ Test "${name}" passed!`)
//         return {
//           status: 'successful',
//           result: stringify(state.result ?? state.errorThrown ?? '(void)'),
//         }
//       } catch (e) {
//         console.log(`❌ Test "${name}" failed! ${e}`)
//         return {
//           status: 'failed',
//           message: stringify(e),
//         }
//       }
//     },
//   }
// }

export function getTests(): TestRunner[] {
  return []
}
